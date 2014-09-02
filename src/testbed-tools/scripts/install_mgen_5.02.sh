#!/bin/sh

# This script requires that mgen has already been copied to ~/src-mgen-5.02.tar.gz on each node.
# Can be uploaded and run remotely with wifictl.py remoterun
# Download from wget http://downloads.pf.itd.nrl.navy.mil/mgen/linux-mgen-5.02.tar.gz
mgen_archive=~/src-mgen-5.02.tar.gz

if [ -e "$mgen_archive" ]; then
    mkdir -p ~/mgen && \
    sudo apt-get -y install build-essential && \
    cd ~/mgen && \
    zcat "$mgen_archive" | tar -xv && \
    cd src-mgen-5.02/mgen/protolib/makefiles && \
    make -f Makefile.linux && \
    cd ../../makefiles && \
    make -f Makefile.linux && \
    sudo cp mgen /usr/bin && \
    echo "Installation complete. Source compiled in ~/src-mgen-5.02/mgen, binaries are installed system-wide." || \
    echo "Installation failed"
else
    echo "$mgen_archive does not exist. Aborting..."
fi

