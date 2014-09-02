#!/usr/bin/env python

'''
(c) Copyright 2014, Magnus Skjegstad / Forsvarets forskningsinstitutt
(c) Copyright 2013, Halvdan Hoem Grelland / Forsvarets forskningsinstitutt (wifihelpers)
'''

import paramiko
class WifiHelpers(object):
    """ Static helper methods for calculating wifi-related values """

    @staticmethod
    def chan_to_freq(channel):
        """Convert a channel number (1->14) to WiFi channel in the 2.4 Ghz spectrum."""
        """Return type is an int given in Mhz (4 digits)."""
        freq_lookup = [0, 2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462, 2467, 2472, 2484]
        
        # Basic sanity checks
        assert isinstance(int, channel)
        assert channel in range(1,14)
        
        return freq_lookup[channel]

    @staticmethod
    def freq_to_chan(freq):
        """Convert a frequency value (4-digit integer [Mhz]) to wifi channel"""
        """Returns a channel number in the range from 1 to 14 (2.4 Ghz spectrum). None else."""
        if freq < 10:
            freq = int(freq * 1000) # convert from float if necessary

        freq_lookup = [0, 2412, 2417, 2422, 2427, 2432, 2437, 2442, 2447, 2452, 2457, 2462, 2467, 2472, 2484]
        
        try:
            chan = freq_lookup.index(freq)
        except ValueError:
            chan = None
            
        return chan

    # Simple enum-like container
    @staticmethod
    def enum(**enums):
        return type('Enum', (), enums)

    @staticmethod
    def get_numbers_from_str(string):
        import re
        numbers = re.findall(r"[-+]?\d+", string)
        return numbers

class General:
    @classmethod
    def get_info_object(cls,connection):
        if cls.is_openwrt(connection):
            import openwrt
            return openwrt.Info(connection)
        else:
            return False

    @classmethod
    def is_openwrt(cls,connection):
        try:
            stdin, stdout, stderr = connection.exec_command('. /etc/openwrt_release && echo $DISTRIB_ID')
        except Exception as e:
            return False

        stdout.readlines()

        if stdout.channel.recv_exit_status() != 0 or stdout.readline().strip() == "OpenWrt":
            return False

        return True

    @classmethod
    def get_platform(cls,connection):
        if cls.is_openwrt(connection):
            return "OpenWrt"
        return "Unknown"
