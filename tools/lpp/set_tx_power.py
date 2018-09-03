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
Sets the transmit power of the UWB radio chip in LPS Anchors.

Requires a system in TDoA mode and a Crazyflie with the LPS deck.
The script will broadcast power settings through the Crazyflie over and over
again.

IMPORTANT: There are regulations on how much energy a radio source is allowed
to transmit, and the regulations are different from country to country. It
might be possible that some configurations that this script can set are not
within limits in some countries. If you choose to run this script and modify
the transmit power, make sure it is within limits where you will use the
anchor.


"""
import logging
import time
import struct
import sys

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie
import argparse


def to_power_byte(db):
    if db <= 0 or db > 33.5:
        raise Exception("Out of range")

    coarse = 0
    fine = 0

    while ((6 - coarse) * 3) > db and coarse <= 6:
        coarse += 1

    while ((6 - coarse) * 3) + (0.5 * fine) < db:
        fine += 1

    return (coarse << 5) + fine


def generate_power_conf(power):
    power_byte = to_power_byte(power)
    return power_byte + 256 * (
            power_byte + 256 * (power_byte + 256 * power_byte))


def generate_lpp_packet(power_conf, force, smart):
    flags = 0

    if smart:
        flags |= 0x01
    if force:
        flags |= 0x02

    LPP_UWB = 0x04
    return struct.pack("<BBI", LPP_UWB, flags, power_conf)


def validate_anchor_id(id):
    if id < 0 or id > 255:
        raise Exception("Valid anchor ids are 0 - 255")


def generate_config(power):
    DEFAULT_CONFIG = 0x07274767

    power_conf = DEFAULT_CONFIG
    force = False
    smart = True

    if power != 'default':
        power_conf = generate_power_conf(float(power))
        force = True
        smart = False

    return (power_conf, force, smart)


def main():
    parser = argparse.ArgumentParser(
        description='Set transmit power of LPS Anchors. A new power '
                    'configuration is sent to anchors using a Crazyflie with '
                    'an LPS deck as a bridge. IMPORTANT: There are '
                    'regulations on how much energy a '
                    'radio source is allowed to transmit, and the regulations '
                    'are different from country to country. It might be '
                    'possible that some configurations that this script can '
                    'set are not within limits in some countries. If you '
                    'choose to run this script and modify the transmit power, '
                    'make sure it is within limits where you will use the '
                    'anchor.')

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

    valid_powers = list(map(lambda x: ('%.1f' % (x / 2)).replace(".0", ""),
                            range(1, 68)))
    valid_powers.append('default')
    parser.add_argument('power',
                        type=str,
                        choices=valid_powers,
                        help='The transmit power to use. Valid values are '
                             'between 0.5 and 33.5 (dB) in steps of 0.5 or '
                             '"default". "default" will set the power to the '
                             'default power as well as configure the anchor '
                             'to use smart power.')

    args = parser.parse_args()

    unit = 'dB'
    if args.power == 'default':
        unit = ' (smart)'

    validate_anchor_id(args.anchor_id_lower)
    validate_anchor_id(args.anchor_id_upper)
    if args.anchor_id_lower > args.anchor_id_upper:
        raise Exception("(%i to %i) is not a valid anchor id range" % (
            args.anchor_id_lower, args.anchor_id_upper
        ))

    (power_conf, force, smart) = generate_config(args.power)
    power_pack = generate_lpp_packet(power_conf, force, smart)

    print('Sending anchor transmit power configuration to anchors (%i to %i) '
          'using %s. Setting transmit power to %s%s. Anchors will reset'
          'when configured.' % (
              args.anchor_id_lower,
              args.anchor_id_upper,
              args.uri,
              args.power, unit))

    logging.basicConfig(level=logging.ERROR)
    cflib.crtp.init_drivers(enable_debug_driver=False)
    cf = Crazyflie(rw_cache='./cache')
    with SyncCrazyflie(args.uri, cf=cf) as scf:
        print('Starting transmission. Terminate with CTRL+C.')
        while True:
            for id in range(args.anchor_id_lower, args.anchor_id_upper + 1):
                print('Anchor %i' % id)
                for _ in range(7):
                    scf.cf.loc.send_short_lpp_packet(id, power_pack)
                    time.sleep(0.2)
                time.sleep(0.5)


main()
