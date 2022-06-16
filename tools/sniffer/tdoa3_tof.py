#!/usr/bin/env python3
# YAML sniffer that extracts tof data from tdoa3 parsed yaml documents.
# out of simplicity (and lazyness), reads from STDIN and write to STDOUT,
# just use your shell piping functionalities!
# sniffer_binary -> tdoa3_decoder -> tdoa3_tof

import sys
import yaml
import struct

unit = 'ticks'

ANTENNA_OFFSET = 154.6
LOCODECK_TS_FREQ = 499.2e6 * 128
SPEED_OF_LIGHT = 299792458.0
M_PER_TICK = SPEED_OF_LIGHT / LOCODECK_TS_FREQ

if len(sys.argv) > 1:
    for arg in sys.argv[1:]:
        if arg == 'm':
            unit = 'meters'
        elif arg == 't':
            unit = 'ticks'
        else:
            print("usage: {} [unit]".format(sys.argv[0]))
            print("  unit: t = ticks (default), m = meters (excluding antenna delay)")
            sys.exit(1)


for packet in yaml.load_all(sys.stdin, Loader=yaml.Loader):
    if not packet:
        continue

    data = {'id': packet['from'], 'tof': {}}

    for remote in packet['remoteAnchorData']:
        if 'distance' in remote:
            tof = remote['distance']
            remote_id = remote['id']
            if unit == 'ticks':
                data['tof'][remote_id] = tof
            if unit == 'meters':
                data['tof'][remote_id] = tof * M_PER_TICK - ANTENNA_OFFSET

    print("---")
    print(yaml.dump(data, Dumper=yaml.Dumper))
