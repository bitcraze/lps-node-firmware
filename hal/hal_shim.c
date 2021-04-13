#include <stm32f0xx_hal.h>
static int l4 = 0;
HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef * a,  uint32_t b) {
	if (l4)
		return l4_HAL_I2CEx_ConfigAnalogFilter(a, b);
	else
		return f0_HAL_I2CEx_ConfigAnalogFilter(a, b);
}

HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter(I2C_HandleTypeDef * a,  uint32_t b) {
	if (l4)
		return l4_HAL_I2CEx_ConfigDigitalFilter(a, b);
	else
		return f0_HAL_I2CEx_ConfigDigitalFilter(a, b);
}

void HAL_I2CEx_DisableFastModePlus(uint32_t a) {
	if (l4)
		l4_HAL_I2CEx_DisableFastModePlus(a);
	else
		f0_HAL_I2CEx_DisableFastModePlus(a);
}

HAL_StatusTypeDef HAL_I2CEx_DisableWakeUp(I2C_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_I2CEx_DisableWakeUp(a);
	else
		return f0_HAL_I2CEx_DisableWakeUp(a);
}

void HAL_I2CEx_EnableFastModePlus(uint32_t a) {
	if (l4)
		l4_HAL_I2CEx_EnableFastModePlus(a);
	else
		f0_HAL_I2CEx_EnableFastModePlus(a);
}

HAL_StatusTypeDef HAL_I2CEx_EnableWakeUp(I2C_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_I2CEx_EnableWakeUp(a);
	else
		return f0_HAL_I2CEx_EnableWakeUp(a);
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_AbortCpltCallback(a);
	else
		f0_HAL_I2C_AbortCpltCallback(a);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef * a,  uint8_t b,  uint16_t c) {
	if (l4)
		l4_HAL_I2C_AddrCallback(a, b, c);
	else
		f0_HAL_I2C_AddrCallback(a, b, c);
}

HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_I2C_DeInit(a);
	else
		return f0_HAL_I2C_DeInit(a);
}

HAL_StatusTypeDef HAL_I2C_DisableListen_IT(I2C_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_I2C_DisableListen_IT(a);
	else
		return f0_HAL_I2C_DisableListen_IT(a);
}

void HAL_I2C_ER_IRQHandler(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_ER_IRQHandler(a);
	else
		f0_HAL_I2C_ER_IRQHandler(a);
}

void HAL_I2C_EV_IRQHandler(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_EV_IRQHandler(a);
	else
		f0_HAL_I2C_EV_IRQHandler(a);
}

HAL_StatusTypeDef HAL_I2C_EnableListen_IT(I2C_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_I2C_EnableListen_IT(a);
	else
		return f0_HAL_I2C_EnableListen_IT(a);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_ErrorCallback(a);
	else
		f0_HAL_I2C_ErrorCallback(a);
}

uint32_t HAL_I2C_GetError(I2C_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_I2C_GetError(a);
	else
		return f0_HAL_I2C_GetError(a);
}

HAL_I2C_ModeTypeDef HAL_I2C_GetMode(I2C_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_I2C_GetMode(a);
	else
		return f0_HAL_I2C_GetMode(a);
}

HAL_I2C_StateTypeDef HAL_I2C_GetState(I2C_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_I2C_GetState(a);
	else
		return f0_HAL_I2C_GetState(a);
}

HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_I2C_Init(a);
	else
		return f0_HAL_I2C_Init(a);
}

HAL_StatusTypeDef HAL_I2C_IsDeviceReady(I2C_HandleTypeDef * a,  uint16_t b,  uint32_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_I2C_IsDeviceReady(a, b, c, d);
	else
		return f0_HAL_I2C_IsDeviceReady(a, b, c, d);
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_ListenCpltCallback(a);
	else
		f0_HAL_I2C_ListenCpltCallback(a);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_MasterRxCpltCallback(a);
	else
		f0_HAL_I2C_MasterRxCpltCallback(a);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_MasterTxCpltCallback(a);
	else
		f0_HAL_I2C_MasterTxCpltCallback(a);
}

HAL_StatusTypeDef HAL_I2C_Master_Abort_IT(I2C_HandleTypeDef * a,  uint16_t b) {
	if (l4)
		return l4_HAL_I2C_Master_Abort_IT(a, b);
	else
		return f0_HAL_I2C_Master_Abort_IT(a, b);
}

HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d,  uint32_t e) {
	if (l4)
		return l4_HAL_I2C_Master_Receive(a, b, c, d, e);
	else
		return f0_HAL_I2C_Master_Receive(a, b, c, d, e);
}

HAL_StatusTypeDef HAL_I2C_Master_Receive_DMA(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d) {
	if (l4)
		return l4_HAL_I2C_Master_Receive_DMA(a, b, c, d);
	else
		return f0_HAL_I2C_Master_Receive_DMA(a, b, c, d);
}

HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d) {
	if (l4)
		return l4_HAL_I2C_Master_Receive_IT(a, b, c, d);
	else
		return f0_HAL_I2C_Master_Receive_IT(a, b, c, d);
}

HAL_StatusTypeDef HAL_I2C_Master_Seq_Receive_DMA(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d,  uint32_t e) {
	if (l4)
		return l4_HAL_I2C_Master_Seq_Receive_DMA(a, b, c, d, e);
	else
		return f0_HAL_I2C_Master_Seq_Receive_DMA(a, b, c, d, e);
}

HAL_StatusTypeDef HAL_I2C_Master_Seq_Receive_IT(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d,  uint32_t e) {
	if (l4)
		return l4_HAL_I2C_Master_Seq_Receive_IT(a, b, c, d, e);
	else
		return f0_HAL_I2C_Master_Seq_Receive_IT(a, b, c, d, e);
}

HAL_StatusTypeDef HAL_I2C_Master_Seq_Transmit_DMA(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d,  uint32_t e) {
	if (l4)
		return l4_HAL_I2C_Master_Seq_Transmit_DMA(a, b, c, d, e);
	else
		return f0_HAL_I2C_Master_Seq_Transmit_DMA(a, b, c, d, e);
}

HAL_StatusTypeDef HAL_I2C_Master_Seq_Transmit_IT(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d,  uint32_t e) {
	if (l4)
		return l4_HAL_I2C_Master_Seq_Transmit_IT(a, b, c, d, e);
	else
		return f0_HAL_I2C_Master_Seq_Transmit_IT(a, b, c, d, e);
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d,  uint32_t e) {
	if (l4)
		return l4_HAL_I2C_Master_Transmit(a, b, c, d, e);
	else
		return f0_HAL_I2C_Master_Transmit(a, b, c, d, e);
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit_DMA(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d) {
	if (l4)
		return l4_HAL_I2C_Master_Transmit_DMA(a, b, c, d);
	else
		return f0_HAL_I2C_Master_Transmit_DMA(a, b, c, d);
}

HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef * a,  uint16_t b,  uint8_t * c,  uint16_t d) {
	if (l4)
		return l4_HAL_I2C_Master_Transmit_IT(a, b, c, d);
	else
		return f0_HAL_I2C_Master_Transmit_IT(a, b, c, d);
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_MemRxCpltCallback(a);
	else
		f0_HAL_I2C_MemRxCpltCallback(a);
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_MemTxCpltCallback(a);
	else
		f0_HAL_I2C_MemTxCpltCallback(a);
}

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef * a,  uint16_t b,  uint16_t c,  uint16_t d,  uint8_t * e,  uint16_t f,  uint32_t g) {
	if (l4)
		return l4_HAL_I2C_Mem_Read(a, b, c, d, e, f, g);
	else
		return f0_HAL_I2C_Mem_Read(a, b, c, d, e, f, g);
}

HAL_StatusTypeDef HAL_I2C_Mem_Read_DMA(I2C_HandleTypeDef * a,  uint16_t b,  uint16_t c,  uint16_t d,  uint8_t * e,  uint16_t f) {
	if (l4)
		return l4_HAL_I2C_Mem_Read_DMA(a, b, c, d, e, f);
	else
		return f0_HAL_I2C_Mem_Read_DMA(a, b, c, d, e, f);
}

HAL_StatusTypeDef HAL_I2C_Mem_Read_IT(I2C_HandleTypeDef * a,  uint16_t b,  uint16_t c,  uint16_t d,  uint8_t * e,  uint16_t f) {
	if (l4)
		return l4_HAL_I2C_Mem_Read_IT(a, b, c, d, e, f);
	else
		return f0_HAL_I2C_Mem_Read_IT(a, b, c, d, e, f);
}

HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef * a,  uint16_t b,  uint16_t c,  uint16_t d,  uint8_t * e,  uint16_t f,  uint32_t g) {
	if (l4)
		return l4_HAL_I2C_Mem_Write(a, b, c, d, e, f, g);
	else
		return f0_HAL_I2C_Mem_Write(a, b, c, d, e, f, g);
}

HAL_StatusTypeDef HAL_I2C_Mem_Write_DMA(I2C_HandleTypeDef * a,  uint16_t b,  uint16_t c,  uint16_t d,  uint8_t * e,  uint16_t f) {
	if (l4)
		return l4_HAL_I2C_Mem_Write_DMA(a, b, c, d, e, f);
	else
		return f0_HAL_I2C_Mem_Write_DMA(a, b, c, d, e, f);
}

HAL_StatusTypeDef HAL_I2C_Mem_Write_IT(I2C_HandleTypeDef * a,  uint16_t b,  uint16_t c,  uint16_t d,  uint8_t * e,  uint16_t f) {
	if (l4)
		return l4_HAL_I2C_Mem_Write_IT(a, b, c, d, e, f);
	else
		return f0_HAL_I2C_Mem_Write_IT(a, b, c, d, e, f);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_SlaveRxCpltCallback(a);
	else
		f0_HAL_I2C_SlaveRxCpltCallback(a);
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef * a) {
	if (l4)
		l4_HAL_I2C_SlaveTxCpltCallback(a);
	else
		f0_HAL_I2C_SlaveTxCpltCallback(a);
}

HAL_StatusTypeDef HAL_I2C_Slave_Receive(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_I2C_Slave_Receive(a, b, c, d);
	else
		return f0_HAL_I2C_Slave_Receive(a, b, c, d);
}

HAL_StatusTypeDef HAL_I2C_Slave_Receive_DMA(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_I2C_Slave_Receive_DMA(a, b, c);
	else
		return f0_HAL_I2C_Slave_Receive_DMA(a, b, c);
}

HAL_StatusTypeDef HAL_I2C_Slave_Receive_IT(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_I2C_Slave_Receive_IT(a, b, c);
	else
		return f0_HAL_I2C_Slave_Receive_IT(a, b, c);
}

HAL_StatusTypeDef HAL_I2C_Slave_Seq_Receive_DMA(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_I2C_Slave_Seq_Receive_DMA(a, b, c, d);
	else
		return f0_HAL_I2C_Slave_Seq_Receive_DMA(a, b, c, d);
}

HAL_StatusTypeDef HAL_I2C_Slave_Seq_Receive_IT(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_I2C_Slave_Seq_Receive_IT(a, b, c, d);
	else
		return f0_HAL_I2C_Slave_Seq_Receive_IT(a, b, c, d);
}

HAL_StatusTypeDef HAL_I2C_Slave_Seq_Transmit_DMA(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_I2C_Slave_Seq_Transmit_DMA(a, b, c, d);
	else
		return f0_HAL_I2C_Slave_Seq_Transmit_DMA(a, b, c, d);
}

HAL_StatusTypeDef HAL_I2C_Slave_Seq_Transmit_IT(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_I2C_Slave_Seq_Transmit_IT(a, b, c, d);
	else
		return f0_HAL_I2C_Slave_Seq_Transmit_IT(a, b, c, d);
}

HAL_StatusTypeDef HAL_I2C_Slave_Transmit(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_I2C_Slave_Transmit(a, b, c, d);
	else
		return f0_HAL_I2C_Slave_Transmit(a, b, c, d);
}

HAL_StatusTypeDef HAL_I2C_Slave_Transmit_DMA(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_I2C_Slave_Transmit_DMA(a, b, c);
	else
		return f0_HAL_I2C_Slave_Transmit_DMA(a, b, c);
}

HAL_StatusTypeDef HAL_I2C_Slave_Transmit_IT(I2C_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_I2C_Slave_Transmit_IT(a, b, c);
	else
		return f0_HAL_I2C_Slave_Transmit_IT(a, b, c);
}

HAL_StatusTypeDef HAL_HalfDuplex_EnableReceiver(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_HalfDuplex_EnableReceiver(a);
	else
		return f0_HAL_HalfDuplex_EnableReceiver(a);
}

HAL_StatusTypeDef HAL_HalfDuplex_EnableTransmitter(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_HalfDuplex_EnableTransmitter(a);
	else
		return f0_HAL_HalfDuplex_EnableTransmitter(a);
}

HAL_StatusTypeDef HAL_HalfDuplex_Init(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_HalfDuplex_Init(a);
	else
		return f0_HAL_HalfDuplex_Init(a);
}

HAL_StatusTypeDef HAL_LIN_Init(UART_HandleTypeDef * a,  uint32_t b) {
	if (l4)
		return l4_HAL_LIN_Init(a, b);
	else
		return f0_HAL_LIN_Init(a, b);
}

HAL_StatusTypeDef HAL_LIN_SendBreak(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_LIN_SendBreak(a);
	else
		return f0_HAL_LIN_SendBreak(a);
}

HAL_StatusTypeDef HAL_MultiProcessor_DisableMuteMode(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_MultiProcessor_DisableMuteMode(a);
	else
		return f0_HAL_MultiProcessor_DisableMuteMode(a);
}

HAL_StatusTypeDef HAL_MultiProcessor_EnableMuteMode(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_MultiProcessor_EnableMuteMode(a);
	else
		return f0_HAL_MultiProcessor_EnableMuteMode(a);
}

void HAL_MultiProcessor_EnterMuteMode(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_MultiProcessor_EnterMuteMode(a);
	else
		f0_HAL_MultiProcessor_EnterMuteMode(a);
}

HAL_StatusTypeDef HAL_MultiProcessor_Init(UART_HandleTypeDef * a,  uint8_t b,  uint32_t c) {
	if (l4)
		return l4_HAL_MultiProcessor_Init(a, b, c);
	else
		return f0_HAL_MultiProcessor_Init(a, b, c);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef * a,  uint16_t b) {
	if (l4)
		l4_HAL_UARTEx_RxEventCallback(a, b);
	else
		f0_HAL_UARTEx_RxEventCallback(a, b);
}

HAL_StatusTypeDef HAL_UART_Abort(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_Abort(a);
	else
		return f0_HAL_UART_Abort(a);
}

void HAL_UART_AbortCpltCallback(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_UART_AbortCpltCallback(a);
	else
		f0_HAL_UART_AbortCpltCallback(a);
}

HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_AbortReceive(a);
	else
		return f0_HAL_UART_AbortReceive(a);
}

void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_UART_AbortReceiveCpltCallback(a);
	else
		f0_HAL_UART_AbortReceiveCpltCallback(a);
}

HAL_StatusTypeDef HAL_UART_AbortReceive_IT(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_AbortReceive_IT(a);
	else
		return f0_HAL_UART_AbortReceive_IT(a);
}

HAL_StatusTypeDef HAL_UART_AbortTransmit(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_AbortTransmit(a);
	else
		return f0_HAL_UART_AbortTransmit(a);
}

void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_UART_AbortTransmitCpltCallback(a);
	else
		f0_HAL_UART_AbortTransmitCpltCallback(a);
}

HAL_StatusTypeDef HAL_UART_AbortTransmit_IT(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_AbortTransmit_IT(a);
	else
		return f0_HAL_UART_AbortTransmit_IT(a);
}

HAL_StatusTypeDef HAL_UART_Abort_IT(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_Abort_IT(a);
	else
		return f0_HAL_UART_Abort_IT(a);
}

HAL_StatusTypeDef HAL_UART_DMAPause(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_DMAPause(a);
	else
		return f0_HAL_UART_DMAPause(a);
}

HAL_StatusTypeDef HAL_UART_DMAResume(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_DMAResume(a);
	else
		return f0_HAL_UART_DMAResume(a);
}

HAL_StatusTypeDef HAL_UART_DMAStop(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_DMAStop(a);
	else
		return f0_HAL_UART_DMAStop(a);
}

HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_DeInit(a);
	else
		return f0_HAL_UART_DeInit(a);
}

HAL_StatusTypeDef HAL_UART_DisableReceiverTimeout(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_DisableReceiverTimeout(a);
	else
		return f0_HAL_UART_DisableReceiverTimeout(a);
}

HAL_StatusTypeDef HAL_UART_EnableReceiverTimeout(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_EnableReceiverTimeout(a);
	else
		return f0_HAL_UART_EnableReceiverTimeout(a);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_UART_ErrorCallback(a);
	else
		f0_HAL_UART_ErrorCallback(a);
}

uint32_t HAL_UART_GetError(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_GetError(a);
	else
		return f0_HAL_UART_GetError(a);
}

HAL_UART_StateTypeDef HAL_UART_GetState(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_GetState(a);
	else
		return f0_HAL_UART_GetState(a);
}

void HAL_UART_IRQHandler(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_UART_IRQHandler(a);
	else
		f0_HAL_UART_IRQHandler(a);
}

HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_UART_Init(a);
	else
		return f0_HAL_UART_Init(a);
}

HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_UART_Receive(a, b, c, d);
	else
		return f0_HAL_UART_Receive(a, b, c, d);
}

HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_UART_Receive_DMA(a, b, c);
	else
		return f0_HAL_UART_Receive_DMA(a, b, c);
}

HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_UART_Receive_IT(a, b, c);
	else
		return f0_HAL_UART_Receive_IT(a, b, c);
}

void HAL_UART_ReceiverTimeout_Config(UART_HandleTypeDef * a,  uint32_t b) {
	if (l4)
		l4_HAL_UART_ReceiverTimeout_Config(a, b);
	else
		f0_HAL_UART_ReceiverTimeout_Config(a, b);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_UART_RxCpltCallback(a);
	else
		f0_HAL_UART_RxCpltCallback(a);
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_UART_RxHalfCpltCallback(a);
	else
		f0_HAL_UART_RxHalfCpltCallback(a);
}

HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_UART_Transmit(a, b, c, d);
	else
		return f0_HAL_UART_Transmit(a, b, c, d);
}

HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_UART_Transmit_DMA(a, b, c);
	else
		return f0_HAL_UART_Transmit_DMA(a, b, c);
}

HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_UART_Transmit_IT(a, b, c);
	else
		return f0_HAL_UART_Transmit_IT(a, b, c);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_UART_TxCpltCallback(a);
	else
		f0_HAL_UART_TxCpltCallback(a);
}

void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef * a) {
	if (l4)
		l4_HAL_UART_TxHalfCpltCallback(a);
	else
		f0_HAL_UART_TxHalfCpltCallback(a);
}

void UART_AdvFeatureConfig(UART_HandleTypeDef * a) {
	if (l4)
		l4_UART_AdvFeatureConfig(a);
	else
		f0_UART_AdvFeatureConfig(a);
}

HAL_StatusTypeDef UART_CheckIdleState(UART_HandleTypeDef * a) {
	if (l4)
		return l4_UART_CheckIdleState(a);
	else
		return f0_UART_CheckIdleState(a);
}

HAL_StatusTypeDef UART_SetConfig(UART_HandleTypeDef * a) {
	if (l4)
		return l4_UART_SetConfig(a);
	else
		return f0_UART_SetConfig(a);
}

HAL_StatusTypeDef UART_Start_Receive_DMA(UART_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_UART_Start_Receive_DMA(a, b, c);
	else
		return f0_UART_Start_Receive_DMA(a, b, c);
}

HAL_StatusTypeDef UART_Start_Receive_IT(UART_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_UART_Start_Receive_IT(a, b, c);
	else
		return f0_UART_Start_Receive_IT(a, b, c);
}

HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef * a,  uint32_t b,  FlagStatus c,  uint32_t d,  uint32_t e) {
	if (l4)
		return l4_UART_WaitOnFlagUntilTimeout(a, b, c, d, e);
	else
		return f0_UART_WaitOnFlagUntilTimeout(a, b, c, d, e);
}

HAL_StatusTypeDef HAL_SPI_Abort(SPI_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_SPI_Abort(a);
	else
		return f0_HAL_SPI_Abort(a);
}

void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef * a) {
	if (l4)
		l4_HAL_SPI_AbortCpltCallback(a);
	else
		f0_HAL_SPI_AbortCpltCallback(a);
}

HAL_StatusTypeDef HAL_SPI_Abort_IT(SPI_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_SPI_Abort_IT(a);
	else
		return f0_HAL_SPI_Abort_IT(a);
}

HAL_StatusTypeDef HAL_SPI_DMAPause(SPI_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_SPI_DMAPause(a);
	else
		return f0_HAL_SPI_DMAPause(a);
}

HAL_StatusTypeDef HAL_SPI_DMAResume(SPI_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_SPI_DMAResume(a);
	else
		return f0_HAL_SPI_DMAResume(a);
}

HAL_StatusTypeDef HAL_SPI_DMAStop(SPI_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_SPI_DMAStop(a);
	else
		return f0_HAL_SPI_DMAStop(a);
}

HAL_StatusTypeDef HAL_SPI_DeInit(SPI_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_SPI_DeInit(a);
	else
		return f0_HAL_SPI_DeInit(a);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef * a) {
	if (l4)
		l4_HAL_SPI_ErrorCallback(a);
	else
		f0_HAL_SPI_ErrorCallback(a);
}

uint32_t HAL_SPI_GetError(SPI_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_SPI_GetError(a);
	else
		return f0_HAL_SPI_GetError(a);
}

HAL_SPI_StateTypeDef HAL_SPI_GetState(SPI_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_SPI_GetState(a);
	else
		return f0_HAL_SPI_GetState(a);
}

void HAL_SPI_IRQHandler(SPI_HandleTypeDef * a) {
	if (l4)
		l4_HAL_SPI_IRQHandler(a);
	else
		f0_HAL_SPI_IRQHandler(a);
}

HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_SPI_Init(a);
	else
		return f0_HAL_SPI_Init(a);
}

HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_SPI_Receive(a, b, c, d);
	else
		return f0_HAL_SPI_Receive(a, b, c, d);
}

HAL_StatusTypeDef HAL_SPI_Receive_DMA(SPI_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_SPI_Receive_DMA(a, b, c);
	else
		return f0_HAL_SPI_Receive_DMA(a, b, c);
}

HAL_StatusTypeDef HAL_SPI_Receive_IT(SPI_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_SPI_Receive_IT(a, b, c);
	else
		return f0_HAL_SPI_Receive_IT(a, b, c);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef * a) {
	if (l4)
		l4_HAL_SPI_RxCpltCallback(a);
	else
		f0_HAL_SPI_RxCpltCallback(a);
}

void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef * a) {
	if (l4)
		l4_HAL_SPI_RxHalfCpltCallback(a);
	else
		f0_HAL_SPI_RxHalfCpltCallback(a);
}

HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef * a,  uint8_t * b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_SPI_Transmit(a, b, c, d);
	else
		return f0_HAL_SPI_Transmit(a, b, c, d);
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef * a,  uint8_t * b,  uint8_t * c,  uint16_t d,  uint32_t e) {
	if (l4)
		return l4_HAL_SPI_TransmitReceive(a, b, c, d, e);
	else
		return f0_HAL_SPI_TransmitReceive(a, b, c, d, e);
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive_DMA(SPI_HandleTypeDef * a,  uint8_t * b,  uint8_t * c,  uint16_t d) {
	if (l4)
		return l4_HAL_SPI_TransmitReceive_DMA(a, b, c, d);
	else
		return f0_HAL_SPI_TransmitReceive_DMA(a, b, c, d);
}

HAL_StatusTypeDef HAL_SPI_TransmitReceive_IT(SPI_HandleTypeDef * a,  uint8_t * b,  uint8_t * c,  uint16_t d) {
	if (l4)
		return l4_HAL_SPI_TransmitReceive_IT(a, b, c, d);
	else
		return f0_HAL_SPI_TransmitReceive_IT(a, b, c, d);
}

HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_SPI_Transmit_DMA(a, b, c);
	else
		return f0_HAL_SPI_Transmit_DMA(a, b, c);
}

HAL_StatusTypeDef HAL_SPI_Transmit_IT(SPI_HandleTypeDef * a,  uint8_t * b,  uint16_t c) {
	if (l4)
		return l4_HAL_SPI_Transmit_IT(a, b, c);
	else
		return f0_HAL_SPI_Transmit_IT(a, b, c);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef * a) {
	if (l4)
		l4_HAL_SPI_TxCpltCallback(a);
	else
		f0_HAL_SPI_TxCpltCallback(a);
}

void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef * a) {
	if (l4)
		l4_HAL_SPI_TxHalfCpltCallback(a);
	else
		f0_HAL_SPI_TxHalfCpltCallback(a);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef * a) {
	if (l4)
		l4_HAL_SPI_TxRxCpltCallback(a);
	else
		f0_HAL_SPI_TxRxCpltCallback(a);
}

void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef * a) {
	if (l4)
		l4_HAL_SPI_TxRxHalfCpltCallback(a);
	else
		f0_HAL_SPI_TxRxHalfCpltCallback(a);
}

void HAL_RCC_CSSCallback() {
	if (l4)
		l4_HAL_RCC_CSSCallback();
	else
		f0_HAL_RCC_CSSCallback();
}

HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef * a,  uint32_t b) {
	if (l4)
		return l4_HAL_RCC_ClockConfig(a, b);
	else
		return f0_HAL_RCC_ClockConfig(a, b);
}

HAL_StatusTypeDef HAL_RCC_DeInit() {
	if (l4)
		return l4_HAL_RCC_DeInit();
	else
		return f0_HAL_RCC_DeInit();
}

void HAL_RCC_DisableCSS() {
	if (l4)
		l4_HAL_RCC_DisableCSS();
	else
		f0_HAL_RCC_DisableCSS();
}

void HAL_RCC_EnableCSS() {
	if (l4)
		l4_HAL_RCC_EnableCSS();
	else
		f0_HAL_RCC_EnableCSS();
}

void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef * a,  uint32_t * b) {
	if (l4)
		l4_HAL_RCC_GetClockConfig(a, b);
	else
		f0_HAL_RCC_GetClockConfig(a, b);
}

uint32_t HAL_RCC_GetHCLKFreq() {
	if (l4)
		return l4_HAL_RCC_GetHCLKFreq();
	else
		return f0_HAL_RCC_GetHCLKFreq();
}

void HAL_RCC_GetOscConfig(RCC_OscInitTypeDef * a) {
	if (l4)
		l4_HAL_RCC_GetOscConfig(a);
	else
		f0_HAL_RCC_GetOscConfig(a);
}

uint32_t HAL_RCC_GetPCLK1Freq() {
	if (l4)
		return l4_HAL_RCC_GetPCLK1Freq();
	else
		return f0_HAL_RCC_GetPCLK1Freq();
}

uint32_t HAL_RCC_GetSysClockFreq() {
	if (l4)
		return l4_HAL_RCC_GetSysClockFreq();
	else
		return f0_HAL_RCC_GetSysClockFreq();
}

void HAL_RCC_MCOConfig(uint32_t a,  uint32_t b,  uint32_t c) {
	if (l4)
		l4_HAL_RCC_MCOConfig(a, b, c);
	else
		f0_HAL_RCC_MCOConfig(a, b, c);
}

void HAL_RCC_NMI_IRQHandler() {
	if (l4)
		l4_HAL_RCC_NMI_IRQHandler();
	else
		f0_HAL_RCC_NMI_IRQHandler();
}

HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef * a) {
	if (l4)
		return l4_HAL_RCC_OscConfig(a);
	else
		return f0_HAL_RCC_OscConfig(a);
}

HAL_StatusTypeDef HAL_PCDEx_ActivateBCD(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCDEx_ActivateBCD(a);
	else
		return f0_HAL_PCDEx_ActivateBCD(a);
}

HAL_StatusTypeDef HAL_PCDEx_ActivateLPM(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCDEx_ActivateLPM(a);
	else
		return f0_HAL_PCDEx_ActivateLPM(a);
}

void HAL_PCDEx_BCD_Callback(PCD_HandleTypeDef * a,  PCD_BCD_MsgTypeDef b) {
	if (l4)
		l4_HAL_PCDEx_BCD_Callback(a, b);
	else
		f0_HAL_PCDEx_BCD_Callback(a, b);
}

void HAL_PCDEx_BCD_VBUSDetect(PCD_HandleTypeDef * a) {
	if (l4)
		l4_HAL_PCDEx_BCD_VBUSDetect(a);
	else
		f0_HAL_PCDEx_BCD_VBUSDetect(a);
}

HAL_StatusTypeDef HAL_PCDEx_DeActivateBCD(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCDEx_DeActivateBCD(a);
	else
		return f0_HAL_PCDEx_DeActivateBCD(a);
}

HAL_StatusTypeDef HAL_PCDEx_DeActivateLPM(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCDEx_DeActivateLPM(a);
	else
		return f0_HAL_PCDEx_DeActivateLPM(a);
}

HAL_StatusTypeDef HAL_PCDEx_PMAConfig(PCD_HandleTypeDef * a,  uint16_t b,  uint16_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_PCDEx_PMAConfig(a, b, c, d);
	else
		return f0_HAL_PCDEx_PMAConfig(a, b, c, d);
}

void HAL_DBGMCU_DisableDBGStandbyMode() {
	if (l4)
		l4_HAL_DBGMCU_DisableDBGStandbyMode();
	else
		f0_HAL_DBGMCU_DisableDBGStandbyMode();
}

void HAL_DBGMCU_DisableDBGStopMode() {
	if (l4)
		l4_HAL_DBGMCU_DisableDBGStopMode();
	else
		f0_HAL_DBGMCU_DisableDBGStopMode();
}

void HAL_DBGMCU_EnableDBGStandbyMode() {
	if (l4)
		l4_HAL_DBGMCU_EnableDBGStandbyMode();
	else
		f0_HAL_DBGMCU_EnableDBGStandbyMode();
}

void HAL_DBGMCU_EnableDBGStopMode() {
	if (l4)
		l4_HAL_DBGMCU_EnableDBGStopMode();
	else
		f0_HAL_DBGMCU_EnableDBGStopMode();
}

HAL_StatusTypeDef HAL_DeInit() {
	if (l4)
		return l4_HAL_DeInit();
	else
		return f0_HAL_DeInit();
}

void HAL_Delay(uint32_t a) {
	if (l4)
		l4_HAL_Delay(a);
	else
		f0_HAL_Delay(a);
}

uint32_t HAL_GetDEVID() {
	if (l4)
		return l4_HAL_GetDEVID();
	else
		return f0_HAL_GetDEVID();
}

uint32_t HAL_GetHalVersion() {
	if (l4)
		return l4_HAL_GetHalVersion();
	else
		return f0_HAL_GetHalVersion();
}

uint32_t HAL_GetREVID() {
	if (l4)
		return l4_HAL_GetREVID();
	else
		return f0_HAL_GetREVID();
}

uint32_t HAL_GetTick() {
	if (l4)
		return l4_HAL_GetTick();
	else
		return f0_HAL_GetTick();
}

HAL_TickFreqTypeDef HAL_GetTickFreq() {
	if (l4)
		return l4_HAL_GetTickFreq();
	else
		return f0_HAL_GetTickFreq();
}

uint32_t HAL_GetTickPrio() {
	if (l4)
		return l4_HAL_GetTickPrio();
	else
		return f0_HAL_GetTickPrio();
}

uint32_t HAL_GetUIDw0() {
	if (l4)
		return l4_HAL_GetUIDw0();
	else
		return f0_HAL_GetUIDw0();
}

uint32_t HAL_GetUIDw1() {
	if (l4)
		return l4_HAL_GetUIDw1();
	else
		return f0_HAL_GetUIDw1();
}

uint32_t HAL_GetUIDw2() {
	if (l4)
		return l4_HAL_GetUIDw2();
	else
		return f0_HAL_GetUIDw2();
}

void HAL_IncTick() {
	if (l4)
		l4_HAL_IncTick();
	else
		f0_HAL_IncTick();
}

HAL_StatusTypeDef HAL_Init() {
	if (l4)
		return l4_HAL_Init();
	else
		return f0_HAL_Init();
}

HAL_StatusTypeDef HAL_InitTick(uint32_t a) {
	if (l4)
		return l4_HAL_InitTick(a);
	else
		return f0_HAL_InitTick(a);
}

void HAL_MspDeInit() {
	if (l4)
		l4_HAL_MspDeInit();
	else
		f0_HAL_MspDeInit();
}

void HAL_ResumeTick() {
	if (l4)
		l4_HAL_ResumeTick();
	else
		f0_HAL_ResumeTick();
}

HAL_StatusTypeDef HAL_SetTickFreq(HAL_TickFreqTypeDef a) {
	if (l4)
		return l4_HAL_SetTickFreq(a);
	else
		return f0_HAL_SetTickFreq(a);
}

void HAL_SuspendTick() {
	if (l4)
		l4_HAL_SuspendTick();
	else
		f0_HAL_SuspendTick();
}

void HAL_PWR_DeInit() {
	if (l4)
		l4_HAL_PWR_DeInit();
	else
		f0_HAL_PWR_DeInit();
}

void HAL_PWR_DisableBkUpAccess() {
	if (l4)
		l4_HAL_PWR_DisableBkUpAccess();
	else
		f0_HAL_PWR_DisableBkUpAccess();
}

void HAL_PWR_DisableSEVOnPend() {
	if (l4)
		l4_HAL_PWR_DisableSEVOnPend();
	else
		f0_HAL_PWR_DisableSEVOnPend();
}

void HAL_PWR_DisableSleepOnExit() {
	if (l4)
		l4_HAL_PWR_DisableSleepOnExit();
	else
		f0_HAL_PWR_DisableSleepOnExit();
}

void HAL_PWR_DisableWakeUpPin(uint32_t a) {
	if (l4)
		l4_HAL_PWR_DisableWakeUpPin(a);
	else
		f0_HAL_PWR_DisableWakeUpPin(a);
}

void HAL_PWR_EnableBkUpAccess() {
	if (l4)
		l4_HAL_PWR_EnableBkUpAccess();
	else
		f0_HAL_PWR_EnableBkUpAccess();
}

void HAL_PWR_EnableSEVOnPend() {
	if (l4)
		l4_HAL_PWR_EnableSEVOnPend();
	else
		f0_HAL_PWR_EnableSEVOnPend();
}

void HAL_PWR_EnableSleepOnExit() {
	if (l4)
		l4_HAL_PWR_EnableSleepOnExit();
	else
		f0_HAL_PWR_EnableSleepOnExit();
}

void HAL_PWR_EnableWakeUpPin(uint32_t a) {
	if (l4)
		l4_HAL_PWR_EnableWakeUpPin(a);
	else
		f0_HAL_PWR_EnableWakeUpPin(a);
}

void HAL_PWR_EnterSLEEPMode(uint32_t a,  uint8_t b) {
	if (l4)
		l4_HAL_PWR_EnterSLEEPMode(a, b);
	else
		f0_HAL_PWR_EnterSLEEPMode(a, b);
}

void HAL_PWR_EnterSTANDBYMode() {
	if (l4)
		l4_HAL_PWR_EnterSTANDBYMode();
	else
		f0_HAL_PWR_EnterSTANDBYMode();
}

void HAL_PWR_EnterSTOPMode(uint32_t a,  uint8_t b) {
	if (l4)
		l4_HAL_PWR_EnterSTOPMode(a, b);
	else
		f0_HAL_PWR_EnterSTOPMode(a, b);
}

void HAL_RCCEx_CRSConfig(RCC_CRSInitTypeDef * a) {
	if (l4)
		l4_HAL_RCCEx_CRSConfig(a);
	else
		f0_HAL_RCCEx_CRSConfig(a);
}

void HAL_RCCEx_CRSGetSynchronizationInfo(RCC_CRSSynchroInfoTypeDef * a) {
	if (l4)
		l4_HAL_RCCEx_CRSGetSynchronizationInfo(a);
	else
		f0_HAL_RCCEx_CRSGetSynchronizationInfo(a);
}

void HAL_RCCEx_CRSSoftwareSynchronizationGenerate() {
	if (l4)
		l4_HAL_RCCEx_CRSSoftwareSynchronizationGenerate();
	else
		f0_HAL_RCCEx_CRSSoftwareSynchronizationGenerate();
}

uint32_t HAL_RCCEx_CRSWaitSynchronization(uint32_t a) {
	if (l4)
		return l4_HAL_RCCEx_CRSWaitSynchronization(a);
	else
		return f0_HAL_RCCEx_CRSWaitSynchronization(a);
}

void HAL_RCCEx_CRS_ErrorCallback(uint32_t a) {
	if (l4)
		l4_HAL_RCCEx_CRS_ErrorCallback(a);
	else
		f0_HAL_RCCEx_CRS_ErrorCallback(a);
}

void HAL_RCCEx_CRS_ExpectedSyncCallback() {
	if (l4)
		l4_HAL_RCCEx_CRS_ExpectedSyncCallback();
	else
		f0_HAL_RCCEx_CRS_ExpectedSyncCallback();
}

void HAL_RCCEx_CRS_IRQHandler() {
	if (l4)
		l4_HAL_RCCEx_CRS_IRQHandler();
	else
		f0_HAL_RCCEx_CRS_IRQHandler();
}

void HAL_RCCEx_CRS_SyncOkCallback() {
	if (l4)
		l4_HAL_RCCEx_CRS_SyncOkCallback();
	else
		f0_HAL_RCCEx_CRS_SyncOkCallback();
}

void HAL_RCCEx_CRS_SyncWarnCallback() {
	if (l4)
		l4_HAL_RCCEx_CRS_SyncWarnCallback();
	else
		f0_HAL_RCCEx_CRS_SyncWarnCallback();
}

void HAL_RCCEx_GetPeriphCLKConfig(RCC_PeriphCLKInitTypeDef * a) {
	if (l4)
		l4_HAL_RCCEx_GetPeriphCLKConfig(a);
	else
		f0_HAL_RCCEx_GetPeriphCLKConfig(a);
}

uint32_t HAL_RCCEx_GetPeriphCLKFreq(uint32_t a) {
	if (l4)
		return l4_HAL_RCCEx_GetPeriphCLKFreq(a);
	else
		return f0_HAL_RCCEx_GetPeriphCLKFreq(a);
}

HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(RCC_PeriphCLKInitTypeDef * a) {
	if (l4)
		return l4_HAL_RCCEx_PeriphCLKConfig(a);
	else
		return f0_HAL_RCCEx_PeriphCLKConfig(a);
}

HAL_StatusTypeDef HAL_PCD_ActivateRemoteWakeup(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCD_ActivateRemoteWakeup(a);
	else
		return f0_HAL_PCD_ActivateRemoteWakeup(a);
}

HAL_StatusTypeDef HAL_PCD_DeActivateRemoteWakeup(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCD_DeActivateRemoteWakeup(a);
	else
		return f0_HAL_PCD_DeActivateRemoteWakeup(a);
}

HAL_StatusTypeDef HAL_PCD_DeInit(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCD_DeInit(a);
	else
		return f0_HAL_PCD_DeInit(a);
}

HAL_StatusTypeDef HAL_PCD_DevConnect(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCD_DevConnect(a);
	else
		return f0_HAL_PCD_DevConnect(a);
}

HAL_StatusTypeDef HAL_PCD_DevDisconnect(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCD_DevDisconnect(a);
	else
		return f0_HAL_PCD_DevDisconnect(a);
}

HAL_StatusTypeDef HAL_PCD_EP_Close(PCD_HandleTypeDef * a,  uint8_t b) {
	if (l4)
		return l4_HAL_PCD_EP_Close(a, b);
	else
		return f0_HAL_PCD_EP_Close(a, b);
}

HAL_StatusTypeDef HAL_PCD_EP_ClrStall(PCD_HandleTypeDef * a,  uint8_t b) {
	if (l4)
		return l4_HAL_PCD_EP_ClrStall(a, b);
	else
		return f0_HAL_PCD_EP_ClrStall(a, b);
}

HAL_StatusTypeDef HAL_PCD_EP_Flush(PCD_HandleTypeDef * a,  uint8_t b) {
	if (l4)
		return l4_HAL_PCD_EP_Flush(a, b);
	else
		return f0_HAL_PCD_EP_Flush(a, b);
}

uint32_t HAL_PCD_EP_GetRxCount(PCD_HandleTypeDef * a,  uint8_t b) {
	if (l4)
		return l4_HAL_PCD_EP_GetRxCount(a, b);
	else
		return f0_HAL_PCD_EP_GetRxCount(a, b);
}

HAL_StatusTypeDef HAL_PCD_EP_Open(PCD_HandleTypeDef * a,  uint8_t b,  uint16_t c,  uint8_t d) {
	if (l4)
		return l4_HAL_PCD_EP_Open(a, b, c, d);
	else
		return f0_HAL_PCD_EP_Open(a, b, c, d);
}

HAL_StatusTypeDef HAL_PCD_EP_Receive(PCD_HandleTypeDef * a,  uint8_t b,  uint8_t * c,  uint32_t d) {
	if (l4)
		return l4_HAL_PCD_EP_Receive(a, b, c, d);
	else
		return f0_HAL_PCD_EP_Receive(a, b, c, d);
}

HAL_StatusTypeDef HAL_PCD_EP_SetStall(PCD_HandleTypeDef * a,  uint8_t b) {
	if (l4)
		return l4_HAL_PCD_EP_SetStall(a, b);
	else
		return f0_HAL_PCD_EP_SetStall(a, b);
}

HAL_StatusTypeDef HAL_PCD_EP_Transmit(PCD_HandleTypeDef * a,  uint8_t b,  uint8_t * c,  uint32_t d) {
	if (l4)
		return l4_HAL_PCD_EP_Transmit(a, b, c, d);
	else
		return f0_HAL_PCD_EP_Transmit(a, b, c, d);
}

PCD_StateTypeDef HAL_PCD_GetState(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCD_GetState(a);
	else
		return f0_HAL_PCD_GetState(a);
}

void HAL_PCD_IRQHandler(PCD_HandleTypeDef * a) {
	if (l4)
		l4_HAL_PCD_IRQHandler(a);
	else
		f0_HAL_PCD_IRQHandler(a);
}

HAL_StatusTypeDef HAL_PCD_Init(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCD_Init(a);
	else
		return f0_HAL_PCD_Init(a);
}

HAL_StatusTypeDef HAL_PCD_SetAddress(PCD_HandleTypeDef * a,  uint8_t b) {
	if (l4)
		return l4_HAL_PCD_SetAddress(a, b);
	else
		return f0_HAL_PCD_SetAddress(a, b);
}

HAL_StatusTypeDef HAL_PCD_Start(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCD_Start(a);
	else
		return f0_HAL_PCD_Start(a);
}

HAL_StatusTypeDef HAL_PCD_Stop(PCD_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_PCD_Stop(a);
	else
		return f0_HAL_PCD_Stop(a);
}

void HAL_NVIC_ClearPendingIRQ(IRQn_Type a) {
	if (l4)
		l4_HAL_NVIC_ClearPendingIRQ(a);
	else
		f0_HAL_NVIC_ClearPendingIRQ(a);
}

void HAL_NVIC_DisableIRQ(IRQn_Type a) {
	if (l4)
		l4_HAL_NVIC_DisableIRQ(a);
	else
		f0_HAL_NVIC_DisableIRQ(a);
}

void HAL_NVIC_EnableIRQ(IRQn_Type a) {
	if (l4)
		l4_HAL_NVIC_EnableIRQ(a);
	else
		f0_HAL_NVIC_EnableIRQ(a);
}

uint32_t HAL_NVIC_GetPendingIRQ(IRQn_Type a) {
	if (l4)
		return l4_HAL_NVIC_GetPendingIRQ(a);
	else
		return f0_HAL_NVIC_GetPendingIRQ(a);
}

uint32_t HAL_NVIC_GetPriority(IRQn_Type a) {
	if (l4)
		return l4_HAL_NVIC_GetPriority(a);
	else
		return f0_HAL_NVIC_GetPriority(a);
}

void HAL_NVIC_SetPendingIRQ(IRQn_Type a) {
	if (l4)
		l4_HAL_NVIC_SetPendingIRQ(a);
	else
		f0_HAL_NVIC_SetPendingIRQ(a);
}

void HAL_NVIC_SetPriority(IRQn_Type a,  uint32_t b,  uint32_t c) {
	if (l4)
		l4_HAL_NVIC_SetPriority(a, b, c);
	else
		f0_HAL_NVIC_SetPriority(a, b, c);
}

void HAL_NVIC_SystemReset() {
	if (l4)
		l4_HAL_NVIC_SystemReset();
	else
		f0_HAL_NVIC_SystemReset();
}

void HAL_SYSTICK_CLKSourceConfig(uint32_t a) {
	if (l4)
		l4_HAL_SYSTICK_CLKSourceConfig(a);
	else
		f0_HAL_SYSTICK_CLKSourceConfig(a);
}

void HAL_SYSTICK_Callback() {
	if (l4)
		l4_HAL_SYSTICK_Callback();
	else
		f0_HAL_SYSTICK_Callback();
}

uint32_t HAL_SYSTICK_Config(uint32_t a) {
	if (l4)
		return l4_HAL_SYSTICK_Config(a);
	else
		return f0_HAL_SYSTICK_Config(a);
}

void HAL_SYSTICK_IRQHandler() {
	if (l4)
		l4_HAL_SYSTICK_IRQHandler();
	else
		f0_HAL_SYSTICK_IRQHandler();
}

void HAL_PWREx_DisableVddio2Monitor() {
	if (l4)
		l4_HAL_PWREx_DisableVddio2Monitor();
	else
		f0_HAL_PWREx_DisableVddio2Monitor();
}

void HAL_PWREx_EnableVddio2Monitor() {
	if (l4)
		l4_HAL_PWREx_EnableVddio2Monitor();
	else
		f0_HAL_PWREx_EnableVddio2Monitor();
}

void HAL_PWREx_Vddio2MonitorCallback() {
	if (l4)
		l4_HAL_PWREx_Vddio2MonitorCallback();
	else
		f0_HAL_PWREx_Vddio2MonitorCallback();
}

void HAL_PWREx_Vddio2Monitor_IRQHandler() {
	if (l4)
		l4_HAL_PWREx_Vddio2Monitor_IRQHandler();
	else
		f0_HAL_PWREx_Vddio2Monitor_IRQHandler();
}

void HAL_PWR_ConfigPVD(PWR_PVDTypeDef * a) {
	if (l4)
		l4_HAL_PWR_ConfigPVD(a);
	else
		f0_HAL_PWR_ConfigPVD(a);
}

void HAL_PWR_DisablePVD() {
	if (l4)
		l4_HAL_PWR_DisablePVD();
	else
		f0_HAL_PWR_DisablePVD();
}

void HAL_PWR_EnablePVD() {
	if (l4)
		l4_HAL_PWR_EnablePVD();
	else
		f0_HAL_PWR_EnablePVD();
}

void HAL_PWR_PVDCallback() {
	if (l4)
		l4_HAL_PWR_PVDCallback();
	else
		f0_HAL_PWR_PVDCallback();
}

void HAL_PWR_PVD_IRQHandler() {
	if (l4)
		l4_HAL_PWR_PVD_IRQHandler();
	else
		f0_HAL_PWR_PVD_IRQHandler();
}

void HAL_GPIO_DeInit(GPIO_TypeDef * a,  uint32_t b) {
	if (l4)
		l4_HAL_GPIO_DeInit(a, b);
	else
		f0_HAL_GPIO_DeInit(a, b);
}

void HAL_GPIO_EXTI_IRQHandler(uint16_t a) {
	if (l4)
		l4_HAL_GPIO_EXTI_IRQHandler(a);
	else
		f0_HAL_GPIO_EXTI_IRQHandler(a);
}

void HAL_GPIO_Init(GPIO_TypeDef * a,  GPIO_InitTypeDef * b) {
	if (l4)
		l4_HAL_GPIO_Init(a, b);
	else
		f0_HAL_GPIO_Init(a, b);
}

HAL_StatusTypeDef HAL_GPIO_LockPin(GPIO_TypeDef * a,  uint16_t b) {
	if (l4)
		return l4_HAL_GPIO_LockPin(a, b);
	else
		return f0_HAL_GPIO_LockPin(a, b);
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef * a,  uint16_t b) {
	if (l4)
		return l4_HAL_GPIO_ReadPin(a, b);
	else
		return f0_HAL_GPIO_ReadPin(a, b);
}

void HAL_GPIO_TogglePin(GPIO_TypeDef * a,  uint16_t b) {
	if (l4)
		l4_HAL_GPIO_TogglePin(a, b);
	else
		f0_HAL_GPIO_TogglePin(a, b);
}

void HAL_GPIO_WritePin(GPIO_TypeDef * a,  uint16_t b,  GPIO_PinState c) {
	if (l4)
		l4_HAL_GPIO_WritePin(a, b, c);
	else
		f0_HAL_GPIO_WritePin(a, b, c);
}

HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_DMA_Abort(a);
	else
		return f0_HAL_DMA_Abort(a);
}

HAL_StatusTypeDef HAL_DMA_Abort_IT(DMA_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_DMA_Abort_IT(a);
	else
		return f0_HAL_DMA_Abort_IT(a);
}

HAL_StatusTypeDef HAL_DMA_DeInit(DMA_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_DMA_DeInit(a);
	else
		return f0_HAL_DMA_DeInit(a);
}

uint32_t HAL_DMA_GetError(DMA_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_DMA_GetError(a);
	else
		return f0_HAL_DMA_GetError(a);
}

HAL_DMA_StateTypeDef HAL_DMA_GetState(DMA_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_DMA_GetState(a);
	else
		return f0_HAL_DMA_GetState(a);
}

void HAL_DMA_IRQHandler(DMA_HandleTypeDef * a) {
	if (l4)
		l4_HAL_DMA_IRQHandler(a);
	else
		f0_HAL_DMA_IRQHandler(a);
}

HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef * a) {
	if (l4)
		return l4_HAL_DMA_Init(a);
	else
		return f0_HAL_DMA_Init(a);
}

HAL_StatusTypeDef HAL_DMA_PollForTransfer(DMA_HandleTypeDef * a,  uint32_t b,  uint32_t c) {
	if (l4)
		return l4_HAL_DMA_PollForTransfer(a, b, c);
	else
		return f0_HAL_DMA_PollForTransfer(a, b, c);
}

HAL_StatusTypeDef HAL_DMA_Start(DMA_HandleTypeDef * a,  uint32_t b,  uint32_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_DMA_Start(a, b, c, d);
	else
		return f0_HAL_DMA_Start(a, b, c, d);
}

HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef * a,  uint32_t b,  uint32_t c,  uint32_t d) {
	if (l4)
		return l4_HAL_DMA_Start_IT(a, b, c, d);
	else
		return f0_HAL_DMA_Start_IT(a, b, c, d);
}

HAL_StatusTypeDef HAL_DMA_UnRegisterCallback(DMA_HandleTypeDef * a,  HAL_DMA_CallbackIDTypeDef b) {
	if (l4)
		return l4_HAL_DMA_UnRegisterCallback(a, b);
	else
		return f0_HAL_DMA_UnRegisterCallback(a, b);
}

