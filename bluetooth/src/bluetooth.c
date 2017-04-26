/*
 ============================================================================
 Name        : bluetooth.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : communication with FRDM KV31 board through a bluetooth connection
               lunix command:  'sudo ./bluetooth /dev/rfcomm0'
               make sure that the /dev/rfcomm0 is enabled before using it
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/fcntl.h>

#include "stdtype.h"
#include "uart.h"
#include "hdlc.h"


int counter;
int FrameIsReceived;
static UI08 au8RxFrame[HDLC_U8_MAX_NB_BYTE_IN_FRAME];
static UI08 u8RxSize;
static UI08 au8TxFrame[HDLC_U8_MAX_NB_BYTE_IN_FRAME];
static UI08 u8TxSize;

static UI08 u8MotorCommand = 1;
static UI08 u8PWMLevelLeft = 0;
static UI08 u8PWMLevelRight = 0;
static UI08 u8Cpt = 0;

int main (int argc, char *argv[])
{
	BOOL bInitHDLCIsOK;

	if(argc != 2)
	{
		printf("bad argument, add device path");
		return(EXIT_FAILURE);
	}

	/* open HDLC link on "/dev/rfcomm*" */
	bInitHDLCIsOK = HDLC_bInitialize(argv[1],
			   	    				 UART_bOpenDevice,
									 UART_bGetRxChar,
									 UART_bPutTxChar,
									 UART_bCloseDevice);

    /* hdlc communication loop */
	while ( bInitHDLCIsOK==TRUE )
	{
		BOOL bRxResult;
		BOOL bTxResult;
#ifdef LOG_BUFFER_HDLC
		UI16 i;
#endif
		//wait read a frame
		bRxResult = HDLC_bGetFrame(au8RxFrame, &u8RxSize);
		if(bRxResult == TRUE)
		{
		    union
			{
		    	float f32Value;
		    	UI32  u32Value;
		    	UI08  au8Data[4];
			}ConvertBytes;

			//analyze the data of the frame
			//print all data of the hdlc frame received
			printf("Frame received:");
#if LOG_BUFFER_HDLC
			for(i=0;i<u8RxSize;i++)
			{
				printf("%2.2x ",au8RxFrame[i]);
			}
#else
	        /*update life byte*/
			printf("Tx Life byte:%2.2x ",au8RxFrame[0]);
			printf("Rx Life byte:%2.2x ",au8RxFrame[1]);

		    /* update linear acceleration (g) X */
			ConvertBytes.au8Data[0] = au8RxFrame[2];
			ConvertBytes.au8Data[1] = au8RxFrame[3];
			ConvertBytes.au8Data[2] = au8RxFrame[4];
     		ConvertBytes.au8Data[3] = au8RxFrame[5];
     		printf("Gx:%10.6f ",ConvertBytes.f32Value);

     		/* update linear acceleration (g) Y */
     		ConvertBytes.au8Data[0] = au8RxFrame[6];
			ConvertBytes.au8Data[1] = au8RxFrame[7];
			ConvertBytes.au8Data[2] = au8RxFrame[8];
     		ConvertBytes.au8Data[3] = au8RxFrame[9];
     		printf("Gy:%10.6f ",ConvertBytes.f32Value);

     		/* update linear acceleration (g) Z */
     		ConvertBytes.au8Data[0] = au8RxFrame[10];
			ConvertBytes.au8Data[1] = au8RxFrame[11];
			ConvertBytes.au8Data[2] = au8RxFrame[12];
     		ConvertBytes.au8Data[3] = au8RxFrame[13];
     		printf("Gz:%10.6f ",ConvertBytes.f32Value);

     		/* update roll (deg) */
			ConvertBytes.au8Data[0] = au8RxFrame[14];
			ConvertBytes.au8Data[1] = au8RxFrame[15];
			ConvertBytes.au8Data[2] = au8RxFrame[16];
     		ConvertBytes.au8Data[3] = au8RxFrame[17];
     		printf("Roll:%10.6f° ",ConvertBytes.f32Value);

     		/* update pitch (deg) */
     		ConvertBytes.au8Data[0] = au8RxFrame[18];
			ConvertBytes.au8Data[1] = au8RxFrame[19];
			ConvertBytes.au8Data[2] = au8RxFrame[20];
     		ConvertBytes.au8Data[3] = au8RxFrame[21];
     		printf("Pitch:%10.6f° ",ConvertBytes.f32Value);

     		/* update compass (deg) */
			ConvertBytes.au8Data[0] = au8RxFrame[22];
			ConvertBytes.au8Data[1] = au8RxFrame[23];
			ConvertBytes.au8Data[2] = au8RxFrame[24];
     		ConvertBytes.au8Data[3] = au8RxFrame[25];
     		printf("Compass:%10.6f°",ConvertBytes.f32Value);

#endif
     		printf("\n");
			//send hdlc response motor command
			au8TxFrame[0] = u8MotorCommand; //motor command (eMotorCommand)
			au8TxFrame[1] = u8PWMLevelLeft; //left motor duty cycle in % (u8PWMLevelLeft)
			au8TxFrame[2] = u8PWMLevelRight; //right motor duty cycle in % (u8PWMLevelRight)
			au8TxFrame[3] = 0x45;
			u8TxSize = 4;

			u8Cpt++;
			if(u8Cpt>10)
			{
				u8PWMLevelLeft++;
				if(u8PWMLevelLeft > 80) u8PWMLevelLeft = 0;
				u8PWMLevelRight++;
				if(u8PWMLevelRight > 50) u8PWMLevelRight= 0;

				u8Cpt=0;
			}
			//send tx frame
			bTxResult = HDLC_bPutFrame(&au8TxFrame[0], &u8TxSize);
			if(bTxResult!=TRUE)
			{
				printf("Error to put a frame\n");
			}
		}
	}
	return EXIT_SUCCESS;
}
