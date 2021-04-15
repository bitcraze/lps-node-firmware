#include "i2c.h"

extern int isL4;

void MX_I2C1_Init(void)
{
  if (isL4) {
    MX_I2C1_Init_l4();
  } else {
    MX_I2C1_Init_f0();    
  }
}
