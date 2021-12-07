/*
 * party the code in C used to communicate with supervisor
 * send vision events through mq_queue */
extern "C" {

/* system include */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <mqueue.h>
#include <errno.h>

/* project include */
#include "stdtype.h"
#include "supervisor.h"
#include "com.h"

mqd_t stMsgQueue = 0;

/*
 * function used to send vision event through message queue
 */
void COM_SendVisionEventThroughMsgQueue(SPR_stVisionEvent lstVisionEvent)
{
   SPR_ProcessEvent lstMsgEvent;
   SI32 ls32Return;

   /* set the process id*/
   lstMsgEvent.stEvent.eProcessId = SPR_eProcessIdVisionControl;
   
   /* update the vison event part */
   lstMsgEvent.stEvent.List.stVisionEvent = lstVisionEvent;

   ls32Return = mq_send(stMsgQueue, lstMsgEvent.acBuffer, sizeof(SPR_ProcessEvent), SPR_PRIORITY_LOW);
   if(ls32Return!=0)
   {
       printf("mq_send:%s\n",strerror(errno)); 
   }
}

int COM_InitCommunicationWithSupervisorProcess(void)
{
    /* open the message queue created by supervisor process to send vision events to supervisior */
    stMsgQueue = mq_open(SPR_ucPathOfSupervisorMsgQueue, O_WRONLY| O_NONBLOCK, 0, NULL);
    if ((int) stMsgQueue == -1)
    {
        perror("vision_control error : mq_open failed !\n");
        return(-1);
    }
    return(0);
}

    
}/* end extern C */
