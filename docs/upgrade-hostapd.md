
wnload the source code from [here](http://hostap.epitest.fi/hostapd/) as a tar.gz:

```
mkdir ~/hostapd ; cd ~/hostapd
wget http://hostap.epitest.fi/releases/hostapd-2.0.tar.gz
zcat hostapd-2.0.tar.gz | tar -xv
```

Copy the default configuration to .config:

```
cd ~/hostapd/hostapd-2.0/hostapd
cp defconfig .config
```

If you want to change the default options, edit .config accordingly.

Before we can compile the source code we need libnl-dev and libssl-dev. We also need the compiler tools from build-essential.

```
sudo apt-get install libnl-dev libssl-dev build-essential
```

Now hostapd should compile:

```
cd ~/hostapd/hostapd-2.0/hostapd
make -j3
```

After the installation completes, we install the new binaries manually. Remember to stop hostapd first.

```
killall hostapd
cp ~/hostapd/hostapd-2.0/hostapd/hostapd /usr/sbin
cp ~/hostapd/hostapd-2.0/hostapd/hostapd_cli /usr/sbin
```

You can verify that the old configuration works with the new version by running hostapd from the command line:
```
hostapd /etc/hostapd/hostapd.conf
```

If you are able to connect to the wifi-network and everything seems to work as it should, reboot the device to enable the new hostapd-daemon. 


