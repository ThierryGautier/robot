#ifndef MOTION_CONTROL_SRC_UART_H_
#define MOTION_CONTROL_SRC_UART_H_

typedef unsigned char  UI08;

BOOL UART_bOpenDevice (char pcDeviceName[]);
BOOL UART_bGetRxChar(UI08 *pu8RxChar);
BOOL UART_bPutTxChar(UI08 u8TxChar);
BOOL UART_bCloseDevice (void);

#endif
