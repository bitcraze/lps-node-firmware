# LPS node firmware  [![Build Status](https://api.travis-ci.org/bitcraze/lps-node-firmware.svg)](https://travis-ci.org/bitcraze/lps-node-firmware)

This project contains the source code for the Local positioning System node firmware. 

## Dependencies

You'll need to use either the Crazyflie VM, install some ARM toolchain or the Bitcraze docker builder image.

Frameworks for unit testing is pulled in as git submodules. To get them when cloning

```bash
git clone --recursive https://github.com/bitcraze/lps-node-firmware.git
```
        
or if you already have a cloned repo and want the submodules
 
```bash
git submodule init        
git submodule update        
```

### OS X
```bash
brew tap PX4/homebrew-px4
brew install gcc-arm-none-eabi
```

### Debian/Ubuntu

> `TODO: Please share!`

### Arch Linux

```bash
sudo pacman -S community/arm-none-eabi-gcc community/arm-none-eabi-gdb community/arm-none-eabi-newlib
```

### Windows

> `TODO: Please share!`

## Compiling

`make`

or 

`docker run --rm -v ${PWD}:/module bitcraze/builder ./tools/build/compile`

or 

`tools/do compile`

## Folder description:

> `TODO: Please share!`

# Make targets:
```
all        : Shortcut for build
flash      : Flash throgh jtag
openocd    : Launch OpenOCD
dfu        : Flash throgh DFU 
```

## Unit testing

We use [Unity](https://github.com/ThrowTheSwitch/unity) and [cmock](https://github.com/ThrowTheSwitch/CMock) for unit testing.

To run all tests 

`./tools/do test`

