#include <stm32f0xx_hal.h>

uint32_t SystemCoreClock;
extern uint32_t SystemCoreClock_l4;
extern uint32_t SystemCoreClock_f0;

const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};

#define isL422_MCU_ID_CODE  (((*((volatile unsigned long *) 0xE0042000)) & 0x00000FFFU) == 0x464)


/* L4 */
const uint32_t MSIRangeTable[12] = {100000U,   200000U,   400000U,   800000U,  1000000U,  2000000U, \
                                    4000000U, 8000000U, 16000000U, 24000000U, 32000000U, 48000000U};

void SystemClock_Config(void)
{
  if (isL422_MCU_ID_CODE) {
    SystemCoreClock = SystemCoreClock_l4;
    SystemClock_Config_l4();
  } else {
    SystemCoreClock = SystemCoreClock_f0;
    SystemClock_Config_f0();
  }
}

void SystemInit(void)
{
  if (isL422_MCU_ID_CODE) {
    SystemInit_l4();
  } else {
    SystemInit_f0();
  }
}
