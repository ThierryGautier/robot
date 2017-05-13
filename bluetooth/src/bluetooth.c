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

/* system include */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/fcntl.h>

/* project include */
#include "stdtype.h"
#include "uart.h"
#include "hdlc.h"
#include "joystick.h"
#include "keyboard.h"


/* public variables */
UI08 u8MotorCommand = 0;
UI16 u16PWMLevel = 0;
float f32CompassCommand = 0;
float f32RxeCompass = 0;

/* private variables */
static UI08 au8RxFrame[HDLC_U8_MAX_NB_BYTE_IN_FRAME];
static UI08 u8RxSize;
static UI08 au8TxFrame[HDLC_U8_MAX_NB_BYTE_IN_FRAME];
static UI08 u8TxSize;

static pthread_t tThreadCOMId;
static pthread_t tThreadKeyBoardId;
static pthread_t tThreadJoystickdId;


/* thread in charge to manage HDCL protocol with FRDM_KV31 board */
void* ThreadCOM(void *arg)
{
	static BOOL bInitHDLCIsOK;
	static BOOL bIsTheFirstRxFrame = TRUE;

	printf("arg:%s ",(char *)arg);

	/* open HDLC link on "/dev/rfcomm*" */
	bInitHDLCIsOK = HDLC_bInitialize(arg,
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
			}Convert32Bits;

			union
			{
		    	int16_t  s16Value;
		    	UI16  u16Value;
			    UI08  au8Data[2];
			}Convert16Bits;


			//analyze the data of the frame
			//print all data of the hdlc frame received
			//printf("Frame received:");

	        /*update life byte*/
			//printf("Tx Life byte:%2.2x ",au8RxFrame[0]);
			//printf("Rx Life byte:%2.2x ",au8RxFrame[1]);

			/* update linear acceleration (g) X */
			Convert32Bits.au8Data[0] = au8RxFrame[2];
			Convert32Bits.au8Data[1] = au8RxFrame[3];
			Convert32Bits.au8Data[2] = au8RxFrame[4];
			Convert32Bits.au8Data[3] = au8RxFrame[5];
     		//printf("Gx:%10.6f ",Convert32Bits.f32Value);

     		/* update linear acceleration (g) Y */
			Convert32Bits.au8Data[0] = au8RxFrame[6];
			Convert32Bits.au8Data[1] = au8RxFrame[7];
			Convert32Bits.au8Data[2] = au8RxFrame[8];
			Convert32Bits.au8Data[3] = au8RxFrame[9];
     		//printf("Gy:%10.6f ",Convert32Bits.f32Value);

     		/* update linear acceleration (g) Z */
			Convert32Bits.au8Data[0] = au8RxFrame[10];
			Convert32Bits.au8Data[1] = au8RxFrame[11];
			Convert32Bits.au8Data[2] = au8RxFrame[12];
			Convert32Bits.au8Data[3] = au8RxFrame[13];
     		//printf("Gz:%10.6f ",Convert32Bits.f32Value);

     		/* update roll (deg) */
			Convert32Bits.au8Data[0] = au8RxFrame[14];
			Convert32Bits.au8Data[1] = au8RxFrame[15];
			Convert32Bits.au8Data[2] = au8RxFrame[16];
			Convert32Bits.au8Data[3] = au8RxFrame[17];
     		//printf("Roll:%10.6f° ",Convert32Bits.f32Value);

     		/* update pitch (deg) */
			Convert32Bits.au8Data[0] = au8RxFrame[18];
			Convert32Bits.au8Data[1] = au8RxFrame[19];
			Convert32Bits.au8Data[2] = au8RxFrame[20];
			Convert32Bits.au8Data[3] = au8RxFrame[21];
     		//printf("Pitch:%10.6f° ",Convert32Bits.f32Value);

     		/* update compass (deg) */
			Convert32Bits.au8Data[0] = au8RxFrame[22];
			Convert32Bits.au8Data[1] = au8RxFrame[23];
			Convert32Bits.au8Data[2] = au8RxFrame[24];
			Convert32Bits.au8Data[3] = au8RxFrame[25];
			f32RxeCompass = Convert32Bits.f32Value;
			//printf("Compass:%10.6f°",Convert32Bits.f32Value);


			/* set f32CompassCommand to first value at the first received frame */
			if(bIsTheFirstRxFrame == TRUE)
			{
				f32CompassCommand = f32RxeCompass; //unit is degree
				printf("Init of f32CompassCommand %f\n",f32CompassCommand);
				bIsTheFirstRxFrame = FALSE;
			}

     		//printf("\n");
			//send hdlc response:
			// - motor command
			au8TxFrame[0] = u8MotorCommand;

			// -send PWM command from 0 to 2500
			Convert16Bits.u16Value = u16PWMLevel;
			au8TxFrame[1] = Convert16Bits.au8Data[0];
			au8TxFrame[2] = Convert16Bits.au8Data[1];

			// -compass command in degree
			Convert32Bits.f32Value = f32CompassCommand;
			au8TxFrame[3] = Convert32Bits.au8Data[0];
			au8TxFrame[4] = Convert32Bits.au8Data[1];
			au8TxFrame[5] = Convert32Bits.au8Data[2];
			au8TxFrame[6] = Convert32Bits.au8Data[3];

			u8TxSize = 7;

	        //send tx frame
			bTxResult = HDLC_bPutFrame(&au8TxFrame[0], &u8TxSize);
			if(bTxResult!=TRUE)
			{
				printf("Error to put a frame\n");
			}
		}
	}
    return NULL;
}

int main (int argc, char *argv[])
{
    int err;

    /* check number of arguments
     *  - argv[0] = path
     *  - argv[1] = /dev/refcomm<x> */
    if(argc != 2)
	{
		printf("bad argument, add device path");
		return(EXIT_FAILURE);
	}

    /** create thread in charge to communicate with FRDM_KV31 */
    err = pthread_create(&tThreadCOMId, NULL, &ThreadCOM, argv[1]);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
    else
        printf("\n ThreadCOM created successfully\n");

    /** create thread in charge to communicate with key board */
    err = pthread_create(&tThreadKeyBoardId, NULL, &ThreadKeyBoard, NULL);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
    else
        printf("\n ThreadKeyBoard created successfully\n");

    /** create thread in charge to communicate with joystick */
    err = pthread_create(&tThreadJoystickdId, NULL, &ThreadJoystick, NULL);
    if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
    else
        printf("\n ThreadJoystick created successfully\n");

    while(1)
    {
    	sleep(100);
    }
	return EXIT_SUCCESS;
}
