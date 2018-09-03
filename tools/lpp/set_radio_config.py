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
Sets the bitrate and preamble length of the UWB radio chip in LPS Anchors.

Requires a system in TDoA3 mode and a Crazyflie with the LPS deck.

The script will broadcast configuration settings through the Crazyflie over
and over again.


"""
import logging
import time
import struct
import sys

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
import argparse


def validate_anchor_id(id):
    if id < 0 or id > 255:
        raise Exception("Valid anchor ids are 0 - 255")


def main():
    parser = argparse.ArgumentParser(
        description='Set bitrate and preamble length of LPS Anchors. A new '
                    'configuration is sent to anchors using a Crazyflie with '
                    'an LPS deck as a bridge.')

    parser.add_argument('-u', '--uri',
                        dest='uri',
                        default='radio://0/80/2M/E7E7E7E7E7',
                        help='The URI of the Crazyflie (default '
                             'radio://0/80/2M/E7E7E7E7E7)')

    parser.add_argument('-b', '--begin',
                        dest='anchor_id_lower',
                        default=0,
                        type=int,
                        help='The lower id in the anchor range to send to '
                             '(default 0)')

    parser.add_argument('-e', '--end',
                        dest='anchor_id_upper',
                        default=7,
                        type=int,
                        help='The upper id in the anchor range to send to '
                             '(default 7)')

    parser.add_argument('-r', '--rate',
                        dest='low_bitrate',
                        action='store_true',
                        help='Configure low bitrate. Normal bitrate will be '
                             'used if this switch is not set')

    parser.add_argument('-p', '--preamble',
                        dest='long_preamble',
                        action='store_true',
                        help='Configure long preamble. Normal preamble will '
                             'be used if this switch is not set')

    args = parser.parse_args()

    validate_anchor_id(args.anchor_id_lower)
    validate_anchor_id(args.anchor_id_upper)
    if args.anchor_id_lower > args.anchor_id_upper:
        raise Exception("(%i to %i) is not a valid anchor id range" % (
            args.anchor_id_lower, args.anchor_id_upper
        ))

    print('Sending anchor radio configuration to anchors (%i to %i) '
          'using %s.' % (
        args.anchor_id_lower,
        args.anchor_id_upper,
        args.uri))
    print('Setting %s bitrate and %s preamble. '
          'Anchors will reset when configured.' % (
              ['normal', 'low'][args.low_bitrate],
              ['normal', 'long'][args.long_preamble]))

    logging.basicConfig(level=logging.ERROR)
    cflib.crtp.init_drivers(enable_debug_driver=False)
    cf = Crazyflie(rw_cache='./cache')
    with SyncCrazyflie(args.uri, cf=cf) as scf:
        LPP_SHORT_UWB_MODE = 0x05
        flags = args.low_bitrate * 1 + args.long_preamble * 2
        packet = struct.pack("<BB", LPP_SHORT_UWB_MODE, flags)

        print('Starting transmission. Terminate with CTRL+C.')
        while True:
            for id in range(args.anchor_id_lower, args.anchor_id_upper + 1):
                print('Anchor %i' % id)
                for _ in range(7):
                    scf.cf.loc.send_short_lpp_packet(id, packet)
                    time.sleep(0.2)
                time.sleep(0.5)

main()
