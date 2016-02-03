#ifndef __PORT_H__
#define __PORT_H__

#include "compiler.h"
#include "deca_device_api.h"

#ifdef STM32F103xB
#include <stm32f1xx_hal.h>
#else
#include <stm32f0xx_hal.h>
#endif

int writetospi(uint16 headerLength, const uint8 *headerBuffer,
                             uint32 bodylength, const uint8 *bodyBuffer);

int readfromspi(uint16 headerLength,  const uint8 *headerBuffer,
                              uint32 readlength, uint8 *readBuffer);

decaIrqStatus_t decamutexon(void);

void decamutexoff(decaIrqStatus_t s);

#define SPI_BaudRatePrescaler_4 0
#define SPI_BaudRatePrescaler_16 1

//void SPI_ConfigFastRate(uint16_t scalingfactor);
#define SPI_ConfigFastRate(X)

#define portGetTickCount() 			HAL_GetTick()

void reset_DW1000(void);

void setup_DW1000RSTnIRQ(int enable);

#define port_SPIx_clear_chip_select()
#define port_SPIx_set_chip_select()


#endif
