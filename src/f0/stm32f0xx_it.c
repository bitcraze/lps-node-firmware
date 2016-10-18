/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_FS;

/******************************************************************************/
/*            Cortex-M0 Processor Interruption and Exception Handlers         */
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void vApplicationTickHook(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles USB global interrupt (combined with EXTI line 18 interrupt).
*/
void USB_IRQHandler(void)
{
  /* USER CODE BEGIN USB_IRQn 0 */

  /* USER CODE END USB_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
  /* USER CODE BEGIN USB_IRQn 1 */

  /* USER CODE END USB_IRQn 1 */
}

void EXTI0_1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/* USER CODE BEGIN 1 */

void HardFault_Handler(void)
{
  //http://www.st.com/mcu/forums-cat-6778-23.html
  //****************************************************
  //To test this application, you can use this snippet anywhere:
  // //Let's crash the MCU!
  // asm (" MOVS r0, #1 \n"
  // " LDM r0,{r1-r2} \n"
  // " BX LR; \n");
  /*__asm__( "TST LR, #4 \n"
  "ITE EQ \n"
  "MRSEQ R0, MSP \n"
  "MRSNE R0, PSP \n"
  "B printHardFault");*/

  __asm__(  ".syntax unified\n"
                  "MOVS   R0, #4  \n"
                  "MOV    R1, LR  \n"
                  "TST    R0, R1  \n"
                  "BEQ    _MSP    \n"
                  "MRS    R0, PSP \n"
                  "B      printHardFault      \n"
          "_MSP:  \n"
                  "MRS    R0, MSP \n"
                  "B      printHardFault      \n"
          ".syntax divided\n") ;
}

void __attribute__((optimize("O0"))) printHardFault(unsigned int* hardfault_args)
{
  volatile unsigned int stacked_r0 ;
  volatile unsigned int stacked_r1 ;
  volatile unsigned int stacked_r2 ;
  volatile unsigned int stacked_r3 ;
  volatile unsigned int stacked_r12 ;
  volatile unsigned int stacked_lr ;
  volatile unsigned int stacked_pc ;
  volatile unsigned int stacked_psr ;
  volatile unsigned int _CFSR ;
  volatile unsigned int _HFSR ;
  volatile unsigned int _DFSR ;
  volatile unsigned int _AFSR ;
  volatile unsigned int _BFAR ;
  volatile unsigned int _MMAR ;

  stacked_r0 = ((unsigned int)hardfault_args[0]);
  stacked_r1 = ((unsigned int)hardfault_args[1]);
  stacked_r2 = ((unsigned int)hardfault_args[2]);
  stacked_r3 = ((unsigned int)hardfault_args[3]);
  stacked_r12 = ((unsigned int)hardfault_args[4]);
  stacked_lr = ((unsigned int)hardfault_args[5]);
  stacked_pc = ((unsigned int)hardfault_args[6]);
  stacked_psr = (*((volatile unsigned int *)(&hardfault_args[7])));

  // Configurable Fault Status Register
  // Consists of MMSR, BFSR and UFSR
  _CFSR = (*((volatile unsigned long *)(0xE000ED28))) ;

  // Hard Fault Status Register
  _HFSR = (*((volatile unsigned long *)(0xE000ED2C))) ;

  // Debug Fault Status Register
  _DFSR = (*((volatile unsigned long *)(0xE000ED30))) ;

  // Auxiliary Fault Status Register
  _AFSR = (*((volatile unsigned long *)(0xE000ED3C))) ;

  // Read the Fault Address Registers. These may not contain valid values.
  // Check BFARVALID/MMARVALID to see if they are valid values
  // MemManage Fault Address Register
  _MMAR = (*((volatile unsigned long *)(0xE000ED34))) ;
  // Bus Fault Address Register
  _BFAR = (*((volatile unsigned long *)(0xE000ED38))) ;

  while (1)
  {
    printf("\r\n[Hard fault handler]\r\n");
    printf("R0   = 0x%X\r\n", stacked_r0);
    printf("R1   = 0x%X\r\n", stacked_r1);
    printf("R2   = 0x%X\r\n", stacked_r2);
    printf("R3   = 0x%X\r\n", stacked_r3);
    printf("R12  = 0x%X\r\n", stacked_r12);
    printf("LR   = 0x%X\r\n", stacked_lr);
    printf("PC   = 0x%X\r\n", stacked_pc);
    printf("PSR  = 0x%X\r\n", stacked_psr);
    printf("MMAR = 0x%X\r\n", _MMAR);
    printf("BFAR = 0x%X\r\n", _BFAR);
    printf("CFSR = 0x%X\r\n", _CFSR);
    printf("HFSR = 0x%X\r\n", _HFSR);
    printf("DFSR = 0x%X\r\n", _DFSR);
    printf("AFSR = 0x%X\r\n", _AFSR);
  }
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
