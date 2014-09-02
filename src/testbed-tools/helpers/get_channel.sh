#!/bin/sh

echo "Getting channels..."

seq 1 21 | parallel -j 30 "./wifictl.py wifi --get_channel {}"
