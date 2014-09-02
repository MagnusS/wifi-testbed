#!/usr/bin/env python
'''
(c) Copyright 2014 Magnus Skjegstad / Forsvarets forskningsinstitutt
'''

import argparse
parser = argparse.ArgumentParser(description="Utility used to plot network topologies from file or stdin. Requires nodes.json to exist and contain node metadata. Reads data from stdin if no topology file is specified.")
parser.add_argument("-f",  metavar="topology_file", help="topology and network configuration in json format", required=False)
arg = parser.parse_args()

import json
import sys
with open("nodes.json", "r") as f:
    nodes = json.load(fp=f)

if arg.f:
    with open(arg.f, "r") as f:
        topology = json.load(fp=f)
else:
    topology = json.load(sys.stdin)

# channel colors - see http://matplotlib.org/api/pyplot_api.html#matplotlib.pyplot.plot for more styles
channel_style = ["k:"] * 15 # undefined channel is black, dotted line
channel_style[1] = "b-" # solid lines are distinct channel
channel_style[2] = "b--" 
channel_style[3] = "m-."
channel_style[4] = "m:"
channel_style[5] = "r--"
channel_style[6] = "r-"
channel_style[7] = "r-."
channel_style[8] = "g:"
channel_style[9] = "g-."
channel_style[10] = "g--"
channel_style[11] = "y-"
channel_style[12] = "y--"
channel_style[13] = "y:"


# create array for each coordinate. As nodeid != array position, keep track of order in arrays "n" and "keys"
x = []
y = []
z = []
n = []
keys = {}
i = 0
for key in nodes:
    while len(x) <= i:
        x.append(0)
        y.append(0)
        z.append(0)
        n.append(0)

    x[i] = nodes[key]["location"][0]
    y[i] = nodes[key]["location"][1]
    z[i] = nodes[key]["location"][2]
    n[i] = key # node id
    keys[key] = i # position of key

    i = i + 1

from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# plot nodes
ax.plot(x,y,z,'ro')

# plot node id labels
for i in range(0,len(n)):
    ax.text(x[i], y[i], z[i]+0.3, n[i])

# plot edges between connected nodes
for t in topology:
    if topology[t]["enabled"] and topology[t]["ap"]:
        ax.text(x[keys[t]], y[keys[t]], z[keys[t]]-0.4,"AP")
        for c in topology[t]["clients"]:
            print channel_style[topology[t]["channel"]]
            try:
                channel = topology[t]["channel"]
            except KeyError:
                channel = 0 # use style from channel 0 if not defined in json

            ax.plot( [x[keys[t]], x[keys[str(c)]]],
                     [y[keys[t]], y[keys[str(c)]]],
                     [z[keys[t]], z[keys[str(c)]]], channel_style[channel])

plt.show()
