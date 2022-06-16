# -*- coding: utf-8 -*-
#
#     ||          ____  _ __
#  +------+      / __ )(_) /_______________ _____  ___
#  | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
#  +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
#   ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
#
#  Copyright (C) 2018 Bitcraze AB
#
#  Crazyflie Nano Quadcopter Client
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA  02110-1301, USA.
"""
Sets the position data in anchors.

Requires a system in TDoA mode and a Crazyflie with the LPS deck.

Use by piping yaml document with positions into the script
cat ./tools/lpp/positions_example.yaml | python3 tools/lpp/set_positions.py
"""
import logging
import time
import struct
import sys
import yaml

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie

if sys.argv[0] == '-h' or sys.argv[0] == '--help':
    print("usage: {} [URI]".format(sys.argv[0]))
    print("  URI - the URI to use to connect to the Crazyflie, default value is  radio://0/80/2M/E7E7E7E7E7")
    sys.exit(1)

uri = 'radio://0/80/2M/E7E7E7E7E7'
if len(sys.argv) > 1:
    uri = sys.argv[1]

# Only output errors from the logging framework
logging.basicConfig(level=logging.ERROR)

# Initialize the low-level drivers (don't list the debug drivers)
cflib.crtp.init_drivers(enable_debug_driver=False)

cf = Crazyflie(rw_cache='./cache')
with SyncCrazyflie(uri, cf=cf) as scf:
    print("Setting positions")

    for packet in yaml.load_all(sys.stdin, Loader=yaml.Loader):
        if not packet:
            continue

        while True:
            for id, position in packet.items():
                x = position['x']
                y = position['y']
                z = position['z']

                print("Anchor {}, ({}, {}, {})".format(id, x, y, z))
                LPP_SHORT_ANCHOR_POSITION = 0x01
                position_pack = struct.pack("<Bfff", LPP_SHORT_ANCHOR_POSITION, x, y, z)
                scf.cf.loc.send_short_lpp_packet(id, position_pack)
                time.sleep(0.01)

            time.sleep(0.5)
