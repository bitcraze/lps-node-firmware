#include <stm32f0xx_hal.h>

static int isL4 = L4_ENABLED;

void SystemClock_Config(void)
{
  if (isL4) {
    SystemClock_Config_l4();
  } else {
    SystemClock_Config_f0();
  }
}
