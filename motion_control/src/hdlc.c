#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <mqueue.h>
#include <pthread.h>

#include "stdtype.h"
#include "hdlc.h"

//used to check periodicity of HDLC frame from motion control board
//put in comment to remove log
//#define MONITOR_PERIODICITY

#define ucPathOfTxHDLC "/MsgQueueTxHDLC"
#define ucPathOfRxHDLC "/MsgQueueRxHDLC"

/* 2 * nb bytes of the application frame (if all data = 0xDLE) +
       1 byte for 0xDLE +
       1 byte for 0xSTX +
       1 byte for 0xDLE +
       1 byte for 0xETX +
       1 byte for  BCC 
    => 5 bytes for hdlc */
#define HDLC_U8_MAX_NB_BYTE_IN_HDLC_FRAME (HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME*2+5)

/** list of specific HDLC characters */
#define U8_DLE         (UI08)0x10U /**<@brief escape character */
#define U8_STX         (UI08)0x02U /**<@brief start frame character */
#define U8_ETX         (UI08)0x03U /**<@brief end frame character */

/** list of state of the finite state machine in charge to decode HDLC */
#define U8_STATE_WAIT_FIRST_DLE  (UI08)0U
#define U8_STATE_WAIT_STX        (UI08)1U
#define U8_STATE_WAIT_FRAME      (UI08)2U
#define U8_STATE_ALGO_DLE        (UI08)3U
#define U8_STATE_WAIT_CHECKSUM   (UI08)4U
#define U8_NUMBER_OF_STATE       (UI08)5U

/** @brief context of HDLC */
typedef struct
{
    UI08 u8RxState;
    UI08 u8RxSize;
    UI08 au8RxAppData[HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME];
    UI08 u8ChecksumOfFrame;
    UI16 gu16DLLRxFrameErrorCounter;
    UI16 gu16DLLRxFrameOKCounter;

    /* @list of function pointers used to control hardware device */
    BOOL (*bOpenDevice) (CHAR pcDeviceName[]);
    UI32 (*s32GetRxData) (UI08 *pu8RxBuffer, UI32 u32Size);
    UI32 (*s32PutTxData) (UI08* pu8TxBuffer, UI32 u32Size);
    BOOL (*bCloseDevice) (void);
}HDLC_stContext;

static HDLC_stContext HDLC_gstContext;

/* list of msg queue */
static mqd_t stMsgQueueTxHDLC = 0;
static mqd_t stMsgQueueRxHDLC = 0;

static pthread_t tThreadRxHDLC;
static pthread_t tThreadTxHDLC;
static pthread_attr_t AttributsOfTxHDLC;
static pthread_attr_t AttributsOfRxHDLC;
static struct sched_param ParametersOfTxHDLC;
static struct sched_param ParametersOfRxHDLC;

static void UpdateDLLRxFrameErrorCounter(UI08 u8LastCaracter,UI08 u8LastState)
{
    if(HDLC_gstContext.gu16DLLRxFrameErrorCounter < 0xFFFF)
    {
        HDLC_gstContext.gu16DLLRxFrameErrorCounter++;
    }
    printf("Error RX HDLC frame ErrorCounter:%d char:%2.2x state:%2.2x\n",HDLC_gstContext.gu16DLLRxFrameErrorCounter,u8LastCaracter,u8LastState);
}

static void UpdateDLLRxFrameOKCounter(void)
{
    
    if(HDLC_gstContext.gu16DLLRxFrameOKCounter < 0xFFFF)
    {
        HDLC_gstContext.gu16DLLRxFrameOKCounter++;
    }
}

#if defined(MONITOR_PERIODICITY)
static struct timespec stCurrentTime;
static struct timespec stPreviousTime;
static UI64 gu64DeltaTimeInns;
// s ramp up 0 to max of 64 bits or 32 bits it depends on tha CPU arch
// nano second ramp up from 0 to 999999999 ns
static inline int64_t calcdiff_ns(struct timespec stCurrent, struct timespec stPrevious)
{
    #define NSEC_PER_SEC 1000000000

    int64_t diff = 0;
    if(stCurrent.tv_sec == stPrevious.tv_sec)
    {
        diff = 0;
    }
    else
    {
        diff = NSEC_PER_SEC * (int64_t)((int) stCurrent.tv_sec - (int) stPrevious.tv_sec);
    }
    diff += ((int) stCurrent.tv_nsec - (int) stPrevious.tv_nsec);
    return diff;
}
#endif /* MONITOR_PERIODICITY */


static void HDLC_StateWaitFirstDLE(UI08 pu8Char)
{
    if (pu8Char == U8_DLE)
    {

        HDLC_gstContext.u8RxState = U8_STATE_WAIT_STX;
    }
    else
    {
        UpdateDLLRxFrameErrorCounter(pu8Char,U8_STATE_WAIT_FIRST_DLE);
    }
}

static void HDLC_StateWaitSTX(UI08 pu8Char)
{
    if (pu8Char == U8_STX)
    {
        HDLC_gstContext.u8RxSize = 0;
        HDLC_gstContext.u8ChecksumOfFrame = 0;
        HDLC_gstContext.u8RxState = U8_STATE_WAIT_FRAME;
    }
    else
    {
        HDLC_gstContext.u8RxState = U8_STATE_WAIT_FIRST_DLE;
        UpdateDLLRxFrameErrorCounter(pu8Char,U8_STATE_WAIT_STX);
    }
}

static void HDLC_StateWaitFrame(UI08 pu8Char)
{
    if (pu8Char == U8_DLE)
    {
        HDLC_gstContext.u8RxState = U8_STATE_ALGO_DLE;
    }
    else
    {
        if ( HDLC_gstContext.u8RxSize < HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME )
        {
            HDLC_gstContext.au8RxAppData[HDLC_gstContext.u8RxSize] = pu8Char;
            HDLC_gstContext.u8ChecksumOfFrame ^= pu8Char;
            HDLC_gstContext.u8RxSize = HDLC_gstContext.u8RxSize + 1;
        }
        else
        {
            UpdateDLLRxFrameErrorCounter(pu8Char,U8_STATE_WAIT_FRAME);
            HDLC_gstContext.u8RxState = U8_STATE_WAIT_FIRST_DLE;
        }
    }
}

static void HDLC_StateAlgoDLE(UI08 pu8Char)
{
    switch(pu8Char)
    {
        case U8_DLE:
            if ( HDLC_gstContext.u8RxSize < HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME )
            {
                HDLC_gstContext.au8RxAppData[HDLC_gstContext.u8RxSize] = pu8Char;
                HDLC_gstContext.u8ChecksumOfFrame ^= pu8Char;
                HDLC_gstContext.u8RxSize =  HDLC_gstContext.u8RxSize + 1;
                HDLC_gstContext.u8RxState = U8_STATE_WAIT_FRAME;
            }
            else
            {
                UpdateDLLRxFrameErrorCounter(pu8Char,U8_STATE_ALGO_DLE);
                HDLC_gstContext.u8RxState = U8_STATE_WAIT_FIRST_DLE;
            }
            break;

        case U8_ETX:
            HDLC_gstContext.u8RxState = U8_STATE_WAIT_CHECKSUM;
            break;

        case U8_STX:
            HDLC_gstContext.u8RxSize = 0;
            HDLC_gstContext.u8ChecksumOfFrame = 0;
            HDLC_gstContext.u8RxState = U8_STATE_WAIT_FRAME;
            break;
        default:
            UpdateDLLRxFrameErrorCounter(pu8Char,U8_STATE_ALGO_DLE);
            HDLC_gstContext.u8RxState = U8_STATE_WAIT_FIRST_DLE;
            break;
    }
}

static void HDLC_StateWaitChecksum(UI08 pu8Char)
{
    if( HDLC_gstContext.u8RxSize & 0x01 )
    {
        HDLC_gstContext.u8ChecksumOfFrame ^= 0x4A;
    }
    else
    {
    }

    /* check checksum */
    if (pu8Char == HDLC_gstContext.u8ChecksumOfFrame )
    {
        SI32 ls32Return;
        
#if defined(MONITOR_PERIODICITY)
        {
            //get time from real time clock
            clock_gettime(CLOCK_REALTIME, &stCurrentTime);
            gu64DeltaTimeInns = calcdiff_ns(stCurrentTime,stPreviousTime);
            printf("gu64DeltaTimeInns %lld in ns \n",gu64DeltaTimeInns);           
            stPreviousTime = stCurrentTime;
    
        }
#endif        
        /* saved Rx frame */
        ls32Return = mq_send(stMsgQueueRxHDLC,(CHAR*)HDLC_gstContext.au8RxAppData,HDLC_gstContext.u8RxSize,0);
        if(ls32Return!=0)
        {
            printf("HDLC mq_send:%s\n",strerror(errno));
        }
        UpdateDLLRxFrameOKCounter();
    }
    else
    {
        UpdateDLLRxFrameErrorCounter(pu8Char,U8_STATE_WAIT_CHECKSUM);
    }
    HDLC_gstContext.u8RxState = U8_STATE_WAIT_FIRST_DLE;
}

static void* TaskRxHDLCFrame(void *arg)
{
    while(1)
    {
        UI08 au8Char[HDLC_U8_MAX_NB_BYTE_IN_HDLC_FRAME];

        UI32 u32Index = 0;

        /* get all caracters */
        SI32 s32NbBytesRead = HDLC_gstContext.s32GetRxData(au8Char,HDLC_U8_MAX_NB_BYTE_IN_HDLC_FRAME);
        
        //printf("TaskRxHDLCFrame s32NbBytesRead:%ld - ",s32NbBytesRead);       
        
        /* analyze all bytes received */
        while(s32NbBytesRead > 0)
        {
            //printf(" %2.2x -",au8Char[u32Index]);
            switch(HDLC_gstContext.u8RxState)
            {
                case U8_STATE_WAIT_FIRST_DLE:
                    HDLC_StateWaitFirstDLE(au8Char[u32Index]);
                    break;
                case U8_STATE_WAIT_STX:
                    HDLC_StateWaitSTX(au8Char[u32Index]);
                    break;

                case U8_STATE_WAIT_FRAME:
                    HDLC_StateWaitFrame(au8Char[u32Index]);
                    break;

                case U8_STATE_ALGO_DLE:
                    HDLC_StateAlgoDLE(au8Char[u32Index]);
                    break;

                case U8_STATE_WAIT_CHECKSUM:
                    HDLC_StateWaitChecksum(au8Char[u32Index]);
                    break;

                default:
                    break;
            }
            u32Index++;
            s32NbBytesRead--;
        }
        //printf("\n");
    }
}

static void* TaskTxHDLCFrame(void *arg)
{
    while(1)
    {
        CHAR acBuffer[HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME];
        ssize_t len_recv;

        /* wait frame to send */
        len_recv = mq_receive(stMsgQueueTxHDLC, acBuffer, HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME, NULL);

        if(len_recv <= HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME)
        {
            int i;
            UI08 u8Checksum = 0;
            UI08 u8Size = len_recv;
            UI08* pu8TxFrame = (UI08*)acBuffer;
            UI08 u8HDLCFrameSize = 0; 

            UI08 u8TxSize;
            UI08 au8TxHDLCData[HDLC_U8_MAX_NB_BYTE_IN_HDLC_FRAME];

            au8TxHDLCData[u8HDLCFrameSize] = U8_DLE;
            u8HDLCFrameSize++;
            au8TxHDLCData[u8HDLCFrameSize] = U8_STX;
            u8HDLCFrameSize++;
            while( u8Size > 0 )
            {
                if ( *pu8TxFrame == U8_DLE )
                {
                    au8TxHDLCData[u8HDLCFrameSize] = U8_DLE;
                    u8HDLCFrameSize++;
                }
                au8TxHDLCData[u8HDLCFrameSize] = *pu8TxFrame;
                u8HDLCFrameSize++;
                
                u8Checksum ^= *pu8TxFrame;
                pu8TxFrame = pu8TxFrame + 1U;

                u8Size = u8Size - 1U;
            }
            if( (len_recv & 0x01) == 0x01)
            {
                u8Checksum ^= 0x4A;
            }
            au8TxHDLCData[u8HDLCFrameSize] = U8_DLE;
            u8HDLCFrameSize++;
            au8TxHDLCData[u8HDLCFrameSize] = U8_ETX;
            u8HDLCFrameSize++;
            au8TxHDLCData[u8HDLCFrameSize] = u8Checksum;
            u8HDLCFrameSize++;
            
            /* write to the device */
            HDLC_gstContext.s32PutTxData(&au8TxHDLCData[0],u8HDLCFrameSize);
        }
        else
        {
            //TODO
        }
    }
}

BOOL HDLC_bInitialize(CHAR pcDeviceName[],
                      BOOL (*bOpenDevice)(CHAR pcDeviceName[]),
                      UI32 (*u32GetRxData) (UI08 *pu8RxBuffer, UI32 u32Size),
                      UI32 (*u32PutTxData) (UI08* pu8TxBuffer, UI32 u32Size),
                      BOOL (*bCloseDevice)(void))
{
    int err;
    BOOL lbOpenDeviceResult;

    printf("HDLC initialize\n");

    if(bOpenDevice!=0)
        HDLC_gstContext.bOpenDevice = bOpenDevice;

    if(u32GetRxData!=0)
        HDLC_gstContext.s32GetRxData = u32GetRxData;

    if(u32PutTxData!=0)
        HDLC_gstContext.s32PutTxData = u32PutTxData;

    if(bCloseDevice!=0)
        HDLC_gstContext.bCloseDevice = bCloseDevice;


    printf("HDLC open device:%s\n",pcDeviceName);
    
    /* open device */
    lbOpenDeviceResult = HDLC_gstContext.bOpenDevice(pcDeviceName);
    printf("Open device result:%d\n",lbOpenDeviceResult);
    
    printf("create Rx and Tx msg queue for HDLC frame\n");
    {
        struct mq_attr    attr;

        /* open the message queue required to send HDLC frame */
        attr.mq_flags = 0;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME;
        attr.mq_curmsgs = 0;

        stMsgQueueTxHDLC = mq_open(ucPathOfTxHDLC, O_RDWR | O_CREAT, 0644, &attr);
        if ((int) stMsgQueueTxHDLC == -1)
        {
            perror("HDLC error : mq_open failed !\n");
            return(-1);
        }

        stMsgQueueRxHDLC = mq_open(ucPathOfRxHDLC, O_RDWR | O_CREAT, 0644, &attr);
        if ((int) stMsgQueueRxHDLC == -1)
        {
            perror("HDLC error : mq_open failed !\n");
            return(-1);
        }
    }
    
    printf("create two threads in charge to send and receive the HDLC frames\n");
    {
        /* set default value for all attributs */
        pthread_attr_init(&AttributsOfTxHDLC);

        /* set priority to -55 of RT thread TX and RX */
        ParametersOfTxHDLC.sched_priority = 54;

        err = pthread_attr_setschedpolicy(&AttributsOfTxHDLC, SCHED_FIFO);
        if(err!=0)
        {
            printf("\ncan't pthread_attr_setschedpolicy :[%s]", strerror(err));
        }

        err = pthread_attr_setinheritsched(&AttributsOfTxHDLC, PTHREAD_EXPLICIT_SCHED);
        if(err!=0)
        {
            printf("\ncan't pthread_attr_setinheritsched:[%s]", strerror(err));
        }

        err = pthread_attr_setschedparam(&AttributsOfTxHDLC, &ParametersOfTxHDLC);
        if(err!=0)
        {
            printf("\ncan't pthread_attr_setschedparam :[%s]", strerror(err));
        }

        /** create thread in charge to transmit HDLC frame */
        err = pthread_create(&tThreadTxHDLC, &AttributsOfTxHDLC, &TaskTxHDLCFrame, NULL);
        if (err != 0)
        {
            printf("\ncan't create thread :[%s]", strerror(err));
        }

        err = pthread_setname_np(tThreadTxHDLC, "HDLC_TX");
        if (err != 0)
        {
            printf("\ncan't name thread :[%s]", strerror(err));
        }
    }
    
    {
        /* set default value for all attributs */
        pthread_attr_init(&AttributsOfRxHDLC);

        /* set priority to -55 of RT thread TX and RX */
        ParametersOfRxHDLC.sched_priority = 54;

        err = pthread_attr_setschedpolicy(&AttributsOfRxHDLC, SCHED_FIFO);
        if(err!=0)
        {
            printf("\ncan't pthread_attr_setschedpolicy :[%s]", strerror(err));
        }

        err = pthread_attr_setinheritsched(&AttributsOfRxHDLC, PTHREAD_EXPLICIT_SCHED);
        if(err!=0)
        {
            printf("\ncan't pthread_attr_setinheritsched:[%s]", strerror(err));
        }

        err = pthread_attr_setschedparam(&AttributsOfRxHDLC, &ParametersOfRxHDLC);
        if(err!=0)
        {
            printf("\ncan't pthread_attr_setschedparam :[%s]", strerror(err));
        }

        /** create thread in charge to receive HDLC frame */
        err = pthread_create(&tThreadRxHDLC, &AttributsOfRxHDLC, &TaskRxHDLCFrame, NULL);
        if (err != 0)
        {
            printf("\ncan't create thread :[%s]", strerror(err));
        }

        err = pthread_setname_np(tThreadRxHDLC, "HDLC_RX");
        if (err != 0)
        {
            printf("\ncan't name thread :[%s]", strerror(err));
        }
    }
    return(lbOpenDeviceResult);
}

BOOL HDLC_bGetFrame(UI08 *pu8RxFrame, UI08 *pu8RxSize)
{
    static mqd_t stMsgQueueGetFrame = 0;

    size_t ls32Return;
    /* msg queue is initialized ? */
    if(stMsgQueueGetFrame == 0)
    {
        printf("mq_open of %s\n",ucPathOfTxHDLC);
        /* open msg queue of TX HDLC */
        stMsgQueueGetFrame = mq_open(ucPathOfRxHDLC, O_RDONLY, 0, NULL);
        if ((int) stMsgQueueRxHDLC == -1)
        {
            perror("HDLC error : mq_open failed !\n");
            return(FALSE);
        }
    }
    else
    {
        /* do nothing */
    }
    // receive the HDLC frame
    ls32Return = mq_receive(stMsgQueueRxHDLC,(CHAR*)pu8RxFrame,(size_t)*pu8RxSize,0);
    if(
       (ls32Return==0) OR 
       (ls32Return > HDLC_U8_MAX_NB_BYTE_IN_APP_FRAME ))
    {
        *pu8RxSize=0;
        return(FALSE);
    }
    *pu8RxSize = (UI08)(ls32Return&0xFF);
    return( TRUE );
}

BOOL HDLC_bPutFrame(UI08 *pu8TxFrame, UI08 *pu8TxSize)
{
    static mqd_t stMsgQueuePutFrame = 0;
    SI32 ls32Return;
    /* msg queue is initialized ? */
    if(stMsgQueuePutFrame == 0)
    {
        printf("mq_open of %s\n",ucPathOfTxHDLC);
        /* open msg queue of TX HDLC */
        stMsgQueuePutFrame = mq_open(ucPathOfTxHDLC, O_WRONLY, 0, NULL);
        if ((int) stMsgQueuePutFrame == -1)
        {
            perror("HDLC error : mq_open failed !\n");
            return(FALSE);
        }
    }
    else
    {
        /* do nothing */
    }
    // send the HDLC frame
    ls32Return = mq_send(stMsgQueuePutFrame,(CHAR*)pu8TxFrame,(size_t)*pu8TxSize,0);
    if(ls32Return!=0)
    {
        printf("HDLC mq_send:%s\n",strerror(errno));
        return(FALSE);
    }
    return( TRUE );
}

void HDLC_Close(void)
{
    /* close device if open */
    HDLC_gstContext.bCloseDevice();
    
    /* close msq queue */
    if(stMsgQueueTxHDLC!=0) mq_close(stMsgQueueTxHDLC);
    if(stMsgQueueRxHDLC!=0) mq_close(stMsgQueueRxHDLC);
}
