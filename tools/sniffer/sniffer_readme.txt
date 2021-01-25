website: https://wiki.bitcraze.io/doc:lps:toda3

Sniffing
Sniffing the system is useful when you want to start digging into the radio messages exchanged between the anchor. It is a debug functionality.

If an anchor is configured to act as a sniffer, it can be used to listen to the UWB messages in the system. To set an anchor to sniffer mode, connect via USB and configure it to sniffer mode.

Basic sniffing
Connect the sniffer via USB and check in /dev which port that it uses. Use this as the first argument The second argument is the output format, use yaml example: python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml

TDoA 3 parsing
Pipe the yaml into the TDoA 3 decoder to see TDoA 3 data unpacked

example: python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml | python3 tools/sniffer/tdoa3_decoder.py

This will show all data that the sniffer receives. To limit the output it is possible to filter which anchors to show. Add one or more ids at the end to only show data from only these anchors.

example: python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml | python3 tools/sniffer/tdoa3_decoder.py 2 3

Time of flight and anchor to anchor distance
By piping the TDoA 3 data into the TOF tool, the measured anchor to anchor distances will be displayed. By adding the 'm' argument the output will be in meters instead of UWB clock ticks.

example: python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml | python3 tools/sniffer/tdoa3_decoder.py | python3 tools/sniffer/tdoa3_tof.py m
