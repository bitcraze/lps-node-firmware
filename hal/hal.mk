HAL_MODULES = gpio rcc cortex i2c i2c_ex pcd dma pcd_ex rcc_ex spi uart pwr pwr_ex
HAL_OBJS_F0 = $(foreach mod,$(HAL_MODULES),hal/stm32f0xx/Src/stm32f0xx_hal_$(mod).o)
HAL_OBJS_F0 += hal/stm32f0xx/Src/stm32f0xx_hal.o

HAL_OBJS_L4 = $(foreach mod,$(HAL_MODULES),hal/stm32l4xx/Src/stm32l4xx_hal_$(mod).o)
HAL_OBJS_L4 += hal/stm32l4xx/Src/stm32l4xx_hal.o hal/stm32l4xx/Src/stm32l4xx_ll_usb.o

HAL_OBJS = $(HAL_OBJS_F0) $(HAL_OBJS_L4)

#
# Create a rule for each HAL obj file to build.
# $(1): path to object file
# $(2): MCU (f0/l4)
#
define create-hal-obj-rule
$(1): $(1:.o=.c)
	$(CC) -c $(CFLAGS) -I inc/$(2) -Ihal/stm32$(2)xx/Inc $$< -o $$@
endef

$(foreach obj,$(HAL_OBJS_F0),$(eval $(call create-hal-obj-rule,$(obj),f0)))
$(foreach obj,$(HAL_OBJS_L4),$(eval $(call create-hal-obj-rule,$(obj),l4)))

hal/libstm32f0xx_hal.a: $(HAL_OBJS_F0)
	$(AR) rcs $@ $^
	$(GDB) -q -ex 'py prefix="f0"' -x tools/gen_function_sym_pairs.py
	$(OBJCOPY) --redefine-syms=hal/f0.symbols $@ $@

hal/libstm32l4xx_hal.a: $(HAL_OBJS_L4) | $(HAL_OBJS_F0)
	$(AR) rcs $@ $^
	$(GDB) -q -ex 'py prefix="l4"' -x tools/gen_function_sym_pairs.py
	$(OBJCOPY) --redefine-syms=hal/l4.symbols $@ $@

hal/hal_shim.c: $(HAL_OBJS_L4) $(HAL_OBJS_F0) tools/gen_shim_hal.py
	$(GDB) -q -x tools/gen_shim_hal.py

hal/hal_shim.o: hal/hal_shim.c
	$(CC) -c -Ihal/stm32f0xx/Inc  -Iinc/f0 $(CFLAGS) -Wno-implicit-function-declaration  $< -o $@

OBJS += hal/hal_shim.o
LIBS += hal/libstm32f0xx_hal.a hal/libstm32l4xx_hal.a
LDFLAGS += -lstm32f0xx_hal -lstm32l4xx_hal
