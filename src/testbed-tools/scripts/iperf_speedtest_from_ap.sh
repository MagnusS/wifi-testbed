#!/bin/sh

# get client ip
clientip=`cat /var/lib/misc/dnsmasq.leases | head -n 1 | cut -f 3 -d " "`
echo "Testing $clientip"

#ping -c 5 -t 5 -q $clientip > /dev/null && (\
    iperf -c $clientip -t 60 -i 10 
#) || echo "Aborting. Host does not respond to ping."
