#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

static int isL4 = L4_ENABLED;

uint8_t * USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
 if (isL4) {
   USBD_FS_DeviceDescriptor_l4();
 } else {
   USBD_FS_DeviceDescriptor_f0();
 }
}


uint8_t * USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  if (isL4) {
   USBD_FS_LangIDStrDescriptor_l4();
 } else {
   USBD_FS_LangIDStrDescriptor_f0();
 }
}


uint8_t * USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  if (isL4) {
   USBD_FS_ProductStrDescriptor_l4();
 } else {
   USBD_FS_ProductStrDescriptor_f0();
 }
}


uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  if (isL4) {
   USBD_FS_ManufacturerStrDescriptor_l4();
 } else {
   USBD_FS_ManufacturerStrDescriptor_f0();
 }
}


uint8_t * USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  if (isL4) {
   USBD_FS_SerialStrDescriptor_l4();
 } else {
   USBD_FS_SerialStrDescriptor_f0();
 }
}


uint8_t * USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  if (isL4) {
   USBD_FS_ConfigStrDescriptor_l4();
 } else {
   USBD_FS_ConfigStrDescriptor_f0();
 }
}


uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  if (isL4) {
   USBD_FS_InterfaceStrDescriptor_l4();
 } else {
   USBD_FS_InterfaceStrDescriptor_f0();
 }
}

#if (USBD_LPM_ENABLED == 1)
/**
  * @brief  Return the BOS descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_USR_BOSDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  if (isL4) {
   USBD_FS_USR_BOSDescriptor_l4();
 } else {
   USBD_FS_USR_BOSDescriptor_f0();
 }
}
#endif /* (USBD_LPM_ENABLED == 1) */
