# Recompiling the kernel with IEEE 802.11 debugfs-support and Minstrel output #

To enable debugging in the ath9k driver (used for Atheros 9280), we have to recompile the driver from source as it is not enabled in Debian by default. This is required to view debug output from the [Minstrel](http://wireless.kernel.org/en/developers/Documentation/mac80211/RateControl/minstrel/) rate control algorithm.

### Recompiling the kernel ###
This step will take a while to complete as the kernel takes a while to compile on the net6501. These instructions are adapted from the [Debian Administrator Handboook](http://debian-handbook.info/browse/stable/sect.kernel-compilation.html) and [this page](http://www.tecmint.com/kernel-compilation-in-debian-linux/).

Note that if you have already completed this step for a different Soekris-box you can copy the *.deb-packages and install them without having to recompile the kernel. You can then skip to the next section that describes how to mount the debugfs.

If you have a PC with Debian or Ubuntu you can compile the kernel on the PC instead and just copy and install the final .deb-packages to the Soekris-box. Otherwise the instructions below are the same on a PC. Just remember to copy the .config-file for the kernel from the /boot-directory on the Soekris-box, not the PC.

To speed I/O access up a bit if you are compiling directly on the Soekris-box, you should disable access time recording temporarily on the root file system:
```
sudo mount -o remount,noatime /
```

First, we need to install the the necessary packages to compile the kernel source:

```
apt-get install fakeroot kernel-package build-essential libncurses5-dev linux-source
```

You can either use the source that comes with Debian (alt 1) or the latest version from kernel.org (alt 2). To use spectral scanning with the ath9k driver the 3.2 kernel included in Debian 7.3 is too old. If you are running at later version of Debian however, the kernel may be newer.

### Alt 1: Use Debian kernel source code ###
Install the source code in /usr/src:

```
sudo apt-get install linux-source
```

As a normal user (root is not required), run:

```
mkdir ~/kernel; cd ~/kernel
tar -xjf /usr/src/linux-source-3.2.tar.bz2
```

Replace 3.2 with the version you downloaded.

### Alt 2: Download latest kernel ###
Download the latest *stable* kernel from [www.kernel.org](http://www.kernel.org) using wget. In my case, 3.12.8 was the latest stable version. 

```
sudo wget https://www.kernel.org/pub/linux/kernel/v3.x/linux-3.12.8.tar.xz -O /usr/src/linux-source-3.12.8.tar.xz
```

Now extract the kernel:

```
mkdir ~/kernel; cd ~/kernel
xzcat /usr/src/linux-source-3.12.8.tar.bz2 | tar -xv
```

This should create the directory ~/kernel/linux-X.XX.X/, where X.XX.X is the version number.

### Compiling the kernel ###
You should now have a directory ~/kernel with the kernel source in it. This document was originally written for kernel version 3.2, but you may have downloaded a newer version. In the file names used in the remainder of this document, replace 3.2 with the version you downloaded.

To copy the current kernel's configuration from /boot, use the following command:

```
cp /boot/config-`uname -r` ~/kernel/linux-source-3.2/.config
```

"uname -r" expands to the running kernel's release number, e.g. "3.2.0-4-amd64".

If the config-file you copied from /boot is from an older kernel than the one you are compiling, you should run "make olddefconfig" before you continue to import the old configuration. The command will automatically choose the default options for any new features. It is usually safe to use the default options, but you can  use "make oldconfig" instead if you want to set the options manually.

Now start the console based kernel configuration menu:
```
cd ~/kernel/linux-source-3.2
make menuconfig
```

When the configuration menu appears, select "Networking support --->". To speed up compilation you may *optionally* at least disable "Amateur radio support", "CAN bus subsystem support", "IrDA (infrared) subsystem support", "Bluetooth subsystem support", "RxRPC session sockets" and "NFC subsystem support". 

From the main menu, select "Wireless --->" and then enable the option that says "Export mac80211 internals in DebugFS". Go back to the main menu and select "Processor type and features --->". Choose "Processor family --->" and select "Intel Atom".

If you need the ath9k-driver (required for e.g. atheros 9280), also make sure that the Atheros driver is enabled. From the main menu, choose "Device Drivers  --->", "Network device support  --->", "Wireless LAN  --->" and enable "Atheros Wireless Cards". Within "Atheros Wireless Cards" select "Atheros 802.11n wireless cards support" and all sub-options. You may also want to enable "Atheros Wireless Debugging". 

For IPv4 NAT, verify that "Networking support --->", "Networking options --->", "Network packet filtering framework (Netfilter) --->", "IP: Netfilter configuration --->", "IPv4 NAT" and the options below are selected.

Exit the menu and save the configuration.

Before we compile the kernel, clean the kernel source tree
```
make-kpkg clean
```

You can now start the compilation of the new kernel with

```
export CONCURRENCY_LEVEL=3
fakeroot make-kpkg --append-to-version "-mac80211debug" --revision "1" --initrd kernel_image kernel_headers
```

I would recommend running make in a [tmux](http://en.wikipedia.org/wiki/Tmux) session to be able to disconnect while the compilation is running, but this is optional and not covered in this guide. 

The kernel compilation will take some time to complete and this is a good time to go for a coffee or do something else for a few hours. When I compiled the 3.2 kernel on a 600 MHz net6501 it took close to 12 hours with kernel 3.2.51. 

When the compilation completes two new .deb-packages should be available in ~/kernel, called linux-headers*.deb and linux-image-*.deb. Use "dpkg -i" to install them, e.g:

```
dpkg -i linux-headers*.deb linux-image*.deb
```

If the packages are installed without errors, reboot the device to use the new kernel. You can verify that the new kernel is running after reboot with "uname -r". The text string given to the "append-to-version" parameter of make-kpkg above should appear after the kernel version number:

```
testbed@node1:~/kernel$ uname -r
3.2.51-mac80211debug
```

You should now be able to mount the debug filesystem in /sys/debug/. 

Note that these packages can be copied to other routers (e.g. with a USB flash drive or scp) and installed directly without having to recompile the kernel on each device.

### Mounting debugfs ###
To monitor the rate control algorithm [Minstrel](http://wireless.kernel.org/en/developers/Documentation/mac80211/RateControl/minstrel/), the [debugfs](http://en.wikipedia.org/wiki/Debugfs) must be mounted and debugging must be enabled in the wifi-driver. 

You can use the following command to temporarily mount the debugfs:

```
mount -t debugfs none /sys/kernel/debug
```

To mount it automatically after reboot, add the following line to /etc/fstab:

```
nodev /sys/kernel/debug   debugfs   defaults   0  0
```

After debugging has been enabled and debugfs has been mounted, Minstrel statistics can be gathered from the files in /sys/kernel/debug/ieee80211/phy0/netdev:wlan0.

In the "stations" subdirectory, each connected device will have its own directory with additional statistics.

This is the content of the stations-directory on my Soekris-box with one connected client with MAC-address 30:10:xx:xx:xx:xx

```
root@node1:/sys/kernel/debug/ieee80211/phy0/netdev:wlan0/stations# ls -la
total 0
drwxr-xr-x 3 root root 0 Jan 21 14:16 .
drwxr-xr-x 3 root root 0 Jan 21 14:16 ..
drwxr-xr-x 2 root root 0 Jan 21 15:50 30:10:xx:xx:xx:xx
```

Within the 30:10:xx:xx:xx:xx-directory there are several files:

```
root@node1:/sys/kernel/debug/ieee80211/phy0/netdev:wlan0/stations/30:10:xx:xx:xx:xx# ls -la
total 0
drwxr-xr-x 2 root root 0 Jan 21 15:50 .
drwxr-xr-x 3 root root 0 Jan 21 14:16 ..
-r-------- 1 root root 0 Jan 21 15:50 agg_status
-r-------- 1 root root 0 Jan 21 15:50 connected_time
-r-------- 1 root root 0 Jan 21 15:50 dev
-r-------- 1 root root 0 Jan 21 15:50 flags
-r-------- 1 root root 0 Jan 21 15:50 ht_capa
-r-------- 1 root root 0 Jan 21 15:50 inactive_ms
-r-------- 1 root root 0 Jan 21 15:50 last_seq_ctrl
-r-------- 1 root root 0 Jan 21 15:50 last_signal
-r-------- 1 root root 0 Jan 21 15:50 num_ps_buf_frames
-r--r--r-- 1 root root 0 Jan 21 15:50 rc_stats
-r-------- 1 root root 0 Jan 21 15:50 rx_bytes
-r-------- 1 root root 0 Jan 21 15:50 rx_dropped
-r-------- 1 root root 0 Jan 21 15:50 rx_duplicates
-r-------- 1 root root 0 Jan 21 15:50 rx_fragments
-r-------- 1 root root 0 Jan 21 15:50 rx_packets
-r-------- 1 root root 0 Jan 21 15:50 tx_bytes
-r-------- 1 root root 0 Jan 21 15:50 tx_filtered
-r-------- 1 root root 0 Jan 21 15:50 tx_fragments
-r-------- 1 root root 0 Jan 21 15:50 tx_packets
-r-------- 1 root root 0 Jan 21 15:50 tx_retry_count
-r-------- 1 root root 0 Jan 21 15:50 tx_retry_failed
-r-------- 1 root root 0 Jan 21 15:50 wep_weak_iv_count
```

The rate control statistics should be in a file called "rc_stats". Its contents is updated automatically by the wifi driver. Here is the output for station 30:10:xx:xx:xx:xx:

```
root@node1:/sys/kernel/debug/ieee80211/phy0/netdev:wlan0/stations/30:10:e4:31:1c:d2# cat rc_stats 
rate     throughput  ewma prob   this prob  this succ/attempt   success    attempts
     1         0.7       82.2      100.0          0(  0)         13          13
     2         0.0        0.0        0.0          0(  0)          0           0
     5.5       0.0        0.0        0.0          0(  0)          0           0
    11         0.0        0.0        0.0          0(  0)          0           0
     6         0.0        0.0        0.0          0(  0)          0           0
     9         0.0        0.0        0.0          0(  0)          0           0
    12         9.6       86.6      100.0          0(  0)         35          35
    18         0.0        0.0        0.0          0(  0)          0           0
    24         5.3       25.0      100.0          0(  0)          1           1
    36         7.6       25.0      100.0          0(  0)          1           1
T P 48        38.4       98.3      100.0          2(  2)        172         174
 t  54        18.9       43.7      100.0          0(  0)          2           2

Total packet count::    ideal 203      lookaround 22
```

For a detailed description of the output format see the [Minstrel documentation](http://wireless.kernel.org/en/developers/Documentation/mac80211/RateControl/minstrel/).

