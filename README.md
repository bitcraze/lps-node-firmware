# LPS node firmware  [![Build Status](https://api.travis-ci.org/bitcraze/lps-node-firmware.svg)](https://travis-ci.org/bitcraze/lps-node-firmware)

This project contains the source code for the Local positioning System node firmware. 

## Dependencies

You'll need to use either the Crazyflie VM, install some ARM toolchain or the Bitcraze docker builder image. 

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

`docker run --rm -v ${PWD}:/module bitcraze/builder ./tools/build/build`

## Folder description:

> `TODO: Please share!`

# Make targets:
```
all        : Shortcut for build
flash      : Flash throgh jtag
openocd    : Launch OpenOCD
dfu        : Flash throgh DFU 
```
