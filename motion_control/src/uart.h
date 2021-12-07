#ifndef MOTION_CONTROL_SRC_UART_H_
#define MOTION_CONTROL_SRC_UART_H_

BOOL UART_bOpenDevice (CHAR pcDeviceName[]);
UI32 UART_s32GetRxData(UI08 *pu8RxBuffer, UI32 u32Size);
UI32 UART_s32PutTxData(UI08* pu8TxBuffer, UI32 u32Size);
BOOL UART_bCloseDevice (void);

#endif
