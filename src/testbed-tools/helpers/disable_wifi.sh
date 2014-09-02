#!/bin/sh

echo "Disabling wifi on all nodes"

seq 1 21 | parallel -j 30 "./wifictl.py wifi --disable {}"
