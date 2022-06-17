#!/usr/bin/env python3
# YAML sniffer log TDOA3 decoder, generates a stream of YAML documents
# containing the decoded packet
# out of simplicity (and lazyness), reads from STDIN and write to STDOUT,
# just use your shell piping functionalities!

import sys
import yaml
import struct
import time

id_filter = None
if len(sys.argv) > 1:
    for arg in sys.argv[1:]:
        if arg.isdigit():
            if not id_filter:
                id_filter = []
            id_filter.append(int(arg))
        else:
            print("usage: {} [id filter]".format(sys.argv[0]))
            print("  id filter: a list of anchor ids. Default is to let all pass")
            sys.exit(1)


# Packet format:
# typedef struct {
#     uint8_t type;
# uint8_t seq;
# uint32_t txTimeStamp;
# uint8_t remoteCount;
# } __attribute__((packed)) rangePacketHeader3_t;
#
# typedef struct {
#     uint8_t id;
# uint8_t seq;
# uint32_t rxTimeStamp;
# uint16_t distance;
# } __attribute__((packed)) remoteAnchorDataFull_t;
#
# typedef struct {
#     uint8_t id;
# uint8_t seq;
# uint32_t rxTimeStamp;
# } __attribute__((packed)) remoteAnchorDataShort_t;

for packet in yaml.load_all(sys.stdin, Loader=yaml.Loader):
    if not packet:
        continue

    id = packet['from']
    if not id_filter or id in id_filter:

        # Its alost free so we keel the raw data and just add the decoded fields
        packetType = packet["data"][0]
        packet["type"] = packetType

        if packetType == 0x30:
            decoded = struct.unpack("<BBLB", packet["data"][:7])
            packet["seq"] = decoded[1]
            packet["txTimeStamp"] = decoded[2]

            remote_count = decoded[3]
            packet["remoteCount"] = remote_count

            packet['remoteAnchorData'] = []
            anchor_data_index = 7

            for i in range(remote_count):
                decoded_anchor_data = struct.unpack(
                    '<BBL',
                    packet["data"][anchor_data_index:(anchor_data_index + 6)])
                seq = decoded_anchor_data[1] & 0x7f
                anchor_data = {
                    'id': decoded_anchor_data[0],
                    'seq': seq,
                    'rxTimeStamp': decoded_anchor_data[2],
                }
                anchor_data_index += 6

                has_distance = ((decoded_anchor_data[1] & 0x80) != 0)
                if (has_distance):
                    decoded_distance = struct.unpack(
                        '<H',
                        packet["data"][anchor_data_index:(anchor_data_index + 2)])
                    anchor_data['distance'] = decoded_distance[0]
                    anchor_data_index += 2

                packet['remoteAnchorData'].append(anchor_data)

            if len(packet["data"]) > anchor_data_index:
                if packet["data"][anchor_data_index] == 0xf0 and \
                        packet["data"][anchor_data_index + 1] == 0x01:
                    packet["lpp_data"] = {}
                    packet["lpp_data"]['header'] = packet["data"][
                        anchor_data_index]
                    anchor_data_index += 1

                    packet["lpp_data"]['type'] = packet["data"][anchor_data_index]
                    anchor_data_index += 1

                    decoded = struct.unpack(
                        "<fff",
                        packet["data"][anchor_data_index:anchor_data_index + 3 * 6]
                    )
                    packet["lpp_data"]['position'] = {}
                    packet["lpp_data"]['position']['x'] = decoded[0]
                    packet["lpp_data"]['position']['y'] = decoded[1]
                    packet["lpp_data"]['position']['z'] = decoded[2]
                    anchor_data_index += 3 * 6
                else:
                    packet["lpp_data"] = packet["data"][anchor_data_index:]

        print("---")
        print(yaml.dump(packet, Dumper=yaml.Dumper))
