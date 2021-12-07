/*
 ============================================================================
 Name        : gamepad.c
 Author      : GAUTIER
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Joystick control in C, Ansi-style
 ============================================================================
 */

/* system include */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <mqueue.h>
#include <errno.h>
#include <linux/joystick.h>

/* project include */
#include "stdtype.h"
#include "supervisor.h"

//#define GAMEPAD_LOG ///used to log all gamepad events

typedef void (*pfGamePadActionFunction)(mqd_t stMsgQueue, UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);

/* path of the gamepad device */
CHAR ucPathOfDevice[64];

/* file descriptor of gamepad  */
static int joy_fd=0;

/*
 * function used to send gamepad event
 */
static void SendGamepadEventThroughMsgQueue(mqd_t stMsgQueue, SPR_eGamepadTypeEvent eTypeEvt, UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
   SPR_ProcessEvent lstMsgEvent;
   SI32 ls32Return;

   /* set the process id*/
   lstMsgEvent.stEvent.eProcessId = SPR_eProcessIdGamepad;
   
   /* update the gamepad event part */
   lstMsgEvent.stEvent.List.stGamepadEvent.eTypeOfEvent = eTypeEvt;
   lstMsgEvent.stEvent.List.stGamepadEvent.u32TimeOfGamepadEventIn_us =u32TimeOfGamepadEventIn_us;
   lstMsgEvent.stEvent.List.stGamepadEvent.u8Number = u8Number;
   lstMsgEvent.stEvent.List.stGamepadEvent.s16Value = s16Value;

   ls32Return = mq_send(stMsgQueue, lstMsgEvent.acBuffer, sizeof(SPR_ProcessEvent), SPR_PRIORITY_LOW);
   if(ls32Return!=0)
   {
       printf("mq_send:%s\n",strerror(errno)); 
   }
}

/*
 * List of all functions called when a gamepad button changed his state
 */
void ActionButton0(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton A\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton1(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton B\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton2(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    /* do nothing no event for the current gamepad */
}

void ActionButton3(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton X\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton4(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton Y\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton5(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    /* do nothing no event for the current gamepad */
}

void ActionButton6(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton L1\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton7(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton R1\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton8(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton L2\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton9(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton R2\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton10(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton Select\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,s16Value);
}

void ActionButton11(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton Start\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton12(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    /* do nothing no event for the current gamepad */
}

void ActionButton13(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton L joystick\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton14(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton R joystick\n");
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadButtonEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionButton15(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    /* do nothing no event for the current gamepad */
}

void ActionButton16(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    /* do nothing no event for the current gamepad */
}

void ActionButton17(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    /* do nothing no event for the current gamepad */
}

void ActionButton18(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    /* do nothing no event for the current gamepad */
}

void ActionButton19(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    /* do nothing no event for the current gamepad */
}

/*
 * List of all functions called when a gamepad axis changed his state
 */
void ActionAxis0(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Left joystick x:%d\n",s16Value);
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadAxisEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionAxis1(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Left joystick y:%d\n",s16Value);
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadAxisEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionAxis2(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Right joystick x:%d\n",s16Value);
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadAxisEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionAxis3(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Right joystick y:%d\n",s16Value);
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadAxisEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionAxis4(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Right joystick x:%d\n",s16Value);
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadAxisEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionAxis5(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Right joystick y:%d\n",s16Value);
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadAxisEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionAxis6(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Left directional button x:%d\n",s16Value);
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadAxisEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}

void ActionAxis7(mqd_t stMsgQueue,UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Left directional button y:%d\n",s16Value);
#endif
    SendGamepadEventThroughMsgQueue(stMsgQueue,
                                    SPR_eGamepadAxisEvt,
                                    u32TimeOfGamepadEventIn_us,
                                    u8Number,
                                    s16Value);
}
/* list of all function manage buttons events of gamepad */
pfGamePadActionFunction pfGamePadButtonActionArray[19] =
{
    ActionButton0,
    ActionButton1,
    ActionButton2,
    ActionButton3,
    ActionButton4,
    ActionButton5,
    ActionButton6,
    ActionButton7,
    ActionButton8,
    ActionButton9,
    ActionButton10,
    ActionButton11,
    ActionButton12,
    ActionButton13,
    ActionButton14,
    ActionButton15,
    ActionButton16,
    ActionButton17,
    ActionButton18
};
/* list of all function manage axis events of gamepad */
pfGamePadActionFunction pfGamePadAxisActionArray[8] =
{
    ActionAxis0,
    ActionAxis1,
    ActionAxis2,
    ActionAxis3,
    ActionAxis4,
    ActionAxis5,
    ActionAxis6,
    ActionAxis7
};

static mqd_t stMsgQueueSPR = 0;

/* catch Signal to interrupt the gamepad process */
static void sig_handler(int signo)
{
    if (signo == SIGTERM)
    {
        /* close device if open */
        if(joy_fd!=0) close(joy_fd);
        if(stMsgQueueSPR!=0) close(stMsgQueueSPR);
        printf("received SIGTERM\n");
        exit(0);
    }
}
 
/* processus in charge to:
 * - wait the detect the gamepad device specified by his path (arg 1)
 * - read gamepad description (name, number of buttons, numbers of axis..)
 * - in the infinite loop, the software call a user function for each game pad events
 * - arg 0 path of process
 * - arg 1 path of device (/dev/input/js1)
 */
int main(int argc, char *argv[])
{
    int *axis=NULL;
    int num_of_axis=0;
    int num_of_buttons=0;
    int num_of_Version=0;
    int lReadStatus = sizeof(struct js_event);

    char *button=NULL;
    char name_of_joystick[80];
    struct js_event js;

    /* check arguments */
    if(argc!=2)
    {
        printf("no enough arguments\n");
        printf("1: path of device (/dev/input/js0)\n");
        exit(-1);
    }

    /* saved all parameters */
    strncpy(ucPathOfDevice,argv[1],sizeof(ucPathOfDevice));

    /* catch SIGTERM signal */
    if (signal(SIGTERM, sig_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGTERM\n");
    }
    
    /* open the message queue created by supervisor process to send gamepad events */
    stMsgQueueSPR = mq_open(SPR_ucPathOfSupervisorMsgQueue, O_WRONLY |O_NONBLOCK, 0, NULL);
    if ((int) stMsgQueueSPR == -1)
    {
        perror("gamepad error : mq_open failed !\n");
    }

    /*  wait gamepad device */
    while( ( joy_fd = open(ucPathOfDevice , O_RDONLY)) == -1 )
    {
        printf( "Couldn't open gamepad device: %s \n",argv[1] );
        sleep(1);/* wait 1 s*/
    }

    /* read gamepad description */
    ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
    ioctl( joy_fd, JSIOCGVERSION, &num_of_Version );
    ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
    ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

    /* print joystick description */
    printf("Joystick detected: %s\n\t%d version\n\t%d axis\n\t%d buttons\n\n"
            , name_of_joystick
            , num_of_Version
            , num_of_axis
            , num_of_buttons );

    /* allocate the buttons and axis required for this game pad */
    axis = (int *) calloc( num_of_axis, sizeof( int ) );
    button = (char *) calloc( num_of_buttons, sizeof( char ) );

    //fcntl( joy_fd, F_SETFL, O_NONBLOCK );   /* use non-blocking mode: code in comment to used blocking mode (reduce CPU usage) */

    /* infinite loop until device close or an error occurred */
    while( lReadStatus == sizeof(struct js_event) )
    {
        /* wait a joystick event */
        lReadStatus = read(joy_fd, &js, sizeof(struct js_event));

        /* joystick USB or bluetooth is OK */
        if(lReadStatus == sizeof(struct js_event))
        {
        /* print the event occurred */
#ifdef GAMEPAD_LOG
            printf("Event=number:%2.2x, time:%d, type:%2.2x, value:%d \n",js.number,js.time,js.type,js.value);
            fflush(stdout);
#endif
            /*init state of each input from gamepad */
            if((js.type & JS_EVENT_INIT)==JS_EVENT_INIT)
            {
                /* see what to do with the event */
                switch (js.type & ~JS_EVENT_INIT)
                {
                    case JS_EVENT_AXIS:
                        axis   [ js.number ] = js.value;
                        //TODO add init function           pfGamePadAxisActionArray[ js.number ](js.time,js.number,js.value);
                        break;
                    case JS_EVENT_BUTTON:
                        button [ js.number ] = js.value;
                        //TODO add init function           pfGamePadButtonActionArray[ js.number ](js.time,js.number,js.value);
                        break;
                }
            }
            /* gamepad event of one input from gamepad */
            else
            {
                /* see what to do with the event */
                switch (js.type & ~JS_EVENT_INIT)
                {
                    case JS_EVENT_AXIS:
                        axis   [ js.number ] = js.value;
                        pfGamePadAxisActionArray[ js.number ](stMsgQueueSPR,js.time,js.number,js.value);
                        break;
                    case JS_EVENT_BUTTON:
                        button [ js.number ] = js.value;
                        pfGamePadButtonActionArray[ js.number ](stMsgQueueSPR,js.time,js.number,js.value);
                        break;
                }
            }
        }
    }
    close( joy_fd );
    return 0;
}
