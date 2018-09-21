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
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include <sys/fcntl.h>
#include <sys/stat.h>

 #include <sched.h>

/* project include */
#include "stdtype.h"
#include "uart.h"
#include "hdlc.h"

//warning printf reduce reactivity of the comunication
//#define DEBUG_RX_APP_FRAME
//#define DEBUG_TX_APP_FRAME
//#define DEBUG_MOTION_CMD

#define FIFO_FILE  "/tmp/MotionControl.fifo" //FIFO used to exchange motion command

typedef struct
{
    UI08  u8MotorCommand;  //from 0 to 3
    SI16  u16PWMLevel;     //from 0 to 2499
    FL32  f32DeltaCompass; //from 0 to 359.9 in degree
}stMotionCommand;


/* public variables */
UI08 u8MotorCommand = 0;
UI16 u16PWMLevel = 0;
FL32 f32CompassCommand = 0;
FL32 f32RxeCompass = 0;

/* private variables */
static UI08 au8RxFrame[HDLC_U8_MAX_NB_BYTE_IN_FRAME];
static UI08 u8RxSize;
static UI08 au8TxFrame[HDLC_U8_MAX_NB_BYTE_IN_FRAME];
static UI08 u8TxSize;

static pthread_t tThreadRxMotionControl;
static pthread_t tThreadTxMotionControl;

static BOOL bInitHDLCIsOK;

static float CalculateNewCap(float f32Cap,float f32DeltaCap)
{
    f32Cap = f32Cap+f32DeltaCap;
    if(f32Cap >= 360.0) f32Cap = f32Cap-360.0;
    else if(f32Cap < 0) f32Cap = f32Cap+360.0;
    return(f32Cap);
}

/* thread in charge to manage HDCL Rx frames with FRDM_KV31 board */
void* ThreadCOMRx(void *arg)
{

    BOOL bTxfirstResult;

    union
    {
        FL32 f32Value;
        UI32  u32Value;
        UI08  au8Data[4];
    }Convert32Bits;

    printf("arg:%s ",(char *)arg);

    /* try to open HDLC link on "/dev/rfcomm*" */
    bInitHDLCIsOK = HDLC_bInitialize(arg,
                                     UART_bOpenDevice,
                                     UART_bGetRxChar,
                                     UART_bPutTxChar,
                                     UART_bCloseDevice);

    /** if not open wait device */
    while(  bInitHDLCIsOK == FALSE )
    {
        printf( "Couldn't open rfcomm device\n" );
        sleep(1);/* wait 1 s*/
        /* try to open HDLC link on "/dev/rfcomm*" */
        bInitHDLCIsOK = HDLC_bInitialize(arg,
                                         UART_bOpenDevice,
                                         UART_bGetRxChar,
                                         UART_bPutTxChar,
                                         UART_bCloseDevice);
    }
    au8TxFrame[0] = 's';
    u8TxSize = 1;

    //send tx frame
    printf( "send first tx frame to start com with FRDM-KV31,%x\n", bInitHDLCIsOK);
    bTxfirstResult = HDLC_bPutFrame(&au8TxFrame[0], &u8TxSize);
    if(bTxfirstResult!=TRUE)
    {
        printf("Error to put a frame\n");
    }

    /* hdlc communication loop */
    while ( bInitHDLCIsOK==TRUE )
    {
        BOOL bRxResult;

        //wait read a frame
        bRxResult = HDLC_bGetFrame(au8RxFrame, &u8RxSize);
        if((bRxResult == TRUE) && (u8RxSize == 26))
        {
            //analyze the data of the frame
#ifdef DEBUG_RX_APP_FRAME
            //print all data of the hdlc frame received
            printf("Frame received:");

            /*update life byte*/
            printf("Tx Life byte:%2.2x ",au8RxFrame[0]);
            printf("Rx Life byte:%2.2x ",au8RxFrame[1]);
#endif
            /* update linear acceleration (g) X */
            Convert32Bits.au8Data[0] = au8RxFrame[2];
            Convert32Bits.au8Data[1] = au8RxFrame[3];
            Convert32Bits.au8Data[2] = au8RxFrame[4];
            Convert32Bits.au8Data[3] = au8RxFrame[5];
#ifdef DEBUG_RX_APP_FRAME
            printf("Gx:%10.6f ",Convert32Bits.f32Value);
#endif

            /* update linear acceleration (g) Y */
            Convert32Bits.au8Data[0] = au8RxFrame[6];
            Convert32Bits.au8Data[1] = au8RxFrame[7];
            Convert32Bits.au8Data[2] = au8RxFrame[8];
            Convert32Bits.au8Data[3] = au8RxFrame[9];
#ifdef DEBUG_RX_APP_FRAME
            printf("Gy:%10.6f ",Convert32Bits.f32Value);
#endif

            /* update linear acceleration (g) Z */
            Convert32Bits.au8Data[0] = au8RxFrame[10];
            Convert32Bits.au8Data[1] = au8RxFrame[11];
            Convert32Bits.au8Data[2] = au8RxFrame[12];
            Convert32Bits.au8Data[3] = au8RxFrame[13];
#ifdef DEBUG_RX_APP_FRAME
            printf("Gz:%10.6f ",Convert32Bits.f32Value);
#endif

            /* update roll (deg) */
            Convert32Bits.au8Data[0] = au8RxFrame[14];
            Convert32Bits.au8Data[1] = au8RxFrame[15];
            Convert32Bits.au8Data[2] = au8RxFrame[16];
            Convert32Bits.au8Data[3] = au8RxFrame[17];
#ifdef DEBUG_RX_APP_FRAME
            printf("Roll:%10.6f° ",Convert32Bits.f32Value);
#endif

            /* update pitch (deg) */
            Convert32Bits.au8Data[0] = au8RxFrame[18];
            Convert32Bits.au8Data[1] = au8RxFrame[19];
            Convert32Bits.au8Data[2] = au8RxFrame[20];
            Convert32Bits.au8Data[3] = au8RxFrame[21];
#ifdef DEBUG_RX_APP_FRAME
            printf("Pitch:%10.6f° ",Convert32Bits.f32Value);
#endif

            /* update compass (deg) */
            Convert32Bits.au8Data[0] = au8RxFrame[22];
            Convert32Bits.au8Data[1] = au8RxFrame[23];
            Convert32Bits.au8Data[2] = au8RxFrame[24];
            Convert32Bits.au8Data[3] = au8RxFrame[25];
            f32RxeCompass = Convert32Bits.f32Value;
#ifdef DEBUG_RX_APP_FRAME
            printf("RxeCompass:%10.6f°\n",Convert32Bits.f32Value);
#endif
        }
    }
    return NULL;
}

/* thread in charge to manage HDCL Tx frames with FRDM_KV31 board */
void* ThreadCOMTx(void *arg)
{

    FILE* pFileFIFIO;
    stMotionCommand lstMotionCmd = {0,0,0.0};

    union
    {
        FL32 f32Value;
        UI32  u32Value;
        UI08  au8Data[4];
    }Convert32Bits;

    union
    {
        int16_t  s16Value;
        UI16  u16Value;
        UI08  au8Data[2];
    }Convert16Bits;

    /** if not open wait device */
    while(  bInitHDLCIsOK == FALSE )
    {
        printf( "Wait initialization is done\n" );
        sleep(1);/* wait 1 s*/
    }

    /* hdlc communication loop */
    while ( bInitHDLCIsOK==TRUE )
    {
        BOOL bTxResult;

        /* open FIFO read only */
        pFileFIFIO = fopen(FIFO_FILE, "r");

        /* read if new command is received */
        fread((void*)&lstMotionCmd, sizeof(stMotionCommand),1, pFileFIFIO);

 #ifdef DEBUG_MOTION_CMD
        printf("motion_cmd - Received cmd - u8MotorCommand:%d, u16PWMLevel:%d, f32DeltaCompass:%f\n",
               lstMotionCmd.u8MotorCommand,
               lstMotionCmd.u16PWMLevel,
               lstMotionCmd.f32DeltaCompass);
 #endif

        fclose(pFileFIFIO);

        /* update motion command sent to FRDM KV31 */
        u8MotorCommand    = lstMotionCmd.u8MotorCommand;
        u16PWMLevel       = lstMotionCmd.u16PWMLevel;
        f32CompassCommand = CalculateNewCap(f32RxeCompass,lstMotionCmd.f32DeltaCompass);

#ifdef DEBUG_TX_APP_FRAME
        //print all data of the hdlc frame transmit
        printf("Frame transmit:");
#endif
        //send hdlc response:
        // - motor command
        au8TxFrame[0] = u8MotorCommand;
#ifdef DEBUG_TX_APP_FRAME
        printf("motor cmd:%2.2x ",u8MotorCommand);
#endif
        // -send PWM command from 0 to 2500
        Convert16Bits.u16Value = u16PWMLevel;
        au8TxFrame[1] = Convert16Bits.au8Data[0];
        au8TxFrame[2] = Convert16Bits.au8Data[1];
#ifdef DEBUG_TX_APP_FRAME
        printf("PWM cmd:%3.3x ",u16PWMLevel);
#endif

        // -compass command in degree
        Convert32Bits.f32Value = f32CompassCommand;
        au8TxFrame[3] = Convert32Bits.au8Data[0];
        au8TxFrame[4] = Convert32Bits.au8Data[1];
        au8TxFrame[5] = Convert32Bits.au8Data[2];
        au8TxFrame[6] = Convert32Bits.au8Data[3];
#ifdef DEBUG_TX_APP_FRAME
        printf("f32RxeCompass:%f f32CompassCommand:%f \n",f32RxeCompass,f32CompassCommand);
#endif
        u8TxSize = 7;

        //send tx frame
        bTxResult = HDLC_bPutFrame(&au8TxFrame[0], &u8TxSize);
        if(bTxResult!=TRUE)
        {
            printf("Error to put a frame\n");
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int err;

    /* check number of arguments
     *  - argv[0] = path
     *  - argv[1] = /dev/refcomm<x> */
    if(argc != 2)
    {
        printf("bad argument, add device path\n");
        return(EXIT_FAILURE);
    }

    /** create thread in charge to transmit motion command received from gamepad or keyboard */
    err = pthread_create(&tThreadTxMotionControl, NULL, &ThreadCOMTx, NULL);
    if (err != 0)
    {
        printf("\ncan't create thread :[%s]", strerror(err));
    }
    else
    {
        printf("\n ThreadMotionControl created successfully\n");
    }

    /** create thread in charge to receive motion with FRDM_KV31 */
    err = pthread_create(&tThreadRxMotionControl, NULL, &ThreadCOMRx, argv[1]);
    if (err != 0)
    {
      printf("\ncan't create thread :[%s]", strerror(err));
    }
    else
    {
        printf("\n ThreadCOM created successfully\n");
    }

    while(1)
    {
        sleep(100);
    }
    return EXIT_SUCCESS;
}

