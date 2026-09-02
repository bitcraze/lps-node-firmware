# -*- coding: utf-8 -*-
#
#     ||          ____  _ __
#  +------+      / __ )(_) /_______________ _____  ___
#  | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
#  +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
#   ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
#
#  Copyright (C) 2026 Bitcraze AB
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
Sets the list of anchor ids that a TDoA3 anchor should ignore (no ranging,
no relaying, no TDoA pairs formed with them).

Requires a system in TDoA3 mode and a Crazyflie with the LPS deck. The
exclusion list is only meaningful in TDoA3 mode; it is stored in EEPROM and
persists across reboots. Sending an empty list clears all exclusions for
that anchor.

Note: to fully suppress a pair (e.g. 6-7), set the exclusion on BOTH
anchors (exclude 7 on anchor 6, AND exclude 6 on anchor 7) - excluding it
on only one side may not be enough, since the other anchor may still relay
data about its peer and let the Crazyflie form the pair from that.

Usage:
  python3 tools/lpp/set_excluded_anchors.py <target_anchor_id> <id_to_exclude> [<id_to_exclude> ...]
  python3 tools/lpp/set_excluded_anchors.py 6 7 9
  python3 tools/lpp/set_excluded_anchors.py 6      # clears the list on anchor 6
"""
import logging
import struct
import sys
import time

import cflib.crtp
from cflib.crazyflie import Crazyflie
from cflib.crazyflie.syncCrazyflie import SyncCrazyflie

TDOA3_EXCLUDED_ANCHORS_MAX_COUNT = 16
LPP_SHORT_TDOA3_EXCLUDED_ANCHORS = 0x06


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    target_id = int(sys.argv[1])
    excluded_ids = [int(a) for a in sys.argv[2:]]

    if len(excluded_ids) > TDOA3_EXCLUDED_ANCHORS_MAX_COUNT:
        raise Exception("Too many excluded anchors, max is {}".format(
            TDOA3_EXCLUDED_ANCHORS_MAX_COUNT))

    uri = 'radio://0/80/2M/E7E7E7E7E7'

    logging.basicConfig(level=logging.ERROR)
    cflib.crtp.init_drivers(enable_debug_driver=False)

    packet = struct.pack(
        "<B{}B".format(len(excluded_ids)),
        LPP_SHORT_TDOA3_EXCLUDED_ANCHORS,
        *excluded_ids)

    cf = Crazyflie(rw_cache='./cache')
    with SyncCrazyflie(uri, cf=cf) as scf:
        print("Setting excluded anchors {} on anchor {}".format(
            excluded_ids, target_id))
        # Sent a few times since this is fire-and-forget over an unreliable
        # radio link, same approach as the other set_*.py scripts.
        for _ in range(10):
            scf.cf.loc.send_short_lpp_packet(target_id, packet)
            time.sleep(0.1)


main()
