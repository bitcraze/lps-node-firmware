#!/usr/bin/env python3

import serial
import sys
import serial.tools.list_ports
import time

ports = serial.tools.list_ports.comports()
nodes = list(filter(lambda p: p.description == "Loco Positioning Node", ports))

if len(nodes) > 0:
    print("Resetting node to DFU ...")
    try:
        ser = serial.Serial(nodes[0].device)
        ser.write(b'u')
        ser.close()
        time.sleep(2)
    except serial.SerialException:
        print("Error: Cannot open node serial port!")
        sys.exit(1)
    sys.exit(0)
else:
    print("Cannot find node, already in DFU mode?")
    sys.exit(0)
