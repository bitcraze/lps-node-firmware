OPENOCD           ?= openocd
OPENOCD_INTERFACE ?= interface/stlink-v2.cfg
REV								?= B
PYTHON2           ?= python2
# CFLAGS						+= -fdiagnostics-color=auto
# CFLAGS += -DUSE_FTDI_UART

ifeq ($(strip $(REV)),A)
$(error Rev.A not supported anymore)
else ifeq ($(strip $(REV)),B)
HAL_ROOT=hal/stm32f0xx
CPU=f0
PROCESSOR=-mthumb -mcpu=cortex-m0 -DHSI48_VALUE="((uint32_t)48000000)" -DSTM32F072xB
OPENOCD_TARGET    ?= target/stm32f0x_stlink.cfg
else
$(error Rev.$(REV) unknown)
endif

INCLUDES=-Iinc -Iinc/$(CPU) -I$(HAL_ROOT)/Inc -IMiddlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc

# FreeRTOS
FREERTOS_OBJS=list queue timers tasks port event_groups
OBJS+=$(foreach mod, $(FREERTOS_OBJS), lib/freertos/src/$(mod).o)
INCLUDES+=-Ilib/freertos/inc

# Platform specific files
OBJS+=src/f0/startup_stm32f072xb.o src/f0/system_stm32f0xx.o src/f0/stm32f0xx_it.o src/f0/stm32f0xx_hal_msp.o
OBJS+=src/f0/gpio.o src/f0/i2c.o src/f0/spi.o src/f0/system.o src/f0/usart.o
OBJS+=src/f0/usbd_conf.o src/eeprom.o
HALS+=i2c_ex

OBJS+=src/main.o
OBJS+=src/usb_device.o src/usbd_cdc_if.o src/usbd_desc.o src/lps25h.o src/led.o
OBJS+=src/cfg.o src/usbcomm.o src/test_support.o src/production_test.o
OBJS+=src/uwb.o src/uwb_twr_anchor.o src/uwb_sniffer.o src/uwb_twr_tag.o

HALS+=gpio rcc cortex i2c pcd dma pcd_ex rcc_ex spi uart
OBJS+=$(foreach mod, $(HALS), $(HAL_ROOT)/Src/stm32$(CPU)xx_hal_$(mod).o)
OBJS+=$(HAL_ROOT)/Src/stm32$(CPU)xx_hal.o

USB_CORES=core ctlreq ioreq
USB_CDC=cdc
OBJS+=$(foreach mod, $(USB_CORES), Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_$(mod).o)
OBJS+=$(foreach mod, $(USB_CDC), Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_$(mod).o)

#libdw1000
INCLUDES+=-Ivendor/libdw1000/inc
OBJS+=vendor/libdw1000/src/libdw1000.o vendor/libdw1000/src/libdw1000Spi.o

OBJS+=src/dwOps.o

CFLAGS+=$(PROCESSOR) $(INCLUDES) -O3 -g3 -Wall -Wno-pointer-sign -std=gnu11
LDFLAGS+=$(PROCESSOR) --specs=nano.specs --specs=nosys.specs -Ttools/make/stm32f072.ld -lm -lc -u _printf_float

# Remove un-used functions and global variables from output file
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS+=-Wl,-Map=bin/$(PROG).map,--cref,--gc-sections


PREFIX=arm-none-eabi-

CC=$(PREFIX)gcc
LD=$(PREFIX)gcc
AS=$(PREFIX)as
OBJCOPY=$(PREFIX)objcopy

all: check_submodules bin/lps-node-firmware.elf bin/lps-node-firmware.dfu

bin/lps-node-firmware.elf: $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)
	arm-none-eabi-size $@

clean:
	rm -f bin/lps-node-firmware.elf bin/lps-node-firmware.dfu bin/.map $(OBJS)

flash:
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) -f $(OPENOCD_TARGET) -c init -c targets -c "reset halt" \
	           -c "flash write_image erase bin/lps-node-firmware.elf" -c "verify_image bin/lps-node-firmware.elf" -c "reset run" -c shutdown

openocd:
	$(OPENOCD) -d2 -f $(OPENOCD_INTERFACE) -f $(OPENOCD_TARGET) -c init -c targets

dfu:
	dfu-util -d 0483:df11 -a 0 -D bin/lps-node-firmware.dfu -R

# Generic rules
%.bin: %.elf
	$(OBJCOPY) $^ -O binary $@

%.dfu: %.bin
	$(PYTHON2) tools/make/dfu-convert.py -b 0x8000000:$^ $@

check_submodules:
	$(PYTHON2) tools/make/check-for-submodules.py
