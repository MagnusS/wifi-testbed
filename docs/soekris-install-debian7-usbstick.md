# Installing Debian 7.3 on Soekris 6501 from a USB flash drive #
These instructions are for installing Debian 7.3 from a USB flash drive on a Soekris 6501-30 with a 30 GB SSD drive. 

To continue you have to have a serial connection to the Soekris box (see details for installing minicom here http://wiki.soekris.info/Connecting_to_the_serial_console). Eth0 should also be connected to a cabled network with DHCP and a working Internet connection.

The USB flash drive has to be supported by the Soekris BIOS. I used a 16 GB Sandisk USBCruzer fit. To check if the USB stick is supported, reboot the Soekris while connected to the serial port and see if it appears with the correct size in the BIOS output. Mine looks like this:

```
Soekris USB Expansion ROM ver. 1.01  20111203

81: USB 01  SanDisk Cruzer Fit      Xlt 1024-255-63  15633 Mbyte

```

### Create USB stick ###

To boot the Debian installer we have to first copy the installer to the USB stick and then change the 
bootloader configuration to send output to the serial port. If you have already completed this step you can skip to the Installation-section below.

(The following instructions are adapted from http://www.debian.org/releases/stable/i386/ch04s03.html.en)

Download boot.img.gz:

ftp://ftp.uninett.no/pub/linux/debian/dists/Debian7.3/main/installer-amd64/current/images/hd-media/

Extract and copy boot.img.gz to the USB device. This will take some time, as the boot image is about 945 MB uncompressed. 

```
zcat boot.img.gz > /dev/sdXX 
```

Replace XX with the correct device (e.g. /dev/sdb1 if the USB stick is /dev/sdb). 

When the command completes you should be able to mount the device.

Download the netinst.iso and copy it to the USB drive (mount it first). I used http://cdimage.debian.org/debian-cd/7.3.0/amd64/iso-cd/debian-7.3.0-amd64-netinst.iso. The file should be called "netinst.iso" when stored on the USB stick.

### Enable serial port output ###
On the USB flash drive, open the file txt.cfg and edit the line that begins with "append". Change the line to:

```
append console=ttyS0,19200n8 vga=normal initrd=initrd.gz
```

Before removing the USB flash drive from the computer, make sure that all changes are written to it. You can do this by executing "sync" or by manually umounting it with "umount".

### Installation ###

Connect the USB flash drive to the Soekris and restart the device (make sure you unmount the USB stick before you remove it from the computer). On the Soekris, press CTRL-P during boot and then use the command "boot 81" to boot from the USB device. If this does not work - i.e. no response - you may have to specify the partition number like this: "boot 81:1".

The bootloader should now appear - or a garbled version of it. Press enter to begin the installation. 

To navigate the menus, use the arrow keys and tab.

Select the correct geographical area, keyboard setup, time zone and so on. When the installer asks for a network connection, select eth0 (eth0 must be connected to a network). 

As the hostname, enter nodeX, where X is the node number. Leave the domain name blank. 

When asked for the root password, set it to a randomly generated password and write it down for now. Add a new user called "testbed" and generate a random password for that user as well (remember to write it down). When the installation completed, the testbed-user will be given sudo-access.

When the partition manager starts, select "Guided - use entire disk", unless you want to partition the disk manually. In the next screen, be careful to select the correct device for the installation (usually /dev/sda). If you used the guided partitioning, choose "All files in on partition" when asked. Complete the partitioning and confirm all changes. The disk will now be formatted and the base installation will begin.

After the base installation is complete, the installer asks for the closest download mirror. Select the country that is closest to you. Choose one of the FTP servers and then enter your HTTP proxy settings. The installation should now continue. If you are asked to participate in a survey, select "no".

When the core system is installed, the installer needs to know what type of system it should configure ("Software selection"). Only leave "SSH server" and "Standard system utilities" selected. Then press continue to proceed with the installation.

When asked, press yes to install GRUB to the boot device - this should be the default. The installation will now complete. Select continue to reboot the device.

After reboot, you should see the GRUB boot menu. Press enter to boot Debian.

Login as "root" with the password generated earlier. Install the sudo-package with 

```
apt-get install sudo
```

Add the testbed-user to the sudo-group with the following command:

```
usermod -a -G sudo testbed
```

The testbed-user should now be able to gain root-privileges with the sudo-command when needed. As the testbed-user has root privileges we can disable remote logins by root over SSH.

Edit /etc/ssh/sshd_config and make sure that "PermitRootLogin" is set to "no". Restart sshd with the command

```
/etc/init.d/ssh restart
```

The installation is now complete.

## Additional setup ##

### SSH keys ###

To automate ssh logins without requiring a password we have to set up key based authentication between the computer that coordinates the testbed ("coordinator") and the soekris-node.

First, we need to create a key on the coordinator. You only have to do this once. If you have already created a key (check if ~/.ssh/id_rsa* exists), you should skip to the scp-command below.

From the terminal, do

```
cd ~/.ssh
ssh-keygen -t rsa -b 4096
```

When asked for a key file name use the default. Press enter when asked for a passphrase to select "No passphrase".

You should now have two files in ~/.ssh called id_rsa (private key) and id_rsa.pub (public key). 

The public key should be appended to the file ~/.ssh/authorized_keys on the soekris-node. As we will only have a single key in this file, we can overwrite it. 

Copy the public key to the authorized_keys-file on the soekris box using scp:

```
scp ~/.ssh/id_rsa.pub testbed@nodeX:.ssh/authorized_keys
```

Replace "nodeX" with the correct hostname or IP. 

Note that if you get an error that .ssh does not exist, you can create it by logging in with ssh to testbed@nodeX (you have to use a password), and create the directory manually:

```
mkdir ~/.ssh
chmod 700 ~/.ssh
```

Logout and run the scp-command again.

Verify that everything works by ssh'ing to the node:

```
ssh testbed@nodeX
```

You should now be able to login without a password.

After verifying that everything works you should disable password logins in /etc/ssh/ssh_config on the soekris-box. Find the line that says "#PasswordAuthentication yes" and replace it with

```
PasswordAuthentication no
```

Restart sshd with

```
sudo /etc/init.d/sshd restart
```

### Disable sudo-password ###
To enable automated sudo commands we need to disable the sudo-password for the testbed-user. It is *strongly* recommended that ssh key authentication is used (described above) and that password logins are disabled before the sudo-password is disabled.

Edit /etc/sudoers and add the following line:

```
testbed ALL=NOPASSWD: ALL
```

The testbed-user can now use sudo without a password. 

### Useful commands ###

To execute "apt-get -y install vim" (or other commands) on all hosts in parallel. Requires SSH key auth + sudo without password:

```
seq 1 21 | parallel -j 30 'ssh testbed@node{} "sudo apt-get -y install vim"'
```


