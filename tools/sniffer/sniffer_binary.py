#!/usr/bin/env python3
# UWB binary sniffer decoder
#
# The LPS node should be in sniffer mode already, it will be switched to
# binary mode by this script.

import sys
import struct
import serial
import binascii

if len(sys.argv) != 2:
    print("usage: {} <sniffer serial port>".format(sys.argv[0]))
    sys.exit(1)

ser = serial.Serial('/dev/ttyACM0')

# Switch to binary mode
ser.write(b'b')

while True:
    c = ser.read(1)

    if c == b'\xbc':
        ts = ser.read(5)
        ts += b'\0\0\0'
        ts = struct.unpack('<Q', ts)[0]
        addrFrom, addrTo = struct.unpack('<BB', ser.read(2))
        length = struct.unpack('<H', ser.read(2))[0]
        if length > 1024:
            continue
        data = ser.read(length)
        l2 = struct.unpack('<H', ser.read(2))[0]
        if length == l2:
            print("@{:010x} from {} to {}: {}".format(ts, addrFrom, addrTo,
                                                      binascii.hexlify(data)
                                                      .decode('utf8')))
        else:
            print("Out of sync!")
