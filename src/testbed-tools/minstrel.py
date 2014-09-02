#!/usr/bin/env python 

'''
Copyright (c) 2013-2014, Magnus Skjegstad (magnus@skjegstad.com) / FFI
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
'''

class Minstrel(object):

    def __init__(self, minstrel_path, executor):
        """ Initialize with radio_device (radio0) + wifi_device (wlan0) """
        self.minstrel_path = minstrel_path
        self.executor = executor

    def get_frame_error_rates(self):
        import glob
        import os
        result = {}
        for f in self.executor.execute_cmd(["sudo", "ls", self.minstrel_path + "/"]).split("\n"):
            if f.strip() == "": # skip blank lines
                continue

            data = self.executor.execute_cmd(["sudo", "cat", self.minstrel_path + "/" + f.strip() + "/rc_stats", "|", "grep", "-v", "Total"])
            success = 0
            tot_success = 0
            attempts = 0
            tot_attempts = 0

            from utils import WifiHelpers

            for line in data.split("\n"):
                tx_vs_attempts = WifiHelpers.get_numbers_from_str(line)[-4:] # last four numbers from rc_stats are current success and attempts, and total success and attemps
                if len(tx_vs_attempts) == 4:
                    success = success + int(tx_vs_attempts[0])
                    attempts = attempts + int(tx_vs_attempts[1])
                    tot_success = tot_success + int(tx_vs_attempts[2])
                    tot_attempts = tot_attempts + int(tx_vs_attempts[3])

            tot_ratio = 1.0
            if tot_attempts > 0:
                tot_ratio = float(tot_success) / float(tot_attempts)

            ratio = 1.0
            if attempts > 0:
                ratio = float(success) / float(attempts)

            result[f] = {'current' : ratio, 'avg' : tot_ratio} 

        return result

    def get_tx_power(self):
        import glob 
        import os
        data = self.executor.execute_cmd(["cat", self.minstrel_path + "/../../power"])
        return data


