#!/usr/bin/env python3

'''
(c) Copyright 2014 Magnus Skjegstad / Forsvarets forskningsinstitutt

Author: Magnus Skjegstad
'''

import paramiko 
from string import Template
import argparse
import sys

DEBUG=False

apt_dependencies="supervisor libconfig-dev python-twisted python-paramiko libpthread-stubs0-dev ntpdate tcpdump rfkill wget wpasupplicant vim wireless-tools wavemon htop iw bridge-utils rcconf dnsmasq hostapd iperf"

def debug(say, **kwargs):
    if DEBUG:
        print("DEBUG:",say, kwargs)

''' Replace substitutions (dict) in filename and return result as string '''
def parse_template(filename, substitutions):
    with open(filename, 'r') as template_file:
        templatestr = template_file.read()
    return Template(templatestr).safe_substitute(substitutions)

''' Install common dependencies on host '''
def install_deps(host):
    # Set up proxy support for apt first
    from contextlib import closing
    import io
    with closing(io.StringIO()) as result:
        try:
            print("Configuring NFS mounts in /etc/fstab - original is in /etc/fstab.orig")
            subst = dict(
                        hostname=host)
            fstabnfs = parse_template("templates/fstab.template", subst)
            upload_file_from_string(host, "tmp_fstabnfs", fstabnfs)
            success = run_command(host, "if [ ! -f /etc/fstab.orig ]; then sudo cp /etc/fstab /etc/fstab.orig; fi", out=result, err=result) == 0 and\
            run_command(host, "sudo mkdir -p /mnt/nfs/shared_code", out=result, err=result) == 0 and \
            run_command(host, "sudo mkdir -p /mnt/nfs/results", out=result, err=result) == 0 and \
            run_command(host, "sudo mv tmp_fstabnfs /etc/fstab", out=result, err=result) == 0 and \
            run_command(host, "sudo chmod 644 /etc/fstab", out=result, err=result) == 0 and \
            run_command(host, "sudo chown root.root /etc/fstab", out=result, err=result) == 0 and \
            run_command(host, "sudo mount -a", out=result, err=result) == 0
            if not success:
                raise Exception("Unable to set up NFS shares / new fstab")

            subst = dict(
                        httpproxy="http://10.10.1.254:3128",
                        ftpproxy="ftp://10.10.1.254:3128")
            print("Configuring proxy server support (%s)" % subst)

            debconf = parse_template("templates/70debconf.template", subst)
            upload_file_from_string(host, "tmp_70debconf", debconf)
            success = run_command(host, "sudo mv tmp_70debconf /etc/apt/apt.conf.d/70debconf", out=result, err=result) == 0 and \
            run_command(host, "sudo chmod 644 /etc/apt/apt.conf.d/70debconf", out=result, err=result) == 0 and \
            run_command(host, "sudo chown root.root /etc/apt/apt.conf.d/70debconf", out=result, err=result) == 0
            if not success:
                raise Exception("Unable to setup debconf with proxy config")

            print("Uploading timesync.sh to /etc/cron.daily")
            debconf = parse_template("templates/timesync.sh.template", dict())
            upload_file_from_string(host, "tmp_timesync.sh", debconf)
            # remember, no .sh-extension here or cron will ignore it
            success = run_command(host, "sudo mv tmp_timesync.sh /etc/cron.daily/timesync", out=result, err=result) == 0 and \
            run_command(host, "sudo chmod 755 /etc/cron.daily/timesync", out=result, err=result) == 0 and \
            run_command(host, "sudo chown root.root /etc/cron.daily/timesync", out=result, err=result) == 0
            if not success:
                raise Exception("Unable to set up automatic time sync")

            print("Synchronizing time...")
            run_command(host, "sudo /etc/cron.daily/timesync", err=result, out=result) # run timesync just in case :-)

            print("Apt-getting...")

            # Upgrade and update

            upgrade_ok = \
                run_command(host, "sudo apt-get update", err=result, out=result) == 0 and \
                run_command(host, "sudo apt-get -y upgrade", err=result, out=result) == 0

            if not upgrade_ok:
                raise Exception("Unable to update packages. Are the nodes allowed Internet access?")

            # Install deps

            deps_installed = \
                run_command(host, "sudo apt-get -y install %s" % apt_dependencies, out=result, err=result) == 0

            if not deps_installed:
                raise Exception("Unable to install tools. Aborting.")

            print("hostapd installed. Removing from startup")
            run_command(host, "sudo update-rc.d hostapd disable", out=result, err=result)

            print("Disabling AP just in case...")
            disable_ap(host)
        except:
            print("Exception. Output was:")
            print(result.getvalue())
            raise

''' Enable AP by taking br0 up. Hostapd must already be configured '''
def enable_ap(host):
    rfkill_unblock(host)
    return run_command(host, 'sudo ifup br0')

''' Disable AP by taking br0 down. Hostapd must already be configured '''
def disable_ap(host, block=True):
    rfkill_block(host)
    return run_command(host, 'sudo ifdown br0')

''' Enable client by taking wlan0 up '''
def enable_client(host):
    rfkill_unblock(host)
    return run_command(host, 'sudo ifup wlan0')

def get_wifi_state(host):
    import io
    from contextlib import closing 

    with closing(io.StringIO()) as result:
        if run_command(host, "sudo ifconfig | grep wlan0", out=result) == 0:
            return result.getvalue().strip() != ""
        else:
            return False

''' Is wifi client associated with AP? '''
def get_assoc_state(host):
    import io
    from contextlib import closing 

    with closing(io.StringIO()) as result:
        # grep returns 1 when not found, 0 when found
        if run_command(host, "sudo iwconfig wlan0 | grep Not-Associated", out=result) == 0:
            return False
        else:
            return True

''' Get ip the client received from the AP. '''
def get_client_wifi_ip(host):
    import io
    from contextlib import closing 

    with closing(io.StringIO()) as result:
        if run_command(host, 'sudo ifconfig  wlan0  | grep "inet addr" | cut -f2 -d":" | cut -f1 -d" "', out=result) == 0:
            return result.getvalue().strip()
        else:
            return False

''' Disable client by taking wlan0 down '''
def disable_client(host, block=True):
    rfkill_block(host)
    return run_command(host, 'sudo ifdown wlan0')

''' rfkill radio block '''
def rfkill_block(host):
    return run_command(host, 'sudo rfkill block all')

''' rfkill radio unblock '''
def rfkill_unblock(host):
    return run_command(host, 'sudo rfkill unblock all')

''' Restart netwokring on host '''
def restart_networking(host):
    return run_command(host, 'sudo service networking restart')

''' Generate random password string '''
def generate_password(length=8):
    # from http://stackoverflow.com/questions/2782229/most-lightweight-way-to-create-a-random-string-and-a-random-hexadecimal-number
    import os,binascii
    return binascii.b2a_hex(os.urandom(int(length / 2)))

''' Run a command via SSH on host. Returns exit status. Assumes that SSH keys are used for authentication '''
def run_command(host, command, user="testbed", out=None, err=None):
    debug("Running %s on %s..." % (host, command))

    client = paramiko.SSHClient()
    #we can skip this, as we add anyway. It is also very slow on python3 for some reason..
    #client.load_system_host_keys()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

    client.connect(host, look_for_keys=True, username=user) # we assume that key based auth is set up
    
    stdin,stdout,stderr = client.exec_command(command)
    exit_status = stdout.channel.recv_exit_status()

    if out == None and DEBUG:
        out = sys.stdout

    if err == None and DEBUG:
        err = sys.stderr

    if out != None:
        for line in stdout:
            out.write(line)

    if err != None:
        for line in stderr:
            err.write(line)

    client.close()
    return exit_status

''' Configure node as access point by changing network configuration, setting up hostapd and so on. Will reboot the device after install '''
def configure_ap(host, nodename, default_channel=6, default_txpower=20, password=None, reboot=True):
    print("Configuring AP on %s" % host)
    if password == None:
        password = generate_password(8)
    subst = dict(
            node = nodename,
            channel = default_channel,
            txpower = default_txpower,
            wifipassword = password)

    debug("Substitutions...")
    debug("%s" % subst)

    debug( "Uploading templates...")
    hostapd_conf = parse_template("templates/hostapd.conf.template", subst)
    dnsmasq_conf = parse_template("templates/dnsmasq.conf.template", subst)
    interfaces = parse_template("templates/interfaces.template", subst)
    iptables_up_rules = parse_template("templates/iptables.up.rules.template", subst)
    iptables = parse_template("templates/iptables-up.template", subst)

    upload_file_from_string(host, "tmp_hostapd.conf", hostapd_conf)
    upload_file_from_string(host, "tmp_dnsmasq.conf", dnsmasq_conf)
    upload_file_from_string(host, "tmp_interfaces", interfaces)
    upload_file_from_string(host, "tmp_iptables", iptables)
    upload_file_from_string(host, "tmp_iptables.up.rules", iptables_up_rules)

    debug( "Moving to correct location using sudo...")
    run_command(host, "sudo mv tmp_hostapd.conf /etc/hostapd/hostapd.conf")
    run_command(host, "sudo mv tmp_dnsmasq.conf /etc/dnsmasq.conf")
    run_command(host, "sudo mv tmp_interfaces /etc/network/interfaces")
    run_command(host, "sudo mv tmp_iptables /etc/network/if-up.d/iptables")
    run_command(host, "sudo mv tmp_iptables.up.rules /etc/iptables.up.rules")

    debug("Disable iperf server...")
    run_command(host, "sudo rm /etc/supervisor/conf.d/iperfserver.conf")
    run_command(host, "sudo supervisorctl update")

    run_command(host, "sudo chmod +x /etc/network/if-up.d/iptables")

    if reboot:
        reboot_host(host)

''' Configure node as access point by changing network configuration, setting up hostapd and so on. Will reboot the device after install '''
def configure_client(host, nodename, ssid, key, reboot=True, default_txpower=20):
    print("Configuring client on %s" % host)
    subst = dict(
            node = nodename,
            wifissid = ssid,
            wifipassword = key,
            txpower = default_txpower)

    debug( "Substitutions...")
    debug("%s" % subst)

    debug( "Uploading templates...")
    interfaces = parse_template("templates/interfaces.client.template", subst)
    upload_file_from_string(host, "tmp_interfaces", interfaces)
    run_command(host, "sudo mv tmp_interfaces /etc/network/interfaces")

    debug( "Remove AP configuration files")
    run_command(host, "sudo rm /etc/hostapd/hostapd.conf")
    run_command(host, "sudo rm /etc/dnsmasq.conf")
    run_command(host, "sudo rm /etc/network/if-up.d/iptables")
    run_command(host, "sudo rm /etc/iptables.up.rules")

    debug("Enabling iperf server")
    iperfconf = parse_template("templates/iperf.supervisor.conf", dict())
    upload_file_from_string(host, "tmp_iperf.conf", iperfconf)
    run_command(host, "sudo mv tmp_iperf.conf /etc/supervisor/conf.d/iperfserver.conf")
    run_command(host, "sudo supervisorctl update")

    if reboot:
        reboot_host(host)

''' Reboot host '''
def reboot_host(host):
    debug( "Rebooting %s..." % host)
    run_command(host, "sudo reboot")

''' Upload file over SFTP '''
def upload_file_from_string(host, remote_filename, content, user="testbed"):
    client = paramiko.SSHClient()
    #skip this step as we accept all hosts anyway
    #client.load_system_host_keys()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(host, look_for_keys=True, username=user) # we assume that key based auth is set up
    sftp = client.open_sftp()

    f = sftp.open(remote_filename, mode='w')
    f.write(content)
    f.flush()

    client.close()

''' Parse iw scan result as StringIO '''
def parse_scan(result,host,nodeid):
    scan = []
    for _line in result.getvalue().split("\n"):
        line = _line.strip()
        s = line.split(" ", 1)
        if s[0] == "BSS":
            scan.append({})
            scan[len(scan)-1]["scanner_name"] = host
            scan[len(scan)-1]["scanner_id"] = nodeid
            scan[len(scan)-1]["bss"] = s[1].split(" ")[0]

        if s[0] == "SSID:":
            if len(s) > 1:
                scan[len(scan)-1]["ssid"] = s[1]
                if s[1][:4] == "node":
                    scan[len(scan)-1]["found_id"] = int(s[1].split("-")[0][4:])
            else:
                scan[len(scan)-1]['ssid'] = ""
        if s[0] == "signal:":
            scan[len(scan)-1]["signal"] = s[1].split(" ")[0]
        if s[0] == "freq:":
            scan[len(scan)-1]["freq"] = s[1]
        if s[0] == "last":
            scan[len(scan)-1]["last_seen_ms"] = s[1].split(" ")[1]

    return scan


''' Scan for other APs '''
def scan_wifi(nodeid, host=None, dumpfile=None):
    import io
    from contextlib import closing
    if host == None:
        host = "node%d" % nodeid

    with closing(io.StringIO()) as result:
        debug( "Scanning for SSIDs...")
        if run_command(host, 'sudo ifconfig wlan0 up') == 0 and \
                    run_command(host, 'sudo iw dev wlan0 scan', out=result) == 0 and \
                    run_command(host, 'sudo ifconfig wlan0 down') == 0:

            # parse output
            parsed_result = parse_scan(result,host,nodeid)
            if dumpfile:
                with open(dumpfile, "a") as myfile:
                    import time
                    myfile.write("%s\n" % dict(time=int(time.time()), scan=parsed_result))
            return parsed_result
        else:
            raise Exception("Unable to scan host %s" % host)


''' Check if host is configured as AP '''
def is_ap(host):
    return run_command(host, 'sudo test -e /etc/hostapd/hostapd.conf') == 0

''' Get ap ssid '''
def get_ap_ssid(host):
    import io
    from contextlib import closing 

    with closing(io.StringIO()) as result:
        if run_command(host, "sudo cat /etc/network/interfaces  | grep ssid | sed -e 's/ssid=\(.*\)/\\1/g'", out=result) == 0:
            return result.getvalue().strip()
        else:
            return None

''' Get client ssid '''
def get_client_ssid(host):
    import io
    from contextlib import closing 
    with closing(io.StringIO()) as result:
        if run_command(host, "sudo cat /etc/network/interfaces  | grep wpa-ssid | sed -e 's/.*wpa-ssid \(.*\)/\\1/g'", out=result) == 0:
            return result.getvalue().strip()
        else:
            return None

''' Get ssid '''
def get_ssid(host):
    if is_ap(host):
        return get_ap_ssid(host)
    else:
        return get_client_ssid(host)
    
''' Change wifi channel on an already configure AP '''
def ap_change_channel(host, channel):
    if not is_ap(host):
        raise Exception("%s is not an access point." % host)

    # replace channel=xx and pipe output to a tmp-file
    run_command(host, "sudo cat /etc/hostapd/hostapd.conf | sed -e 's/channel=[0-9][0-9]*/channel=%d/g' > tmp_hostapd.conf" % channel)
    # overwrite original with temp file
    run_command(host, "sudo mv tmp_hostapd.conf /etc/hostapd/hostapd.conf")

    # reconfigure ap by restarting the service
    disable_ap(host)
    enable_ap(host)

''' Change txpower in /etc/network/interfaces '''
def set_txpower(host, newtx):
    run_command(host, "sudo cat /etc/network/interfaces | sed -ne '/txpower/ !{p }; /txpower/ { s/txpower [0-9][0-9]*/txpower %d/g; p} > tmp_interfaces" % newtx)
    run_command(host, "sudo mv tmp_interfaces /etc/network/interfaces")
    run_command(host, "sudo iwconfig wlan0 txpower %d" % newtx) # set right away

''' Get current wifi channel '''
def get_channel(host):
    from utils import WifiHelpers as wh
    freq = get_freq(host)
    if freq:
        return wh.freq_to_chan(get_freq(host))
    else:
        return 0

''' Get current wifi frequency '''
def get_freq(host):
    import io
    from contextlib import closing 

    with closing(io.StringIO()) as result:
        if run_command(host, "sudo iwconfig wlan0 | grep Freq", out=result) == 0:
            return float(result.getvalue().split(':')[2].split(' ')[0].strip())
        else:
            return None

''' Get current wifi tx power '''
def get_txpower(host):
    import io
    from contextlib import closing 

    with closing(io.StringIO()) as result:
        # use sed to extract Tx-Power from iwconfig output
        if run_command(host, "sudo iwconfig wlan0 | sed -ne '/.*Tx-Power=.*/ { s/.*Tx-Power=\([0-9][0-9]*\).*/\\1/g; p }'", out=result) == 0:
            try:
                return int(result.getvalue())
            except:
                return None # assume None if not parsable
        else:
            return None

''' Create connectivity matrix '''
def graph_connectivity_matrix(live=False, show=True, outputfile=None):
    import pylab as plt
    import numpy as np

    mat = np.zeros((21,21))
    mat.fill(0)
    mat[0,0] = -100

    if show:
        plt.ion()
        graph = plt.matshow(mat)
        graph.set_data(mat)
        plt.draw()

    while True:
        for n in range(1,22):
            host = "node%d" % n
            debug("Scanning from %s" % host)
            mat[n-1].fill(-100)
            is_accesspoint = is_ap(host)
            try:
                if is_accesspoint:
                    disable_ap(host)
                else:
                    disable_client(host)

                scan = scan_wifi(nodeid=n, dumpfile=outputfile)

                for s in scan:
                    try:
                        mat[s['scanner_id']-1, s['found_id']-1] = s['signal']
                    except:
                        pass

            finally:
                if is_accesspoint:
                    enable_ap(host)
                else:
                    enable_client(host)

            debug ("%s" % mat)
            if show:
                graph.set_data(mat)
                plt.pause(0.01)
                plt.draw()
        
        if not live:
            break

        debug( "Recalculating...")

    debug( "Done.")

def parallel_isap(nodeids):
    isap = [ False ] * 22 # creating array with true/false for AP in parallel
    from concurrent.futures import ThreadPoolExecutor
    with ThreadPoolExecutor(max_workers=30) as e:
        futures = {}
        for nodeid in nodeids:
            futures[nodeid] = e.submit(is_ap, host=nodeinfo[nodeid]['hostname'])

        for f in futures:
            isap[int(f)] = futures[f].result()
    return isap
            
''' Upload and run script '''
def upload_and_run(host, script_path, out=sys.stdout, err=sys.stderr):
    debug("Uploading %s and running on %s" % (script_path, host))
    import os
    basename = os.path.basename(script_path)
    f = parse_template(script_path, dict()) # no substitutions yet
    script_dir = "tmp_script"
    run_command(host, "mkdir %s" % script_dir)
    try: 
        upload_file_from_string(host, "%s/%s" % (script_dir, basename), f)
        run_command(host, "chmod +x %s/%s" % (script_dir,  basename), out=out, err=err)
        run_command(host, "cd %s ; ./%s" % (script_dir, basename), out=out, err=err)
    finally: # always clean up by deleting tmp folder
        run_command(host, "rm -rf %s" % script_dir, out=out, err=err)

def get_iperf_throughput(ap, time=10):
    import io
    from contextlib import closing 

    with closing(io.StringIO()) as result:
        # use sed to extract Tx-Power from iwconfig output
        if run_command(ap, "sudo cat /var/lib/misc/dnsmasq.leases | head -n 1 | cut -f 3 -d ' '", out=result) == 0:
            clientip = result.getvalue().strip()
        else:
            raise Exception("Unable to obtain client IP from dnsmasq.leases")

    with closing(io.StringIO()) as result:
        # use sed to extract Tx-Power from iwconfig output
        if run_command(ap, "sudo iperf -c %s -y c -t %d | cut -f9 -d','" % ( clientip, time ), out=result) == 0:
            throughput = result.getvalue().strip()
        else:
            raise Exception("Unable to test throughput")

    return throughput        

def get_minstrel_fer(ap, time=10):
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(ap, look_for_keys=True, username="testbed") # we assume that key based auth is set up

    from executor import Executor
    e = Executor(sshclient=client)

    from minstrel import Minstrel
    m = Minstrel(minstrel_path="/sys/kernel/debug/ieee80211/phy0/netdev:wlan0/stations/", executor=e)

    from time import sleep
    samples = []
    for i in range(time):
        rates = m.get_frame_error_rates()
        for key in rates:
            samples.append(rates[key]["current"])
        sleep(1)

    return samples

def dump_node(nodeid, isap):
    host = nodeinfo[nodeid]['hostname']
    channel  = get_channel(host)
    txpower = get_txpower(host)
    enabled = get_wifi_state(host)
    if isap:
        ssid = get_ap_ssid(host)
    else:
        ssid = get_client_ssid(host)

    return dict(channel=channel, txpower=txpower, ssid=ssid, enabled=enabled)

def dump_topology(nodeids):
    isap = parallel_isap(nodeids)

    # create data structure
    results = {}
    for nodeid in nodeids:
        results[nodeid] = {}
        if isap[int(nodeid)]:
            results[nodeid]['clients'] = []
            results[nodeid]['ap'] = True
        else:
            results[nodeid]['ap'] = False

    futures = {}
    from concurrent.futures import ThreadPoolExecutor
    with ThreadPoolExecutor(max_workers=30) as e:
        for nodeid in nodeids:
            futures[nodeid] = e.submit(dump_node, nodeid=nodeid, isap=isap[int(nodeid)])

    # read info from nodes
    for nodeid in nodeids:
        host = nodeinfo[nodeid]['hostname']
        results[nodeid]['channel'] = futures[nodeid].result()["channel"]
        results[nodeid]['txpower'] = futures[nodeid].result()["txpower"]
        results[nodeid]['enabled'] = futures[nodeid].result()["enabled"]
        if results[nodeid]['ap'] == False: # if this is a client, add it to its AP's list of clients
            master_ssid = futures[nodeid].result()["ssid"] # get clients ssid first
            for key in nodeinfo: # find ap node with this ssid 
                if nodeinfo[key]['ssid'] == master_ssid:
                    try:
                        results[key]['clients'].append(int(nodeid)) # store client id in ap's client list
                    except KeyError:
                        raise Exception("Referred to unknown AP with id %s" % key)

    return results







###################################################################
#######################Endringer###################################
###################################################################
def get_topology():
    import os
    # aapner tmp-fila for aa hente ut klientene til hvert av APene

    with open("wifi_measurement/tmp_topology.txt", "r") as input_file, \
        open("wifi_measurement/aps_with_clients.txt", 'w') as output_file:

        k = -1

        lines_in_file = input_file.readlines()  # lagrer alle linjer i et array
        p = len(lines_in_file)

        for i in range(0, p):
            k = k + 1

            if '"' in lines_in_file[i].strip()[0]:      # Kvalifiseet soek
                if lines_in_file[i+2].strip() == ('"ap": true,'):
                    tt = lines_in_file[i].strip()[1:3]          # PLukker ut 5. og 6. tegn
                    if '"' in tt[1]:          # f.eks "1"
                        output_file.write(tt[0]+":")

                    else:                 # f.eks "10"
                        output_file.write(tt+":") 
          


                    for n in range(k, (k+10)):

                        if lines_in_file[n].strip()[0].isdigit():    # henter ut siste klient som har ett siffer
                            output_file.write(lines_in_file[n].strip() + '\n')

    output_file.close()
    input_file.close()


def sort_results():
  import os
  aps_w_c = open("wifi_measurement/aps_with_clients.txt")
  lines_in_file = aps_w_c.readlines()
  aps_w_c.close()

  os.system("> wifi_measurement/Results.txt")

  for v in range(0, len(lines_in_file)):
  
    ac = lines_in_file[v].split(":")
    ap = ac[0]              # AP of current line

    fil = open("wifi_measurement/Results.txt", "a")        # Writes current AP to file as header
    fil.write("node%s-wifi:\n" % (str(ap)))
    fil.close()

    ac[1] = ac[1].replace('\n', '')
    clients = ac[1].split(",")      # All clients of current line
    for cls in range(0, len(clients)):
      write_results(clients[cls])     # Gets results from clients that are not last
          
    fil = open("wifi_measurement/Results.txt", "a")
    fil.write("\n\n")
    fil.close()

  os.system("cat wifi_measurement/Results.txt")      # Displays file in terminal


def write_results(m):
  import os 

  if os.path.isfile("wifi_measurement/mgen_log/plot_node%s.gp" % (m)):
    p = open("wifi_measurement/mgen_log/plot_node%s.gp" % (m))
    plines = p.readlines()
    p.close()

    if len(plines) > 15:
      with open("wifi_measurement/mgen_log/plot_node%s.gp" % m) as b:
        
        counter = 0
        bitrate = 0
        lines = b.readlines()
        nbitrates = 0

        for i in range(11,len(lines)):
          line = lines[i]
          
          for k in range(5, 25):          # Gathers bitrates between packets sent after 5 seconds till 25 seconds
            if line.startswith('%s' % k):
              f = line.split(', ')
              value = f[1].split('e') # tallverdien ligger i value[0]   
              value[1] = value[1].replace('+0', '')
              value[1] = value[1].replace('\n', '')

              tv = float(value[0])
              eksp = float(value[1])
              
              bitrate = tv*(10**eksp)

              counter += float(bitrate)
              nbitrates += 1

        avg = counter/nbitrates    # Calculates average bitrate
        avglim = round(avg)
    

        h = open("wifi_measurement/Results.txt", "a")
        if int(m) > 9:
          h.write("Node" + str(m) + ": " + str(avglim) + " kbit/s\n")     # Writes results to file
        if int(m) <= 9:
          h.write("Node" + str(m) + ":  " + str(avglim) + " kbit/s\n")     # Writes results to file
        h.close()

###################################################################
#######################Endringer Slutt#############################
###################################################################

parser = argparse.ArgumentParser(description='Utility used to control and configure wifi testbed nodes.')

subparsers = parser.add_subparsers(dest="subparser", help='sub-command help')

parser_reboot = subparsers.add_parser('reboot', help='reboot node(s)')
parser_reboot.add_argument('node', nargs='+', help='node id from 1 to 21')

parser_setup = subparsers.add_parser('setup', help='installation utilities')
parser_setup.add_argument('node', nargs='+', help='node id from 1 to 21. Use ap:client1,client2... for setting up APs with clients.')
parser_setup.add_argument('--ap', metavar="CHAN:AP_TX:CLI_TX", dest="ap", type=str, help="install access point configuration for channel CHAN with AP tx power AP_TX and client tx power CLI_TX, then reboot")
parser_setup.add_argument('--deps', dest="deps", action="store_true", help="update/upgrade apt-get and install dependencies if needed")

parser_test = subparsers.add_parser('test', help='test utilities')
parser_test.add_argument('--ap_to_client_upload', type=int, dest="ap_to_client_upload", help='measure client download throughput')
parser_test.add_argument('node', nargs='+', help='node id from 1 to 21')

parser_topo = subparsers.add_parser('topology', help='json topology deployment utilities')
parser_topo.add_argument('node', nargs='+', help='node id from 1 to 21')
parser_topo.add_argument('--info', dest="info", action="store_true", help="print node info from node.json for these nodes (location and name) and exit")
parser_topo.add_argument('--dump', dest="dump", action="store_true", help="dump current configuration from nodes and output in json topology format")
parser_topo.add_argument('--state', dest="state", action="store_true", help="as dump, but includes additional information like client ips and associated state")
parser_topo.add_argument('--load', dest='load', type=str, help="load topology from json topology file")

parser_ap = subparsers.add_parser('wifi', help='configure access point or client')
parser_ap.add_argument('node', nargs='+', help='node id from 1 to 21')
parser_ap.add_argument('--enable', dest="enable", action="store_true", help="enable wifi")
parser_ap.add_argument('--disable', dest="disable", action="store_true", help="disable wifi")
parser_ap.add_argument('--set_channel', type=int, help='change channel - only valid for APs')
parser_ap.add_argument('--get_channel', action="store_true", help='get channel')
parser_ap.add_argument('--get_tx', action="store_true", help='get current tx power')
parser_ap.add_argument('--set_tx', type=int, help='set tx power (runs wifi enable/disable)')
parser_ap.add_argument('--scan', dest="scan", action="store_true", help="perform ssid scan. Wifi must be disabled first on APs")

parser_webrun = subparsers.add_parser('remoterun', help='upload and run a script')
parser_webrun.add_argument('node', nargs='+', help='node id from 1 to 21')
parser_webrun.add_argument('--run', type=str, help='local path to script to run')
parser_webrun.add_argument('--command', type=str, help='command to run')

parser_graph = subparsers.add_parser('graph', help='graphs')
parser_graph.add_argument('--connectivitymatrix', action="store_true", help='generate connectivity matrix based on wifi scan and signal strength observed from AP nodes')
parser_graph.add_argument('--show', action="store_true", help="show graph using pyplot")
parser_graph.add_argument('--outputfile', type=str, help="dump output to file")
parser_graph.add_argument('--live', action="store_true", help='recalculate periodically')

###################################################################
#######################Endringer###################################
###################################################################
parser_measurement = subparsers.add_parser('top', help='Testing')

parser_measurement.add_argument('--topologi', action="store_true", help='get current topology')




args = parser.parse_args()
debug("%s" % args)

debug("Reading nodes metadata from nodes.json")
import json, sys
# TODO: All references to node%s as hostname and node%s-wifi for ssid should be replaced with values from this json-file
with open("nodes.json", "r") as f:
    nodeinfo = json.load(fp=f)

nodes = []
#try to extract nodes, but may fail for some param combinations
if args.node != None:
  for f in args.node:
    try:  # if ap:client1,client2... format do this:
      (ap_host, clients) = f.split(":")
    except:  # on error we assume that only ap is specified
      ap_host = f
      clients = None
      pass
    nodes.append(nodeinfo[ap_host]['hostname'])
    if clients:
      for clientid in clients.split(","):
        nodes.append(nodeinfo[clientid]['hostname'])

###################################################################
#######################Endringer###################################
###################################################################
if args.subparser == 'top':
    if args.topologi:
        get_topology()






if args.subparser == 'remoterun':
    if args.run:
        print("Uploading and running %s on nodes %s.." % (args.run, nodes))
        for n in nodes:
            print("#### Node %s" % n)
            upload_and_run(n, args.run, out=sys.stdout, err=sys.stderr)
    if args.command:
        print("Running %s on nodes %s..." % (args.command, nodes))
        for n in nodes:
            print("#### Node %s" % n)
            run_command(n, args.command, out=sys.stdout, err=sys.stderr)

if args.subparser == "graph":
    if args.connectivitymatrix:
        graph_connectivity_matrix(live=args.live, outputfile=args.outputfile, show=args.show)

if args.subparser == "reboot":
    from concurrent.futures import ThreadPoolExecutor
    with ThreadPoolExecutor(max_workers=30) as e:
        for host in nodes:
            print("Rebooting %s" % host)
            e.submit(reboot_host, host=host)

if args.subparser == "test":
    if args.ap_to_client_upload:
        isap = parallel_isap(args.node)
        from concurrent.futures import ThreadPoolExecutor

        fut_throughput = {}
        fut_minstrel = {}
        time = args.ap_to_client_upload
        print("# Testing throughput on AP nodes for %d seconds" % time)
        with ThreadPoolExecutor(max_workers=60) as e:
            for nodeid in args.node:
                if isap[int( nodeid )]:
                    fut_minstrel[nodeid] = e.submit(get_minstrel_fer, ap=nodeinfo[nodeid]['hostname'], time=time)
                    fut_throughput[nodeid] = e.submit(get_iperf_throughput, ap=nodeinfo[nodeid]['hostname'], time=time)

        results = {}
        for n in fut_throughput:
            import numpy as np
            minstrel = fut_minstrel[n].result()

            results[n] = dict(minstrel_avg = np.mean(minstrel), 
                             minstrel_std = np.std(minstrel, ddof=1), 
                             minstrel_median = np.median(minstrel), 
                             throughput = fut_throughput[n].result())

        print(json.dumps(results, indent=4))

if args.subparser == "topology":
    if args.info:
        for nodeid in args.node:
            print("Node ID:",nodeid)
            for key in nodeinfo[nodeid]:
                print("%s: %s" % (key, nodeinfo[nodeid][key]))

    if args.dump:
        results = dump_topology(args.node)
        print(json.dumps(results, indent=4))

    if args.state: # as dump, but with additional info
        results = dump_topology(args.node)
        for key in results:
            if results[key]['ap'] == False:
                results[key]['is_associated'] = get_assoc_state(nodeinfo[key]['hostname'])
                results[key]['wifi_ip'] = get_client_wifi_ip(nodeinfo[key]['hostname'])
            results[key]['ssid'] = get_ssid(nodeinfo[key]['hostname'])

        print(json.dumps(results, indent=4))


    if args.load:
        debug("Reading node topology from %s" % args.load)
        import json, sys
        with open(args.load, "r") as f:
            topology = json.load(fp=f)


        # reformat so clients know their ssid, as the json file has client lists on the ap entries
        for f in topology:
            if topology[f]['enabled'] == True and topology[f]['ap'] == True:
                found_client = False
                for client in map(str, topology[f]['clients']):
                    found_client = True
                    topology[client]['ssid'] = nodeinfo[f]['ssid']
                    wifipassword = generate_password(16)
                    topology[client]['key'] = wifipassword
                    topology[f]['key'] = wifipassword

        # check that no clients are without APs
        for f in topology:
            try:
                if topology[f]['ap'] == False and not topology[f]['ssid']:
                    raise Exception("No SSID for client %s" % f)
            except KeyError:
                raise Exception("No SSID for client %s" % f)

        # now configure ap's and clients in parallel
        from concurrent.futures import ThreadPoolExecutor
        with ThreadPoolExecutor(max_workers=30) as e:
            futures = []
            for t in topology:
                if topology[t]['enabled'] == True:
                    if topology[t]['ap'] == True:
                        futures.append(e.submit(configure_ap, host=nodeinfo[t]['hostname'], nodename=nodeinfo[t]['hostname'], default_channel=topology[t]['channel'], 
                                default_txpower=topology[t]['txpower'], password=topology[t]['key'], reboot=True))
                    else:
                        futures.append(e.submit(configure_client, host=nodeinfo[t]['hostname'], nodename=nodeinfo[t]['hostname'], 
                                ssid=topology[t]['ssid'], key=topology[t]['key'], reboot=True, default_txpower=topology[t]['txpower']))

if args.subparser == "setup":
    if args.deps:
        for host in nodes:
            print("Installing dependencies and upgrading packages on %s" % (host))
            install_deps(host)

    if args.ap != None:
        ap_channel, ap_tx, cli_tx = args.ap.split(":")

        for host in args.node:
            try: # if ap:client1,client2... format do this:
                (ap_host,clients) = host.split(":")
            except: # on error we assume that only ap is specified
                ap_host = host
                clients = None
                pass

            wifipassword = generate_password(16)

            print("Connecting %s to client(s) %s (with reboot)" % (ap_host, clients))
            print("Installing access point files on node %s with default channel %s ap power %s and cli power %s" % (ap_host, ap_channel, ap_tx, cli_tx))
            ap_hostname=nodeinfo[ap_host]['hostname']
            configure_ap(ap_hostname, ap_hostname, password=wifipassword, default_channel=int(ap_channel), default_txpower=int(ap_tx))
            if clients:
                print("Installing client configurations")
                for clientid in clients.split(","):
                    print("Configuring client node%s" % clientid)
                    configure_client("node%s" % clientid, "node%s" % clientid, key=wifipassword, default_txpower=int(cli_tx), ssid="%s-wifi" % ap_host)

if args.subparser == "wifi":
    isap = parallel_isap(args.node)

    # now enable wifi 
    from concurrent.futures import ThreadPoolExecutor
    with ThreadPoolExecutor(max_workers=30) as e:
        for nodeid in args.node:
            host = nodeinfo[nodeid]['hostname']
            if args.set_channel:
                e.submit(ap_change_channel, host=host, channel=args.set_channel)

            if args.get_channel:
                print("Current channel on %s is %d" % (host, get_channel(host)))

            if args.get_tx:
                print("Current tx power on %s is %d" % (host, get_txpower(host)))

            if args.set_tx:
                e.submit(set_txpower, host=host, txpower=args.set_tx)
                print("Current tx power on %s is set to %d" % (host, args.set_tx))

            if isap[int( nodeid )]:
                if args.disable:
                    print("# Disabling wifi on AP %s" % host)
                    e.submit(disable_ap, host=host)

                if args.scan:
                    print("# Performing scan from AP %s" % host)
                    scan = scan_wifi(nodeid=int(host[4:]))
                    print(scan)

                if args.enable:
                    print("# Enabling wifi on AP %s" % host)
                    e.submit(enable_ap, host=host)
            else:
                if args.disable:
                    print("# Disabling wifi on client %s" % host)
                    e.submit(disable_client, host=host)

                if args.scan:
                    print("# Performing scan from client %s" % host)
                    scan = scan_wifi(nodeid=int(host[4:]))
                    print(scan)

                if args.enable:
                    print("# Enabling wifi on client %s" % host)
                    e.submit(enable_client, host=host)

