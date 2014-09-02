# Debian 7 + Soekris wifi-router setup instructions #
These instructions assumes a Soekris net6501 with an SSD-drive and an Atheros 9280 wifi card. It will probably work with other hardware (e.g. other wifi cards), but the instructions must be adapted accordingly. The guide is adapted from [this page](https://wiki.debian.org/green/Router), which can be helpful as a an additional reference.

After you have followed this guide, eth0 will be the WAN-port and eth1-3 and wlan0 will be private, bridged LAN ports. DHCP will be used to assign addresses in the 10.0.100.x range to clients connected to eth1-3 and wlan0. A DHCP-client will be running on eth0.

Before we begin, Debian must be [installed on the Soekris box](SoekrisInstallDebian7).

### Preparations ###
Connect to the router using ssh and log in as a user with sudo-access ("testbed"). Install necessary utilities and your favorite editor:

```
sudo apt-get install vim wavemon htop wireless-tools iw bridge-utils rcconf
```

### Set up WLAN ###
We will use hostapd to set up the wifi-card as an Access Point.  

Install hostapd:
```
sudo apt-get install hostapd
```

Create the file /etc/hostapd/hostapd.conf and add the following content (remember to replace SSID and password):

```
interface=wlan0
bridge=br0
driver=nl80211
ctrl_interface=/var/run/hostapd

ht_capab=[HT20+]

# replace the correct SSID. 
ssid=nodeX-wifi

country_code=NO
ieee80211d=1
hw_mode=g
channel=10
ignore_broadcast_ssid=0
ieee8021x=0
eap_server=0

wpa=2

# replace with real password (8+ letters)
wpa_passphrase=xxxxxxx
macaddr_acl=0
auth_algs=1
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP

# see /usr/share/doc/hostapd/examples/hostapd.conf.gz for extensive documentation w/examples
```

To start hostapd automatically and setup the network bridge, edit /etc/network/interfaces to contain:

```
# note that br0 is not included in auto
auto lo eth0 wlan0 
iface lo inet loopback

iface eth0 inet dhcp
iface wlan0 inet manual

iface br0 inet static
        bridge_fd 0
        address 10.0.100.1
        netmask 255.255.255.0
        network 10.0.100.0
        broadcast 10.0.100.255
        # hostapd will add wlan0 to the bridge... don't list it here
        bridge_ports eth1 eth2 eth3
        bridge_hw 00:0f:b5:80:d3:25
        hostapd /etc/hostapd/hostapd.conf
```

Note that there is an issue in some kernels/hostapd-versions that requires the bridge to be set up before the wlan-interface is added to it. To workaround this bug the bridge is created first with eth1-3 and the wlan interface is added later when hostapd starts.

Verify that everything works by restarting networking:

```
/etc/init.d/networking restart
```

While the network configuration is reset your ssh-connection will freeze. It usually resumes when the setup is complete. If you loose the connection and you are unable to reconnect, use a serial cable to log in and fix any errors in the configuration.

If networking restarts successfully, a new wifi-network with the ssid from the configuration file should be visible to wifi clients. As the DHCP server is not configured, you will not be able to use the new wifi-network yet. In the next section we will set up dnsmasq to dynamically assign IP-addresses to clients on eth1-3 and wlan0.

The wifi-configuration used here is only a minimal hostapd configuration. For more detailed documentation of hostapd with examples, see [this page](http://wireless.kernel.org/en/users/Documentation/hostapd) and the local example file /usr/share/doc/hostapd/examples/hostapd.conf.gz.

hostapd should not be started automatically at reboot. Use the "rcconf"-command to disable hostapd. 

To enable or disable the access point use the ifdown or ifup commands:

```
#take ap up
ifup br0

#take ap down
ifdown br0
```

These commands should automatically start and stop hostapd as needed.


### Configure DHCP ###
As a DHCP server we use dnsmasq. The DHCP server will dynamically assign addresses from 10.0.100.100 to 10.0.100.250 to clients connected to one of the interfaces that are connected to the bridge (br0). If you followed the instructions above, the interfaces that accepts clients should be eth1-3 and wlan0 (wifi).

First, install dnsmasq:
```
sudo apt-get install dnsmasq
```

Edit /etc/dnsmasq.conf to contain:
```
interface=br0
dhcp-range=private,10.0.100.100,10.0.100.250,2h

domain-needed
bogus-priv

# Set the NTP time server address to be the same machine as
# is running dnsmasq
dhcp-option=42,0.0.0.0

# Send microsoft-specific option to tell windows to release the DHCP lease
# when it shuts down. 
dhcp-option=vendor:MSFT,2,1i

# Set the limit on DHCP leases, the default is 150
## here, raised to the maximum number of hosts on networks
dhcp-lease-max=506

# Set the DHCP server to authoritative mode. In this mode it will barge in
# and take over the lease for any client which broadcasts on the network,
# whether it has a record of the lease or not. This avoids long timeouts
# when a machine wakes up on a new network. DO NOT enable this if there's
# the slighest chance that you might end up accidentally configuring a DHCP
# server for your campus/company accidentally. The ISC server uses
# the same option, and this URL provides more information:
# http://www.isc.org/index.pl?/sw/dhcp/authoritative.php
#dhcp-authoritative
```

Restart dnsmasq with

```
sudo /etc/init.d/dnsmasq restart
```

You should now be able to connect to the wifi-network and get assigned an IP-address. Internet is not available yet, as we have not set up routing yet. Routing is covered in the next section.

### Routing ###
To route packets between the local and external network we must configure IP packet forwarding. We use an extremely simple set up that also provides IP masquerading (NAT) to mimick a real wifi router. The configuration used here is probably NOT secure enough to be used on an actual router connected to the Internet. For more information about iptables in Debian see [this page](https://wiki.debian.org/iptables).

First, create /etc/network/if-pre.up/iptables. This file is executed automatically before the network interface. It should contain the following:

```
#!/bin/bash

# enable IP forwarding
echo 1 > /proc/sys/net/ipv4/ip_forward

# load saved iptables rules
/sbin/iptables-restore < /etc/iptables.up.rules
```

This script enables ip-forwarding and loads any saved iptables rules from the file /etc/iptables.up.rules. To set the script executable run

```
chmod +x /etc/network/if-pre.up/iptables
```

We now need to create the /etc/iptables.up.rules file. Execute the following commands to configure iptables:

```
iptables -F
iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
iptables -A FORWARD -i br0 -o eth0 -j ACCEPT
```

You can now save the iptables configuration to /etc/iptables.up.rules by executing

```
iptables-save > /etc/iptables.up.rules
```

Reboot the device (command "sudo reboot") to verify that everything works. Log in after restart and run "iptables-save" to verify that the rules from /etc/iptables.up.rules have been added. On my device, the output looks like this:

```
root@node1:/home/testbed# iptables-save 
# Generated by iptables-save v1.4.14 on Mon Jan 20 15:32:16 2014
*filter
:INPUT ACCEPT [200:16088]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [132:15290]
-A FORWARD -i br0 -o eth0 -j ACCEPT
COMMIT
# Completed on Mon Jan 20 15:32:16 2014
# Generated by iptables-save v1.4.14 on Mon Jan 20 15:32:16 2014
*nat
:PREROUTING ACCEPT [2:132]
:INPUT ACCEPT [2:132]
:OUTPUT ACCEPT [10:554]
:POSTROUTING ACCEPT [0:0]
-A POSTROUTING -o eth0 -j MASQUERADE
-A POSTROUTING -o eth0 -j MASQUERADE
-A POSTROUTING -o eth0 -j MASQUERADE
COMMIT
# Completed on Mon Jan 20 15:32:16 2014
```



You should now have a working wifi router and wifi clients should be able to access the Internet.

### Monitoring ###
You can use "wavemon" to monitor the wifi interface with pretty colors in console. Install with 
```
sudo apt-get install wavemon
```


