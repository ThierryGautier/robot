
#ifndef MOTION_CONTROL_SRC_HDLC_H_
#define MOTION_CONTROL_SRC_HDLC_H_

#define HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME        (UI08)32U

BOOL HDLC_bInitialize(CHAR pcDeviceName[],
                      BOOL (*bOpenDevice)(CHAR pcDeviceName[]),
                      UI32 (*u32GetRxData) (UI08 *pu8RxBuffer, UI32 u32Size),
                      UI32 (*u32PutTxData) (UI08* pu8TxBuffer, UI32 u32Size),
                      BOOL (*bCloseDevice)(void));
BOOL HDLC_bGetFrame(UI08 *pu8RxFrame, UI08 *pu8RxSize);
BOOL HDLC_bPutFrame(UI08 *pu8TxFrame, UI08 *pu8TxSize);
void HDLC_Close(void);

#endif /* MOTION_CONTROL_SRC_HDLC_H_ */
