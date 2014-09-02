#!/usr/bin/env python

'''
(c) Copyright 2014 Magnus Skjegstad / Forsvarets forskningsinstitutt

Author: Magnus Skjegstad
'''

import argparse
import sys
import ast

DEBUG=True

def debug(say):
    if DEBUG:
        print "DEBUG:",say

def read_scan(scanfile):
    results = []
    lines=0
    with open(scanfile, "r") as f:
        for line in f:
            lines = lines + 1
            scan = ast.literal_eval(line) # read scan from file as dict
            nodeid = scan[ 'scan' ][0][ 'scanner_id' ]
            while len(results) < nodeid:
                results.append([])
            results[nodeid-1].append(scan)
    return results, lines

''' Create lookup table for bss -> ssid '''
def get_bss_lookup(scanresult):
    bss = {} # bss lookup table
    
    for noderesult in scanresult:
        for allscans in noderesult:
            for singlescan in allscans['scan']:
                bss[singlescan['bss']] = singlescan['ssid']

    return bss

''' Create array with obeservation data. E.g. result[nodeid][bss=xx][signal=xx,time=xx] '''
def get_observations(scanresult, ignore_no_obs=True, convert_to_mw=False):
    stats = []
    obs = {}

    for noderesult in scanresult:
        results = None
        nodeid = -1

        for f in noderesult:
            t = f['time']
            for s in f['scan']:
                nodeid = int(s['scanner_id'])
                if results == None: # get existing results for this scanner or create new key for nodeid in obs
                    try:
                        results = obs[nodeid]
                    except:
                        obs[nodeid] = {}
                        results = obs[nodeid]

                try:
                    obs_for_bss = results[s['bss']]
                except:
                    obs_for_bss = []
                    results[s['bss']] = obs_for_bss

                if not ignore_no_obs or s['signal'] > -100:
                    signal = float(s['signal'])
                    if convert_to_mw:
                        signal = 10**((signal+30)/10) # convert from dbm to microwatt. Assumes signal is in dbm!
                    obs_for_bss.append(signal)

    return obs

parser = argparse.ArgumentParser(description='Utility used to analyze scan output files from wifictl. Matrices for avg, mean, stddev and number of observations are displayed.')
parser.add_argument("scanfile", type=str, help="File to analyze")
parser.add_argument("--dbmtomw", action="store_true", help="Attempt to convert dBm values to mW by using the equation mW=10^((dbm+30)/10)")
parser.add_argument("--shownodes", type=str, help="List of nodes to show all samples from/to. Separated by , and no space - e.g. 1,2,3")

args = parser.parse_args()
debug("Arguments: %s" % args)

print "# Reading data..."
result, lines = read_scan(args.scanfile)

print "# Number of measurements:",lines

print "# Creating BSS lookup..."
bss_lookup= get_bss_lookup(result)
ssid_lookup = {}
for bss in bss_lookup:
    ssid_lookup[bss_lookup[bss]] = bss

print "# Reformatting data..."
if args.dbmtomw:
    print "# (converting dbm to microwatt)"
observations = get_observations(result, convert_to_mw = args.dbmtomw)

import numpy as np

mat_mean = []
mat_med = []
mat_len = []
mat_std = []
obs_cnt = 0

for observer in range(1,22):
    for j in range(1,22):
        bss = ssid_lookup["node%d-wifi" % j]
        try:
            obs_cnt = obs_cnt + len(observations[observer][bss])
            mat_mean.append(np.mean(observations[observer][bss]))
            mat_len.append(len(observations[observer][bss]))
            mat_std.append(np.std(observations[observer][bss], ddof=1))
            mat_med.append(np.median(observations[observer][bss]))
        except KeyError:
            mat_mean.append(0)
            mat_len.append(0)
            mat_std.append(0)
            mat_med.append(0)

print "# Total observations:", obs_cnt

print "# MEAN"
print mat_mean
print "# MEDIAN"
print mat_med
print "# OBSERVATION COUNT"
print mat_len
print "# STDDEV w/DDOF=1"
print mat_std

if args.shownodes != None:
    nodes = map(int, args.shownodes.split(","))
    print "# Showing samples for nodes %s" % nodes
    for observer in nodes:
        for j in nodes:
            bss = ssid_lookup["node%d-wifi" % j]
            print "# %d->%d" % (observer, j)
            try:
                print observations[observer][bss]
            except KeyError:
                print []


