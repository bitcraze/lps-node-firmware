#!/usr/bin/env python3
# UWB binary sniffer decoder
#
# The LPS node should be in sniffer mode already, it will be switched to
# binary mode by this script.


# To use this script: python sniffer_binary.py /dev/ttyACM0 yaml

# Readme: https://wiki.bitcraze.io/doc:lps:toda3
# TDoA 3 parsing
# Pipe the yaml into the TDoA 3 decoder to see TDoA 3 data unpacked

# example: python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml | python3 tools/sniffer/tdoa3_decoder.py

# This will show all data that the sniffer receives. To limit the output it is possible to filter which anchors to show. Add one or more ids at the end to only show data from only these anchors.

# example: python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml | python3 tools/sniffer/tdoa3_decoder.py 2 3

# Time of flight and anchor to anchor distance
# By piping the TDoA 3 data into the TOF tool, the measured anchor to anchor distances will be displayed. By adding the 'm' argument the output will be in meters instead of UWB clock ticks.

# example: python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml | python3 tools/sniffer/tdoa3_decoder.py | python3 tools/sniffer/tdoa3_tof.py m


import sys
import struct
import serial
import binascii
import yaml
import time

if len(sys.argv) < 2:
    print("usage: {} <sniffer serial port> [format]".format(sys.argv[0]))
    print("  Possible format: human (default), csv, yaml")
    sys.exit(1)

ser = serial.Serial(sys.argv[1], 9600)

outputFormat = 'human'
if len(sys.argv) > 2:
    outputFormat = sys.argv[2].strip()

# Switch to binary mode
ser.write(b'b')

while True:
    c = ser.read(1)
    if c == b'\xbc':      # the anchor is in sniffer mode
        ts = ser.read(5)
        ts += b'\0\0\0'
        ts = struct.unpack('<Q', ts)[0]
        addrFrom, addrTo = struct.unpack('<BB', ser.read(2))
        length = struct.unpack('<H', ser.read(2))[0]
        # print(struct.unpack('<H', ser.read(2))[0])
        if length > 1024:
            continue                     # continues the while loop
        data = ser.read(length)
        l2 = struct.unpack('<H', ser.read(2))[0]
       
        if length == l2:
            now = time.time()
            if outputFormat == 'human':
                print("@{:010x} from {} to {}: {}".format(ts, addrFrom, addrTo,
                      binascii.hexlify(data)
                      .decode('utf8')))
            elif outputFormat == 'csv':
                print("0x{:010x}, {}, {}, {}".format(ts, addrFrom, addrTo,
                      binascii.hexlify(data)
                      .decode('utf8')))
            elif outputFormat == 'yaml':
                print("---")
                print(yaml.dump({'ts': ts, 'from': addrFrom,
                                 'to': addrTo, 'data': data, 'rxSys': now},
                                Dumper=yaml.CDumper))
            else:
                sys.stderr.write("Error: Uknown output format: {}\n".format(
                                 outputFormat))

        else:
            print("Out of sync!")
