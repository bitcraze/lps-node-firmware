#!/usr/bin/env python3
#YAML sniffer log TWR decoder

# Wenda

import sys
import yaml
import struct

## Report packet structure format:
# typedef struct {
#   uint8_t pollRx[5];      // R1 timestamp
#   uint8_t answerTx[5];    // T2 timestamp
#   uint8_t finalRx[5];     // R3 timestamp
#   float pressure;         // Pressure measurement at the anchor
#   float temperature;      // Temperature of the pressure sensor
#   float asl;              // Above Sea Level altitude
#   uint8_t pressure_ok;    // Not 0 if the pressure information is correct
# } __attribute__((packed)) lpsTwrTagReportPayload_t;

for packet in yaml.load_all(sys.stdin, Loader=yaml.CLoader):
    if not packet:
        continue

    packetType = packet["data"][0]
    print(packetType)
    # if the rxPacket.payload[0] == LPS_TWR_REPORT (0x04) (line 223 in lpsTwrTag.c on CF) 
    if packetType == 0x02:     # here should be 0x04 !!! (but there is no 0x04)
        print("Before decoder ", packet)
        print("\n")
        decoded = struct.unpack("<BBBBBBBBBBBBBBB",packet["data"][:15])
        packet["pollRx"] = list(decoded[0:5])
        # print(packet["pollRx"])
        packet["answerTx"] = list(decoded[5:10])
        packet["finalRx"] = list(decoded[10:15])
        print(yaml.dump(packet, Dumper=yaml.CDumper))
        print("\n")
    # print("After decoder ", packet)
    # print("\n")
    # print("\n")
    # print(yaml.dump(packet, Dumper=yaml.CDumper))











 