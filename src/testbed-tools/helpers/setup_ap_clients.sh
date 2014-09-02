#!/bin/sh

channel=1
networks="1:2 3:4 5:13 7:8 9:10 6:14 15:16 17:20 19:18 11:12 21"
echo "Note: One node 18 has 0 clients."

echo "Configuring $networks with channel $channel"

for f in $networks; do
    echo $f
done | parallel -j 30 "./wifictl.py setup --ap $channel {}"


