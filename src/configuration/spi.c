#include "spi.h"

extern int isL4;

SPI_HandleTypeDef hspi1;

void MX_SPI1_Init(void)
{
  if (isL4) {
    MX_SPI1_Init_l4();
  } else {
    MX_SPI1_Init_f0();
  }
}

void MX_SPI1_Init_Fast(void)
{
  if (isL4) {
    MX_SPI1_Init_Fast_l4();
  } else {
    MX_SPI1_Init_Fast_f0();
  }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{
  if (isL4) {
    HAL_SPI_MspInit_l4();
  } else {
    HAL_SPI_MspInit_f0();
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{
  if (isL4) {
    MX_SPI1_MspDeInit_l4();
  } else {
    MX_SPI1_MspDeInit_f0();
  }
}
