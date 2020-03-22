---
title: Node low level configuration
page_id: anchor-low-level-config
---



The configuration tool for the Node only supports a sub set of all
settings in the Node, to access the full configuration connect the Node
to a computer with a USB cable. The node will be recognised as a serial
port and you need a terminal program to configure it. The procedure is
OS specific and some suggestions can be found below

Linux
-----

The name of the serial port can be found with *dmesg*

``` {.bash}
$ dmesg
 [ .... ]
[22942.980255] usb 3-2: new full-speed USB device number 13 using xhci_hcd
[22943.109845] usb 3-2: New USB device found, idVendor=0483, idProduct=5740
[22943.109852] usb 3-2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[22943.109855] usb 3-2: Product: Loco Positioning Node
[22943.109858] usb 3-2: Manufacturer: Bitcraze AB
[22943.109861] usb 3-2: SerialNumber: 1C0030000F51485336373420
[22943.110697] cdc_acm 3-2:1.0: ttyACM3: USB ACM device
$
```

The serial port can be opened with your favorite serial console,
picocom:

``` {.bash}
$ picocom /dev/ttyACM3
```

OSX
---

Find the serial port with *ls*

``` {.bash}
$ ls -a /dev/tty.usb*
/dev/tty.usbmodem1415501
```

To open the serial port

``` {.bash}
$ screen /dev/tty.usbmodem1415501
```

The menu and configuration
--------------------------

When you connect to the Node you will be presented with the current
configuration

    ====================
    SYSTEM  : CPU-ID: 380024001757344730323120
    TEST    : Initializing pressure sensor ... [OK]
    TEST    : Pressure sensor self-test ... [OK]
    TEST    : EEPROM self-test ... [OK]
    CONFIG  : EEPROM configuration read and verified
    TEST    : Initialize UWB  ... [OK]
    CONFIG  : Address is 0x2
    CONFIG  : Mode is TDoA Anchor V3
    CONFIG  : Tag mode anchor list (6): 0x01 0x02 0x03 0x04 0x05 0x06 
    CONFIG  : Anchor position enabled: true
    CONFIG  : Anchor position: -7.370000 -1.400000 0.000000
    CONFIG  : SmartPower enabled: False
    CONFIG  : Force TX power: True
    CONFIG  : TX power setting: 1F1F1F1F
    CONFIG  : Bitrate: normal
    CONFIG  : Preamble: normal
    SYSTEM  : Node started ...
    SYSTEM  : Press 'h' for help.

press \'h\' to get information on how to modify the configuration.

Note: the node must be restarted for the configuration to be activated.
