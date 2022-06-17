#!/usr/bin/env python3
# YAML sniffer log TDOA2 decoder, generates a stream of YAML documents
# containing the decoded packet
# out of simplicity (and lazyness), reads from STDIN and write to STDOUT,
# just use your shell piping functionalities!

import sys
import yaml
import struct

# Packet format:
# NSLOTS = 8
# typedef struct rangePacket_s {
#   uint8_t type;
#   uint8_t seqs[NSLOTS];  // Packet sequence number of the timestamps
#   uint32_t timestamps[NSLOTS];  // Relevant time for anchors
#   uint16_t distances[NSLOTS];
# } __attribute__((packed)) rangePacket_t;

for packet in yaml.load_all(sys.stdin, Loader=yaml.Loader):
    if not packet:
        continue

    # Its alost free so we keel the raw data and just add the decoded fields
    packetType = packet["data"][0]

    if packetType == 34:
        decoded = struct.unpack("<BBBBBBBBBLLLLLLLLHHHHHHHH", packet["data"][:57])
        packet["type"] = decoded[0]
        packet["seqs"] = list(decoded[1:9])
        packet["timestamps"] = list(decoded[9:17])
        packet["distances"] = list(decoded[17:25])
        if len(packet["data"]) > 57:
            packet["lpp_data"] = packet["data"][57:]

    print("---")
    print(yaml.dump(packet, Dumper=yaml.Dumper))
