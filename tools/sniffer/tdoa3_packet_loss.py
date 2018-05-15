#!/usr/bin/env python3
# YAML sniffer that tracks packet loss in the system.
# out of simplicity (and lazyness), reads from STDIN and write to STDOUT,
# just use your shell piping functionalities!
# sniffer_binary -> tdoa3_decoder -> tdoa3_packet_loss

import sys
import yaml
import time

anchor_info = {}


dump_interval = 1
next_dump_time = time.time() + dump_interval

for packet in yaml.load_all(sys.stdin, Loader=yaml.CLoader):
    if not packet:
        continue

    now = time.time()

    id = packet['from']
    seq_nr = packet['seq']
    rx_sys = packet['rxSys']

    if id in anchor_info:
        info = anchor_info[id]

        gap = (seq_nr - info['seq']) & 0x7f
        lost = gap - 1

        info['tot'] += gap
        info['lost'] += lost
    else:
        info = {'seq': 0, 'tot': 1, 'lost': 0, 'start': rx_sys}
        anchor_info[id] = info

    info['seq'] = seq_nr

    if now > next_dump_time:
        next_dump_time = now + 1

        for id, info in anchor_info.items():
            interval = rx_sys - info['start']
            if (interval != 0):
                rate = 1.0
                if (info['tot'] != 0):
                    rate = info['lost'] / (info['tot'] * interval)
                info['loss_rate'] = rate
                info['tot_freq'] = info['tot'] / interval

            info.pop('start')
            info.pop('seq')

        print("---")
        print(yaml.dump(anchor_info, Dumper=yaml.CDumper))

        anchor_info = {}
