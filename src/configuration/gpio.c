#include "gpio.h"

extern int isL4;

void MX_GPIO_Init(void)
{
  if (isL4) {
    MX_GPIO_Init_l4();
  } else {
    MX_GPIO_Init_f0();
  }
}

