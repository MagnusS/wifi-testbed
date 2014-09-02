#!/bin/sh

# This script requires that hostapd has already been copied to ~/hostapd-2.0.tar.gz on each node.
# Can be uploaded and run remotely with wifictl.py remoterun
# Download from wget http://hostap.epitest.fi/releases/hostapd-2.0.tar.gz

hostapd_archive=~/hostapd-2.1.tar.gz

if [ -e "$hostapd_archive" ]; then
    mkdir ~/hostapd
    sudo apt-get -y install libnl-dev libssl-dev build-essential && \
    cd ~/hostapd && \
    zcat "$hostapd_archive" | tar -xv && \
    cd ~/hostapd/hostapd-2.1/hostapd && \
    cat defconfig | sed "s/#CONFIG_ACS/CONFIG_ACS/g" > .config && \
    make clean && \
    make -j3 && \
    sudo cp /usr/sbin/hostapd /usr/sbin/hostapd_orig && \
    sudo cp /usr/sbin/hostapd_cli /usr/sbin/hostapd_cli_orig && \
    sudo cp ~/hostapd/hostapd-2.1/hostapd/hostapd /usr/sbin && \
    sudo cp ~/hostapd/hostapd-2.1/hostapd/hostapd_cli /usr/sbin && \
    echo "Installation complete. Source compiled in ~/hostapd/hostapd-2.1, binaries are installed system-wide. You should disable/enable wifi or reboot." || \
    echo "Installation failed"
else
    echo "$hostapd_archive does not exist. Aborting..."
fi

