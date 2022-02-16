# LPS node firmware  [![CI](https://github.com/bitcraze/lps-node-firmware/workflows/CI/badge.svg)](https://github.com/bitcraze/lps-node-firmware/actions?query=workflow%3ACI)

This project contains the source code for the Local positioning System node firmware. 

See the [Bitcraze documentation](https://www.bitcraze.io/documentation/repository/lps-node-firmware/master/) for more information.

## Dependencies

You'll need to use either the Crazyflie VM, install some ARM toolchain or the Bitcraze docker builder image. If you install a toolchain, the [arm embedded gcc](https://launchpad.net/gcc-arm-embedded) toolchain is recomended.

Frameworks for unit testing are pulled in as git submodules. To get them when cloning

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

### Debian/Ubuntu linux

Download, extract and put in your path the compiler from https://launchpad.net/gcc-arm-embedded. For example:
```
mkdir -p ~/opt/
cd ~/opt
wget https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q3-update/+download/gcc-arm-none-eabi-5_4-2016q3-20160926-linux.tar.bz2
tar xvf gcc-arm-none-eabi-5_4-2016q3-20160926-linux.tar.bz2
mv gcc-arm-none-eabi-5_4-2016q3 gcc-arm-none-eabi
rm gcc-arm-none-eabi-5_4-2016q3-20160926-linux.tar.bz2
echo "export PATH=\$PATH:\$HOME/opt/gcc-arm-none-eabi/bin" >> ~/.bashrc
```

On 64Bit Linux you also need to install some 32Bit libs:
```
sudo apt-get install libncurses5:i386
```

If all works you will be able to execute ```arm-none-eabi-gcc```:
```
$ arm-none-eabi-gcc --version
arm-none-eabi-gcc (GNU Tools for ARM Embedded Processors) 5.4.1 20160919 (release) [ARM/embedded-5-branch revision 240496]
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```
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

## Contribute
Go to the [contribute page](https://www.bitcraze.io/contribute/) on our website to learn more.

### Test code for contribution
Run the automated build locally to test your code

	./tools/build/build
