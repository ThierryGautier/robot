/*
 ============================================================================
 Name        : motion_control.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : communication with FRDM KV31 board through a bluetooth connection
               lunix command:  'sudo ./bluetooth /dev/rfcomm0'
               make sure that the /dev/rfcomm0 is enabled before using it
 ============================================================================
 */

/* system include */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <sched.h>
#include <mqueue.h>
#include <pthread.h>
#include <sys/mman.h>


/* project include */
#include "stdtype.h"
#include "supervisor.h"
#include "uart.h"
#include "hdlc.h"
#include "motion_control.h"

//warning printf reduce reactivity of the comunication
//#define DEBUG_RX_APP_FRAME
//#define DEBUG_TX_APP_FRAME
#define DEBUG_MOTION_CMD
//#define RT_TEST

/* public variables */
UI08 u8MotorCommand = 0;
UI16 u16PWMLevel = 0;
FL32 f32CompassCommand = 0;
FL32 f32RxeCompass = 0;

/* private variables */
static UI08 au8RxFrame[HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME];
static UI08 u8RxSize;
static UI08 au8TxFrame[HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME];
static UI08 u8TxSize;


static BOOL bInitHDLCIsOK;

static mqd_t stMsgQueueMotionCtrl = 0;
static mqd_t stMsgQueueSPR = 0;

FL32 f32AverageOfGx;
FL32 f32AverageOfGy;
FL32 f32AverageOfGz;

FL32 f32SumOfGx;
FL32 f32SumOfGy;
FL32 f32SumOfGz;

FL32 f32ArrayOfGx[128];
FL32 f32ArrayOfGy[128];
FL32 f32ArrayOfGz[128];
UI16 gu16IndexIn = 0;
UI16 gu16IndexOut = 1;
UI16 gu16NbMesuresInit = 0;

/** get time in nano second
 */
static UI64 u64GetTimeInns(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return((UI64)spec.tv_nsec);
}

static FL32 f32CalculateNewCap(float f32Cap,float f32DeltaCap)
{
    f32Cap = f32Cap+f32DeltaCap;
    if(f32Cap >= 360.0) f32Cap = f32Cap-360.0;
    else if(f32Cap < 0) f32Cap = f32Cap+360.0;
    return(f32Cap);
}

/*
 * function used to send motion event
 */
static void SendMotionEventThroughMsgQueue(mqd_t stMsgQueue, SPR_stMotionEvent stMotionEvent)
{
   SPR_ProcessEvent lstMsgMotionEvent;
   SI32 ls32Return;

   /* set the process id*/
   lstMsgMotionEvent.stEvent.eProcessId = SPR_eProcessIdMotionControl;
   
   /* update the motion event */
   lstMsgMotionEvent.stEvent.List.stMotionEvent = stMotionEvent;

   ls32Return = mq_send(stMsgQueue, lstMsgMotionEvent.acBuffer, sizeof(SPR_ProcessEvent), SPR_PRIORITY_MOTION_EVT);
   if(ls32Return!=0)
   {
       printf("motion_control mq_send:%s",strerror(errno));
   }
}

/* thread in charge to manage HDCL Rx frames with FRDM_KV31 board */
void* ThreadCOMRx(void *arg)
{
    UI32 u32AvrgDeltaTimeBetweenPrevRqstRspInus = 0;
    UI64 u64SumDeltaTimeBetweenPrevRqstRspInus = 0;
    UI32 u32NbPoints = 0;

    BOOL bTxfirstResult;
    UI08 u8FrameCounter=0;
    union
    {
        FL32 f32Value;
        UI32  u32Value;
        UI08  au8Data[4];
    }Convert32Bits;

    /* create applicative frame to start FRDM-KV31 board hdlc protocol*/
#ifdef RT_TEST
    au8TxFrame[0] = 't'; /*rt test */
#else
    au8TxFrame[0] = 's';
#endif
    u8TxSize = 1;

    //send tx frame
    bTxfirstResult = HDLC_bPutFrame(&au8TxFrame[0], &u8TxSize);
    if(bTxfirstResult!=TRUE)
    {
        printf("Error to put a frame\n");
    }

    /* hdlc communication loop */
    while ( bInitHDLCIsOK==TRUE )
    {
        BOOL bRxResult;
        SPR_stMotionEvent stMotionEvent;
        UI32 u32TaskComPeriod;

        //wait read a frame
        u8RxSize = sizeof(au8RxFrame);
        bRxResult = HDLC_bGetFrame(au8RxFrame, &u8RxSize);

        ///check frame, must match with FRDM KV31F board
        if((bRxResult == TRUE) && (u8RxSize == 30))
        {
            UI16 i=0;
            //analyze the data of the frame
            stMotionEvent.u8TxLife= au8RxFrame[i++];
            stMotionEvent.u8RxLife= au8RxFrame[i++];

            /* update latency  */
            Convert32Bits.au8Data[0] = au8RxFrame[i++];
            Convert32Bits.au8Data[1] = au8RxFrame[i++];
            Convert32Bits.au8Data[2] = au8RxFrame[i++];
            Convert32Bits.au8Data[3] = au8RxFrame[i++];
            stMotionEvent.u32LatencyInus = Convert32Bits.f32Value;

            /* update linear acceleration (g) X */
            Convert32Bits.au8Data[0] = au8RxFrame[i++];
            Convert32Bits.au8Data[1] = au8RxFrame[i++];
            Convert32Bits.au8Data[2] = au8RxFrame[i++];
            Convert32Bits.au8Data[3] = au8RxFrame[i++];
            stMotionEvent.f32Gx = Convert32Bits.f32Value;

            /* update linear acceleration (g) Y */
            Convert32Bits.au8Data[0] = au8RxFrame[i++];
            Convert32Bits.au8Data[1] = au8RxFrame[i++];
            Convert32Bits.au8Data[2] = au8RxFrame[i++];
            Convert32Bits.au8Data[3] = au8RxFrame[i++];
            stMotionEvent.f32Gy = Convert32Bits.f32Value;

            /* update linear acceleration (g) Z */
            Convert32Bits.au8Data[0] = au8RxFrame[i++];
            Convert32Bits.au8Data[1] = au8RxFrame[i++];
            Convert32Bits.au8Data[2] = au8RxFrame[i++];
            Convert32Bits.au8Data[3] = au8RxFrame[i++];
            stMotionEvent.f32Gz = Convert32Bits.f32Value;

            /* update roll (deg) */
            Convert32Bits.au8Data[0] = au8RxFrame[i++];
            Convert32Bits.au8Data[1] = au8RxFrame[i++];
            Convert32Bits.au8Data[2] = au8RxFrame[i++];
            Convert32Bits.au8Data[3] = au8RxFrame[i++];
            stMotionEvent.f32Roll = Convert32Bits.f32Value;

            /* update pitch (deg) */
            Convert32Bits.au8Data[0] = au8RxFrame[i++];
            Convert32Bits.au8Data[1] = au8RxFrame[i++];
            Convert32Bits.au8Data[2] = au8RxFrame[i++];
            Convert32Bits.au8Data[3] = au8RxFrame[i++];
            stMotionEvent.f32Pitch = Convert32Bits.f32Value;

            /* update compass (deg) */
            Convert32Bits.au8Data[0] = au8RxFrame[i++];
            Convert32Bits.au8Data[1] = au8RxFrame[i++];
            Convert32Bits.au8Data[2] = au8RxFrame[i++];
            Convert32Bits.au8Data[3] = au8RxFrame[i++];
            stMotionEvent.f32Compass = Convert32Bits.f32Value;
            f32RxeCompass = Convert32Bits.f32Value;

            /* send motion event to the supervisior */
            SendMotionEventThroughMsgQueue(stMsgQueueSPR,stMotionEvent);
// 
#ifdef DEBUG_RX_APP_FRAME
            //print all data of the hdlc frame received
            printf("Tx Life byte:%2.2x ",stMotionEvent.u8TxLife);
            printf("Rx Life byte:%2.2x ",stMotionEvent.u8RxLife);
        //    printf("u32TaskComPeriod in us:%6.6ld ",u32TaskComPeriod);
            printf("Gx:%10.6f Avg:%10.6f ",stMotionEvent.f32Gx,f32AverageOfGx);
            printf("Gy:%10.6f Avg:%10.6f ",stMotionEvent.f32Gy,f32AverageOfGy);
            printf("Gz:%10.6f Avg:%10.6f ",stMotionEvent.f32Gz,f32AverageOfGz);
            printf("Roll:%10.6f ",stMotionEvent.f32Roll);
            printf("Pitch:%10.6f ",stMotionEvent.f32Pitch);
            printf("Compass:%10.6f \r",stMotionEvent.f32Compass);
            
            f32ArrayOfGx[gu16IndexIn] = stMotionEvent.f32Gx;
            f32ArrayOfGy[gu16IndexIn] = stMotionEvent.f32Gy;
            f32ArrayOfGz[gu16IndexIn] = stMotionEvent.f32Gz;
            
            /* check if buffer is inialize , 128 value aupadted at the startup*/
            if(gu16NbMesuresInit < 128)
            {
                gu16NbMesuresInit++;
                f32SumOfGx = (f32SumOfGx+f32ArrayOfGx[gu16IndexIn]);
                f32SumOfGy = (f32SumOfGy+f32ArrayOfGy[gu16IndexIn]);
                f32SumOfGz = (f32SumOfGz+f32ArrayOfGz[gu16IndexIn]);
            }
            else
            {
                f32SumOfGx = (f32SumOfGx+f32ArrayOfGx[gu16IndexIn]-f32ArrayOfGx[gu16IndexOut]);
                f32SumOfGy = (f32SumOfGy+f32ArrayOfGy[gu16IndexIn]-f32ArrayOfGy[gu16IndexOut]);
                f32SumOfGz = (f32SumOfGz+f32ArrayOfGz[gu16IndexIn]-f32ArrayOfGz[gu16IndexOut]);
            }
            f32AverageOfGx = f32SumOfGx/(FL32)128;
            f32AverageOfGy = f32SumOfGy/(FL32)128;
            f32AverageOfGz = f32SumOfGz/(FL32)128;
            
            gu16IndexIn++;
            gu16IndexOut++;
            if(gu16IndexIn>=128) gu16IndexIn=0;
            if(gu16IndexOut>=128) gu16IndexOut=0;
            
#endif
            /* check if lost Rx frame , used Tx life counter */
            if(u8FrameCounter!=stMotionEvent.u8TxLife)
            {
                printf("error frame lost u8TxLife:%3.3d u8FrameCounter:%3.3d\n",stMotionEvent.u8TxLife,u8FrameCounter);
                u8FrameCounter = stMotionEvent.u8TxLife;
            }
            u8FrameCounter++;
        }
        else if((bRxResult == TRUE) && (u8RxSize == 14))
        {
            BOOL bTxResult;
            UI08 lu8TxLife;
            UI08 lu8RxLife;
            UI32 u32TaskComPeriod;
            UI32 u32RqstTimeInus;
            UI32 u32DeltaTimeBetweenPrevRqstRspInus;
            
            //analyze the data of the frame
            lu8TxLife= au8RxFrame[0];
            lu8RxLife= au8RxFrame[1];

            /* update FRDM KV31F Task com period */
            Convert32Bits.au8Data[0] = au8RxFrame[2];
            Convert32Bits.au8Data[1] = au8RxFrame[3];
            Convert32Bits.au8Data[2] = au8RxFrame[4];
            Convert32Bits.au8Data[3] = au8RxFrame[5];
            u32TaskComPeriod = Convert32Bits.u32Value;

            /* get the request time in us of FRDM KV31F Task */
            Convert32Bits.au8Data[0] = au8RxFrame[6];
            Convert32Bits.au8Data[1] = au8RxFrame[7];
            Convert32Bits.au8Data[2] = au8RxFrame[8];
            Convert32Bits.au8Data[3] = au8RxFrame[9];
            u32RqstTimeInus = Convert32Bits.u32Value;

            /* get the previous delta time between the previous request and response, must be less than 2 ms */
            Convert32Bits.au8Data[0] = au8RxFrame[10];
            Convert32Bits.au8Data[1] = au8RxFrame[11];
            Convert32Bits.au8Data[2] = au8RxFrame[12];
            Convert32Bits.au8Data[3] = au8RxFrame[13];
            u32DeltaTimeBetweenPrevRqstRspInus = Convert32Bits.u32Value;

            //send hdlc response:
            // - Tx life 
            au8TxFrame[0] = lu8TxLife;

            // - request time in us
            Convert32Bits.u32Value = u32RqstTimeInus;
            au8TxFrame[1] = Convert32Bits.au8Data[0];
            au8TxFrame[2] = Convert32Bits.au8Data[1];
            au8TxFrame[3] = Convert32Bits.au8Data[2];
            au8TxFrame[4] = Convert32Bits.au8Data[3];

            u8TxSize = 5;

            //send tx frame
            bTxResult = HDLC_bPutFrame(&au8TxFrame[0], &u8TxSize);
            if(bTxResult!=TRUE)
            {
                printf("Error to put a frame\n");
            }

            u64SumDeltaTimeBetweenPrevRqstRspInus = u64SumDeltaTimeBetweenPrevRqstRspInus + (UI64)u32DeltaTimeBetweenPrevRqstRspInus;
            u32NbPoints++;
            u32AvrgDeltaTimeBetweenPrevRqstRspInus = (UI32)(u64SumDeltaTimeBetweenPrevRqstRspInus/u32NbPoints); 

#ifdef DEBUG_RX_APP_FRAME
            //print all data of the hdlc frame received
           // printf("Tx Life byte:%2.2x ",lu8TxLife);
            //printf("Rx Life byte:%2.2x ",lu8RxLife);
            //printf("u32TaskComPeriod in us:%6.6ld ",u32TaskComPeriod);
            printf("u32RqstTimeInus:%6.6ld ",u32RqstTimeInus);
            printf("u32DeltaTimeBetweenPrevRqstRspInus:%6.6ld ",u32DeltaTimeBetweenPrevRqstRspInus);
            printf("u32AvrgDeltaTimeBetweenPrevRqstRspInus:%6.6ld \n",u32AvrgDeltaTimeBetweenPrevRqstRspInus);
#endif

        }
        else if(bRxResult == TRUE)
        {
            printf("Unknown frame received: %2.2x",u8RxSize);
        }
    }
    return NULL;
}

/* thread in charge to manage HDCL Tx frames with FRDM_KV31 board */
void* ThreadCOMTx(void *arg)
{
    MCL_stMotionCommand lstMotionCmd = {0,0,0.0};

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

    /* hdlc communication loop */
    while ( bInitHDLCIsOK==TRUE )
    {
        BOOL bTxResult;
        ssize_t len_recv;

#ifdef DEBUG_MOTION_CMD
        printf( "Wait event from supervisor\n" );
#endif
        /* read if new motion command is received */
        len_recv = mq_receive(stMsgQueueMotionCtrl,(CHAR*)&lstMotionCmd, sizeof(lstMotionCmd), NULL);
        if(len_recv == sizeof(lstMotionCmd))
        {

 #ifdef DEBUG_MOTION_CMD
            printf("motion_cmd - Received cmd - u8MotorCommand:%d, u16PWMLevel:%d, f32DeltaCompass:%f\n",
                   lstMotionCmd.u8MotorCommand,
                   lstMotionCmd.u16PWMLevel,
                   lstMotionCmd.f32DeltaCompass);
 #endif

            /* update motion command sent to FRDM KV31 */
            u8MotorCommand    = lstMotionCmd.u8MotorCommand;
            u16PWMLevel       = lstMotionCmd.u16PWMLevel;
            f32CompassCommand = f32CalculateNewCap(f32RxeCompass,lstMotionCmd.f32DeltaCompass);

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
    }
    return NULL;
}

/** code from cyclictest.c */
/* Latency trick
 * if the file /dev/cpu_dma_latency exists,
 * open it and write a zero into it. This will tell 
 * the power management system not to transition to 
 * a high cstate (in fact, the system acts like idle=poll)
 * When the fd to /dev/cpu_dma_latency is closed, the behavior
 * goes back to the system default.
 * 
 * Documentation/power/pm_qos_interface.txt
 */
static int latency_target_fd = -1;
static int32_t gu32latency_target_value_default = 0;
static int32_t gu32latency_target_value = 0;
static void set_latency_target(void)
{
	struct stat s;
	int ret;

	if (stat("/dev/cpu_dma_latency", &s) == 0) {
		latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
		if (latency_target_fd == -1)
			return;
        
		ret = read(latency_target_fd, &gu32latency_target_value_default, 4);
		if (ret == 0) {
			printf("# error reading cpu_dma_latency: %s\n", strerror(errno));
			close(latency_target_fd);
			return;
		}
		ret = write(latency_target_fd, &gu32latency_target_value, 4);
		if (ret == 0) {
			printf("# error setting cpu_dma_latency to %d!: %s\n", gu32latency_target_value, strerror(errno));
			close(latency_target_fd);
			return;
		}
		printf("/dev/cpu_dma_latency default value equal to %dus\n", gu32latency_target_value_default);
		printf("/dev/cpu_dma_latency set to %dus\n", gu32latency_target_value);
	}
}

/* catch Signal to interrupt the motion control process */
static void sig_handler(int signo)
{
    if (signo == SIGTERM)
    {
        /* close HDLC */
        HDLC_Close();
        /* close msq queue */
        if(stMsgQueueMotionCtrl!=0) mq_close(stMsgQueueMotionCtrl);
        if(stMsgQueueSPR!=0) mq_close(stMsgQueueSPR);
        printf("received SIGTERM\n");
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    int err;
    pthread_t tThreadRxMotionControl;
    pthread_t tThreadTxMotionControl;
    pthread_attr_t AttributsOfTxMotionControl;
    pthread_attr_t AttributsOfRxMotionControl;
    struct sched_param ParametersOfTxMotionControl;
    struct sched_param ParametersOfRxMotionControl;

    /***/
    printf("Motion control process\n");
    
    /* check number of arguments
     *  - argv[0] = path
     *  - argv[1] = /dev/refcomm<x> */
    if(argc != 2)
    {
        printf("bad argument, add device path\n");
        return(EXIT_FAILURE);
    }

    /*set latency target to 0 */
    set_latency_target();
    
    /* lock all memory (prevent swapping) */
    if (mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
        perror("mlockall");
        return(EXIT_FAILURE);
    }
 
    /* catch SIGTERM signal */
    if (signal(SIGTERM, sig_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGTERM\n");
    }

    /* try to open HDLC link on "/dev/rfcomm*" or /dev/ttyS* */
    bInitHDLCIsOK = HDLC_bInitialize(argv[1],
                                     UART_bOpenDevice,
                                     UART_s32GetRxData,
                                     UART_s32PutTxData,
                                     UART_bCloseDevice);
 
    if(bInitHDLCIsOK !=TRUE)
    {
      printf("Motion_control error: HDLC init KO !\n");
      return(-1);
    }
    else
    {
        printf("Motion_control: Init HDLC done\n");
    }

    /* open ms queue */
    /* open the message queue created by supervisor process */
    stMsgQueueMotionCtrl = mq_open(SPR_ucPathOfMotionControlMsgQueue, O_RDONLY , 0, NULL);
    if ((int) stMsgQueueMotionCtrl == -1)
    {
      perror("Motion_control error: mq_open failed !\n");
      return(-1);
    }

    /* open the message queue created by supervisor process to send gamepad events */
    stMsgQueueSPR = mq_open(SPR_ucPathOfSupervisorMsgQueue, O_WRONLY |O_NONBLOCK, 0, NULL);
    if ((int) stMsgQueueSPR == -1)
    {
        perror("gamepad error : mq_open failed !\n");
        return(-1);
    }

    /* set default value for all attributs */
    pthread_attr_init(&AttributsOfTxMotionControl);

    /* set priority to -55 of RT thread TX and RX */
    ParametersOfTxMotionControl.sched_priority = 54;

    err = pthread_attr_setschedpolicy(&AttributsOfTxMotionControl, SCHED_FIFO);
    if(err!=0)
    {
        printf("\ncan't pthread_attr_setschedpolicy :[%s]", strerror(err));
    }

    err = pthread_attr_setinheritsched(&AttributsOfTxMotionControl, PTHREAD_EXPLICIT_SCHED);
    if(err!=0)
    {
        printf("\ncan't pthread_attr_setinheritsched:[%s]", strerror(err));
    }

    err = pthread_attr_setschedparam(&AttributsOfTxMotionControl, &ParametersOfTxMotionControl);
    if(err!=0)
    {
        printf("\ncan't pthread_attr_setschedparam :[%s]", strerror(err));
    }

    /** create thread in charge to transmit motion command received from gamepad or keyboard */
    err = pthread_create(&tThreadTxMotionControl, &AttributsOfTxMotionControl, &ThreadCOMTx, NULL);
    if (err != 0)
    {
        printf("\ncan't create thread :[%s]", strerror(err));
    }

    err = pthread_setname_np(tThreadTxMotionControl, "COM_TX");
    if (err != 0)
    {
        printf("\ncan't name thread :[%s]", strerror(err));
    }
   

    /* set default value for all attributs */
    pthread_attr_init(&AttributsOfRxMotionControl);

    /* set priority to -55 of RT thread TX and RX */
    ParametersOfRxMotionControl.sched_priority = 54;

    pthread_attr_setschedpolicy(&AttributsOfRxMotionControl, SCHED_FIFO);
    if(err!=0)
    {
        printf("\ncan't pthread_attr_setschedpolicy :[%s]", strerror(err));
    }

    pthread_attr_setinheritsched(&AttributsOfRxMotionControl, PTHREAD_EXPLICIT_SCHED);
    if(err!=0)
    {
        printf("\ncan't pthread_attr_setinheritsched:[%s]", strerror(err));
    }

    pthread_attr_setschedparam(&AttributsOfRxMotionControl, &ParametersOfRxMotionControl);
    if(err!=0)
    {
        printf("\ncan't pthread_attr_setschedparam :[%s]", strerror(err));
    }

    /** create thread in charge to receive motion with FRDM_KV31 */
    err = pthread_create(&tThreadRxMotionControl, &AttributsOfRxMotionControl, &ThreadCOMRx, NULL);
    if (err != 0)
    {
      printf("\ncan't create thread :[%s]", strerror(err));
    }

    err = pthread_setname_np(tThreadRxMotionControl, "COM_RX");
    if (err != 0)
    {
        printf("\ncan't name thread :[%s]", strerror(err));
    }
    
    while(1)
    {
        sleep(100);
    }
    return EXIT_SUCCESS;
}

