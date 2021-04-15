#include <stdbool.h>

#include "stm32f0xx.h"
#include "stm32f0xx_hal.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_cdc.h"

static int isL4 = L4_ENABLED;

PCD_HandleTypeDef hpcd_USB_FS;
void Error_Handler(void);


void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
{
 if (isL4) {
   HAL_PCD_MspInit_l4(pcdHandle);
 } else {
   HAL_PCD_MspInit_f0(pcdHandle);
 }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
{
  if (isL4) {
    HAL_PCD_MspDeInit_l4(pcdHandle);
  } else {
    HAL_PCD_MspDeInit_f0(pcdHandle);
  }
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
  if (isL4) {
    HAL_PCD_SetupStageCallback_l4(hpcd);
 } else {
    HAL_PCD_SetupStageCallback_f0(hpcd);
 }
}

bool USBD_IsSerialConnected()
{
  if (isL4) {
    return USBD_IsSerialConnected_l4();
 } else {
    return USBD_IsSerialConnected_f0();
 }
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  if (isL4) {
    HAL_PCD_DataOutStageCallback_l4(hpcd, epnum);
 } else {
    HAL_PCD_DataOutStageCallback_f0(hpcd, epnum);
 }
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  if (isL4) {
    HAL_PCD_DataInStageCallback_l4(hpcd, epnum);
  } else {
    HAL_PCD_DataInStageCallback_f0(hpcd, epnum);
  }
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
  if (isL4) {
    HAL_PCD_SOFCallback_l4(hpcd);
 } else {
    HAL_PCD_SOFCallback_f0(hpcd);
 }
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
  if (isL4) {
    HAL_PCD_ResetCallback_l4(hpcd);
 } else {
    HAL_PCD_ResetCallback_f0(hpcd);
 }
}


void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
  if (isL4) {
    HAL_PCD_SuspendCallback_l4(hpcd);
 } else {
    HAL_PCD_SuspendCallback_f0(hpcd);
 }
}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
  if (isL4) {
    HAL_PCD_ResumeCallback_l4(hpcd);
 } else {
    HAL_PCD_ResumeCallback_f0(hpcd);
 }
}

void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  if (isL4) {
    HAL_PCD_ISOOUTIncompleteCallback_l4(hpcd, epnum);
 } else {
    HAL_PCD_ISOOUTIncompleteCallback_f0(hpcd, epnum);
 }
}

void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  if (isL4) {
    HAL_PCD_ISOINIncompleteCallback_l4(hpcd, epnum);
 } else {
    HAL_PCD_ISOINIncompleteCallback_f0(hpcd, epnum);
 }
}

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
  if (isL4) {
    HAL_PCD_ConnectCallback_l4(hpcd);
 } else {
   HAL_PCD_ConnectCallback_f0(hpcd);
 }
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
  if (isL4) {
    HAL_PCD_DisconnectCallback_l4(hpcd);
 } else {
    HAL_PCD_DisconnectCallback_f0(hpcd);
 }
}

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
  if (isL4) {
    return USBD_LL_Init_l4(pdev);
 } else {
    return USBD_LL_Init_f0(pdev);
 }
}


USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
  if (isL4) {
    return USBD_LL_DeInit_l4(pdev);
 } else {
    return USBD_LL_DeInit_f0(pdev);
 }
}

USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
  if (isL4) {
    return USBD_LL_Start_l4(pdev);
 } else {
    return USBD_LL_Start_f0(pdev);
 }
}


USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
  if (isL4) {
    return USBD_LL_Stop_l4(pdev);
 } else {
    return USBD_LL_Stop_f0(pdev);
 }
}

USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
  if (isL4) {
    return USBD_LL_OpenEP_l4(pdev, ep_addr, ep_type, ep_mps);
 } else {
   return USBD_LL_OpenEP_f0(pdev, ep_addr, ep_type, ep_mps);
 }
}

USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  if (isL4) {
    return USBD_LL_CloseEP_l4(pdev, ep_addr);
 } else {
    return USBD_LL_CloseEP_f0(pdev, ep_addr);
 }
}

USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  if (isL4) {
    return USBD_LL_FlushEP_l4(pdev, ep_addr);
 } else {
   return USBD_LL_FlushEP_f0(pdev, ep_addr);
 }
}

USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  if (isL4) {
    return USBD_LL_StallEP_l4(pdev, ep_addr);
 } else {
   return USBD_LL_StallEP_f0(pdev, ep_addr);
 }
}

USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  if (isL4) {
    return USBD_LL_ClearStallEP_l4(pdev, ep_addr);
 } else {
    return USBD_LL_ClearStallEP_f0(pdev, ep_addr);
 }
}

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  if (isL4) {
    return USBD_LL_IsStallEP_l4(pdev, ep_addr);
 } else {
    return USBD_LL_IsStallEP_f0(pdev, ep_addr);
 }
}

USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
  if (isL4) {
    return USBD_LL_SetUSBAddress_l4(pdev, dev_addr);
 } else {
    return USBD_LL_SetUSBAddress_f0(pdev, dev_addr);   
 }
}

USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
  if (isL4) {
    return USBD_LL_Transmit_l4(pdev, ep_addr, pbuf, size);
 } else {
    return USBD_LL_Transmit_f0(pdev, ep_addr, pbuf, size);
 }
}

USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
  if (isL4) {
    return USBD_LL_PrepareReceive_l4(pdev, ep_addr, pbuf, size);
 } else {
    return USBD_LL_PrepareReceive_f0(pdev, ep_addr, pbuf, size);
 }
}

uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  if (isL4) {
    return USBD_LL_GetRxDataSize_l4(pdev, ep_addr);
 } else {    
    return USBD_LL_GetRxDataSize_f0(pdev, ep_addr);
 }
}

void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg)
{
  if (isL4) {
    HAL_PCDEx_LPM_Callback_l4(hpcd, msg);
 } else {
    HAL_PCDEx_LPM_Callback_f0(hpcd, msg);
 }
}

void USBD_LL_Delay(uint32_t Delay)
{
  if (isL4) {
    USBD_LL_Delay_l4(Delay);
 } else {
   USBD_LL_Delay_f0(Delay);
 }
}

void *USBD_static_malloc(uint32_t size)
{
  if (isL4) {
    return USBD_static_malloc_l4(size);
 } else {
    return USBD_static_malloc_f0(size);
 }
}

void USBD_static_free(void *p)
{
  if (isL4) {
    USBD_static_free_l4(p);
 } else {
    USBD_static_free_f0(p);
 }
}
