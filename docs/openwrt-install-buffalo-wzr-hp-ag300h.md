# Installing OpenWRT on Buffalo WZR-HP-AG300H

This document contains useful links for installing OpenWRT.

The general installation instructions for this router is available [here](http://wiki.openwrt.org/toh/buffalo/wzr-hp-ag300h).

AG300H is no longer produced. [600DHP](http://wiki.openwrt.org/toh/buffalo/wzr-600dhp) is reported to have the same hardware, and the same installation instructions should work.

Note that if you are unable to update using the web gui, the 600DHP instructions describes an alternative method. The DD-WRT that came with the routers (DD-WRT v24SP2-MULTI (03/21/11) std) seems to work, though.

### Development version (trunk)

The following may be useful if you want to install the latest, experimental [development code](https://dev.openwrt.org/wiki/GetSource).

To install the development version instead of the current release, replace the image in these instructions with a development image for [ar71xx](http://downloads.openwrt.org/snapshots/trunk/ar71xx/). Look for the file that begins with "**openwrt-ar71xx-generic-wzr-hp-ag300h-squashfs-**" and select the correct version - currently either **tftp**, **sysupgrade** or **factory**. See the [general installation instructions](http://wiki.openwrt.org/toh/buffalo/wzr-hp-ag300h) for guidance on which version to choose. 

If you want to upgrade from the factory-installed DD-WRT to OpenWRT from the web interface, you should probably use the file that ends with "factory" ([this file](http://downloads.openwrt.org/snapshots/trunk/ar71xx/openwrt-ar71xx-generic-wzr-hp-ag300h-squashfs-factory.bin)) - but please double check that this is still correct.

**Note**: I (MS) was unable to upgrade from DD-WRT (upgraded to latest DD-WRT version from dec. 2013) with the *-factory-version of OpenWRT trunk from 31. jan 2014. See the [600DHP instructions](http://wiki.openwrt.org/toh/buffalo/wzr-600dhp) for a workaround with sysupgrade. Change the commands to use the image from trunk instead of the latest relase - but you should probably not use trunk for this! When I used trunk, I lost the web gui and had to telnet to the router and use mtd to reflash the firmware with the 12.09 release (using the command: "mtd -r write [...] firmware"). 

### After installing

If you do not set a root password and enable ssh, telnet will be open WITHOUT login (try telnet 192.168.1.1). Wifi may also be unencrypted / open.

Remember to:

1. Set root password, go to System -> Administration, Router Password, enter password twice, Press "Save & Apply")

2. Make sure that SSH is running on all interfaces (System -> Administration, SSH access, Interface: unspecied, Press "Save & Apply"). This will enable ssh on both the LAN and WAN interface and is generally **insecure**. This is to be able to reach teh wifi routers from the "internet"-side, which in our case is already behind a firewall.

3. To make SSH and Luci (web interface) reachable from the WAN-port, you also have to add two firewall rules in "Network" -> "Firewall" -> "Custom rules" :
```
iptables --append input_wan --protocol tcp --dport 22 --jump ACCEPT
iptables --append input_wan --protocol tcp --dport 80 --jump ACCEPT
```

4. Secure the wifi network. Go to "Network -> Wifi". Edit radio0: Change ESSID if necessary and press "Save / Apply". Go to the "Wireless security"-tab under Interface configuration. Select WPA2-PSK and enter the password.  "Save & Apply". Go back to "Network -> Wifi" and Verify that only radio0 is enabled (2.4 ghz). 

5. Change hostname to the same as the ESSID under System -> System Properties -> General Settings -> Hostname. If the hostname is correct it can be used to e.g. recognize the router when it is configured via dhcp. Put the same hostname under "Network" -> "Interfaces" -> WAN edit-button -> "Hostname to send when requesting DHCP".

6. From the same page as you set the hostname you can also set the correct time. If the router is on the Internet it should update itself automatically, otherwise press the "Sync with Browser" button.

7. Make sure that the correct country is configured in wifi settings and that the transmit power does not exceed 20 dbm (max in Norway). Otherwise it will transmit at 27dbm (0.5W). To set the country, see under Wifi -> radio0 -> edit -> Advanced settings. To set the transmit power, see under Wifi -> radio0 -> edit -> General settings.

8. Configure the LAN interface to a unique IP-address. This is to make it easier to connect the routers as clients/ap's without IP conflicts. E.g. use 192.168.[id].1 as the IP, where IP is a counter. Another alternative is to disable the LAN-interface.


