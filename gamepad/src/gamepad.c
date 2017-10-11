/*
 ============================================================================
 Name        : gamepad.c
 Author      : GAUTIER
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Joystick control in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

/* system include */
#include <sys/fcntl.h>
#include <sys/stat.h>

/* project include */
#include "stdtype.h"

#define GAMEPAD_LOG ///used to log all gamepad events

#define JOY_DEV "/dev/input/js1"

#define FIFO_FILE "/tmp/MotionControl.fifo" //FIFO used to exchange motion command
#define FIFO_CMD  "/tmp/VoiceControl.fifo" //FIFO used to exchange sound command

typedef struct
{
    UI08  u8MotorCommand;  //from 0 to 3
    SI16  u16PWMLevel;     //from 0 to 2499
    FL32  f32DeltaCompass; //from 0 to 359.9 in degree
}stMotionCommand;

typedef struct
{
    UI08  u8EspeakCommand;  //from 0 to 255
}stEspeakCommand;

SI16 s16LeftJoystickPositionX;
SI16 s16LeftJoystickPositionY;

SI16 s16RightJoystickPositionX;
SI16 s16RightJoystickPositionY;

SI16 s16LeftDirectionalButtonX;
SI16 s16LeftDirectionalButtonY;

SI16 s16Right2ButtonX;
SI16 s16Left2ButtonY;

stMotionCommand gstMotionCommand = {0,0,0.0f};

typedef void (*pfGamePadActionFunction)(__u8 u8Number, SI16 s16Value);

/*
 * function used to send motion command through pipe
 */
static void SendMotionCommandThroughFIFO(stMotionCommand* lstMotionCmd)
{
    FILE *pFileFIFIO;
#ifdef GAMEPAD_LOG
    printf("gamepad - u8MotorCommand:%d u16PWMLevel:%d f32DeltaCompass:%f\n",
           lstMotionCmd->u8MotorCommand,
           lstMotionCmd->u16PWMLevel,
           lstMotionCmd->f32DeltaCompass);
#endif
    // open FIFO
    if((pFileFIFIO = fopen(FIFO_FILE, "w")) == NULL) {
        perror("gamepad:fopen FIFO_FILE");
        exit(1);
     }
    // send the motion cmd
    fwrite(lstMotionCmd,sizeof(stMotionCommand),1, pFileFIFIO);

    // close the FIFO
    fclose(pFileFIFIO);
}

/*
 * function used to send espeak command through pipe
 */
static void SendEspeakCommandThroughFIFO(stEspeakCommand* lstEspeakCmd)
{
    FILE *pFileFIFIO;
#ifdef GAMEPAD_LOG
    printf("gamepad - Espeak Command:%d\n",
           lstEspeakCmd->u8EspeakCommand);
#endif
    // open FIFO
    if((pFileFIFIO = fopen(FIFO_CMD, "w")) == NULL) {
        perror("gamepad:fopen FIFO_FILE");
        exit(1);
     }
    // send the motion cmd
    fwrite(lstEspeakCmd,sizeof(stEspeakCommand),1, pFileFIFIO);

    // close the FIFO
    fclose(pFileFIFIO);
}


UI16 CalculatePWM(SI16 ls16JoystickX, SI16 ls16JoystickY, UI16 lu16MaxJoystickValue, UI16 lu16MaxPWM)
{
    FL32  lf32PWM;
    // absolute value
    if(ls16JoystickX<0) ls16JoystickX=-ls16JoystickX;
    if(ls16JoystickY<0) ls16JoystickY=-ls16JoystickY;

    if((ls16JoystickX > lu16MaxJoystickValue) || (ls16JoystickY > lu16MaxJoystickValue))
    {
        lf32PWM = 0.0;
    }
    else if((ls16JoystickX > 0) && (ls16JoystickY > 0))
    {
        lf32PWM = sqrt(((FL32)ls16JoystickX) * ((FL32)ls16JoystickY))/lu16MaxJoystickValue*lu16MaxPWM;
    }
    else if((ls16JoystickX > 0) && (ls16JoystickY == 0))
    {
        lf32PWM = (((FL32)ls16JoystickX)/lu16MaxJoystickValue*lu16MaxPWM);
    }
    else if((ls16JoystickX == 0) && (ls16JoystickY > 0))
    {
        lf32PWM = (((FL32)ls16JoystickY)/lu16MaxJoystickValue*lu16MaxPWM);
    }

    return(((UI32)lf32PWM)&0xFFFF);
}

FL32 f32CalculateDeltaCompass(SI16 ls16JoystickX, SI16 ls16JoystickY, UI16 lu16MaxJoystickValue)
{
    FL64 lfl64DeltaCompass,lf64_pi;
    if((ls16JoystickX > lu16MaxJoystickValue) || (ls16JoystickY > lu16MaxJoystickValue))
    {
        lfl64DeltaCompass = 0.0;
    }
    else
    {
        lf64_pi = 4 * atan(1);
        lfl64DeltaCompass= (atan2((FL64)ls16JoystickX, (FL64)ls16JoystickY)) * ((FL64)180.0/lf64_pi);
    }
    return((FL32)lfl64DeltaCompass);
}

FL32 f32LimitDeltaCompass(FL32 lf32DeltaCompass)
{
    if(lf32DeltaCompass < -60.0)
    {
        return(-60.0);
    }
    else if (lf32DeltaCompass > +60.0)
    {
        return(+60.0);
    }
    else
    {
        return(lf32DeltaCompass);
    }
}

void CalculateMotionCommandWithAnalogJoystickInfo(SI16 s16JoystickX , SI16 s16JoystickY, stMotionCommand* pstMotionCmd)
{
    UI16 u16PWM;
    FL32 f32DeltaCompass,f32DeltaCompassLimited;

    u16PWM = CalculatePWM(s16JoystickX,s16JoystickY,32767,2499);
    // calculate delta compass from 0° to 180° on the right side and from 0° to -180° on the left side
    f32DeltaCompass = f32CalculateDeltaCompass(s16JoystickX,s16JoystickY,32767);
#ifdef GAMEPAD_LOG
    printf("X:%5.5d Y:%5.5d u16PWM:%4.4d f32DeltaCompass:%f ", s16JoystickX,s16JoystickY,u16PWM,f32DeltaCompass);
#endif
    //calculate motor order, PWM, and DeltaCompass
    if((-90.0<=f32DeltaCompass) && (f32DeltaCompass<=90.0)) // front area
    {
        //limit delta compass   -90 °<= Delta Compass <=90 °
        f32DeltaCompassLimited = f32LimitDeltaCompass(f32DeltaCompass);
        if(f32DeltaCompass == f32DeltaCompassLimited)//not limited
        {
            pstMotionCmd->u8MotorCommand  = 1;
            pstMotionCmd->u16PWMLevel = u16PWM;
            pstMotionCmd->f32DeltaCompass = f32DeltaCompassLimited;
        }
        else
        {
            //do not modify the motor command, the joystick is in black area
            pstMotionCmd->u16PWMLevel = u16PWM;
            pstMotionCmd->f32DeltaCompass = f32DeltaCompassLimited;
        }
    }
    else if((+90.0< f32DeltaCompass) && (f32DeltaCompass<=180.0)) //area 90 °< Delta Compass <= 180 °
    {
        f32DeltaCompass = -(180.0-f32DeltaCompass);
        f32DeltaCompassLimited = f32LimitDeltaCompass(f32DeltaCompass);
        if(f32DeltaCompass == f32DeltaCompassLimited) //delta compas not limited
        {
            pstMotionCmd->u8MotorCommand  = 3;
            pstMotionCmd->u16PWMLevel = u16PWM;
            pstMotionCmd->f32DeltaCompass = f32DeltaCompassLimited;
        }
        else
        {
            //do not modify the motor command, the joystick is in black area
            pstMotionCmd->u16PWMLevel = u16PWM;
            pstMotionCmd->f32DeltaCompass = f32DeltaCompassLimited;
        }
    }
    else if((-180.0<=f32DeltaCompass) && (f32DeltaCompass<-90.0)) //area -180 °<= Delta Compass < -90 °
    {
        f32DeltaCompass = 180.0+f32DeltaCompass;
        f32DeltaCompassLimited = f32LimitDeltaCompass(f32DeltaCompass);
        if(f32DeltaCompass == f32DeltaCompassLimited) //not limited
        {
            pstMotionCmd->u8MotorCommand  = 3;
            pstMotionCmd->u16PWMLevel = u16PWM;
            pstMotionCmd->f32DeltaCompass = f32DeltaCompassLimited;
        }
        else
        {
            //do not modify the motor command, the joystick is in black area
          pstMotionCmd->u16PWMLevel = u16PWM;
          pstMotionCmd->f32DeltaCompass = f32DeltaCompassLimited;
        }
    }
    else
    {
        pstMotionCmd->u8MotorCommand = 0;
        pstMotionCmd->u16PWMLevel = 0;
        pstMotionCmd->f32DeltaCompass = 0;
    }
#ifdef GAMEPAD_LOG
    printf("u8MotorCommand:%d u16PWMLevel:%d f32DeltaCompass:%f\n",
           pstMotionCmd->u8MotorCommand,
           pstMotionCmd->u16PWMLevel,
           pstMotionCmd->f32DeltaCompass);
#endif
}

static void ManageEventOfLeftDirectionalButton(void)
{
    // release all direction buttons
    if     ((s16LeftDirectionalButtonX ==     0) && (s16LeftDirectionalButtonY == 0))
    {
        gstMotionCommand.f32DeltaCompass = 0.0;
    }
    // front (=8)
    else if((s16LeftDirectionalButtonX ==     0) && (s16LeftDirectionalButtonY == 32767))
    {
        gstMotionCommand.f32DeltaCompass = 0.0;
        gstMotionCommand.u8MotorCommand = 1;
    }
    // back (=2)
    else if((s16LeftDirectionalButtonX ==     0) && (s16LeftDirectionalButtonY == -32767))
    {
        gstMotionCommand.f32DeltaCompass  = 0.0;
        gstMotionCommand.u8MotorCommand = 3;
    }
    // front on the right (=9)
    else if((s16LeftDirectionalButtonX == 32767) && (s16LeftDirectionalButtonY == 32767))
    {
        gstMotionCommand.f32DeltaCompass = 10.0;
        gstMotionCommand.u8MotorCommand = 1;
    }
    // front on the left (=7)
    else if((s16LeftDirectionalButtonX ==-32767) && (s16LeftDirectionalButtonY == 32767))
    {
        gstMotionCommand.f32DeltaCompass = -10.0;
        gstMotionCommand.u8MotorCommand = 1;
    }
    // right  (=6)
    else if((s16LeftDirectionalButtonX == 32767) && (s16LeftDirectionalButtonY == 0))
    {
        gstMotionCommand.f32DeltaCompass = 45.0;
    }
    // left  (=4)
    else if((s16LeftDirectionalButtonX ==-32767) && (s16LeftDirectionalButtonY == 0))
    {
        gstMotionCommand.f32DeltaCompass = -45.0;
    }

    // back on the left (=1)
    else if((s16LeftDirectionalButtonX ==-32767) && (s16LeftDirectionalButtonY == -32767))
    {
        gstMotionCommand.f32DeltaCompass = -10.0;
        gstMotionCommand.u8MotorCommand = 3;
    }
    // back on the right (=3)
    else if((s16LeftDirectionalButtonX == 32767) && (s16LeftDirectionalButtonY == -32767))
    {
        gstMotionCommand.f32DeltaCompass = +10.0;
        gstMotionCommand.u8MotorCommand = 3;
    }
    else
    {

    }
    SendMotionCommandThroughFIFO(&gstMotionCommand);
}

/*
 * List of all functions called when a gamepad button changed his state
 */
void ActionButton0(__u8 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton A\n");
#endif
    if(s16Value == 1)
    {
        if(gstMotionCommand.u16PWMLevel <= (2449-10)) gstMotionCommand.u16PWMLevel+=10;
        SendMotionCommandThroughFIFO(&gstMotionCommand);
    }
}

void ActionButton1(__u8 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton B\n");
#endif
    if(s16Value == 1)
    {
        if(gstMotionCommand.u16PWMLevel >= 10) gstMotionCommand.u16PWMLevel-=10;
        SendMotionCommandThroughFIFO(&gstMotionCommand);
    }
}

void ActionButton2(__u8 u8Number, SI16 s16Value)
{

}

void ActionButton3(__u8 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton X\n");
#endif
    gstMotionCommand.u8MotorCommand = 0;
    gstMotionCommand.u16PWMLevel = 0;
    gstMotionCommand.f32DeltaCompass = 0.0;
    SendMotionCommandThroughFIFO(&gstMotionCommand);
}

void ActionButton4(__u8 u8Number, SI16 s16Value)
{
  stEspeakCommand lstEspeakCmd = {3};
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton Y\n");
#endif
    if(s16Value == 1)
    {
        SendEspeakCommandThroughFIFO(&lstEspeakCmd);
    }
}

void ActionButton5(__u8 u8Number, SI16 s16Value)
{

}

void ActionButton6(__u8 u8Number, SI16 s16Value)
{
    stEspeakCommand lstEspeakCmd = {1};
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton L1\n");
#endif
    if(s16Value == 1)
    {
        SendEspeakCommandThroughFIFO(&lstEspeakCmd);
    }
}

void ActionButton7(__u8 u8Number, SI16 s16Value)
{
    stEspeakCommand lstEspeakCmd = {2};
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton R1\n");
#endif
    if(s16Value == 1)
    {
        SendEspeakCommandThroughFIFO(&lstEspeakCmd);
    }
}

void ActionButton8(__u8 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton L2\n");
#endif
}

void ActionButton9(__u8 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("Boutton R2\n");
#endif
}

void ActionButton10(__u8 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton Select\n");
#endif
}

void ActionButton11(__u8 u8Number, SI16 s16Value)
{
    int result;
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton Start\n");
#endif

    if(s16Value == 1)
    {
        /* start robot application */
        printf("start motion control\n");

        /* create a pipe FIFO required for motion control*/
        result = mknod (FIFO_FILE,S_IRUSR| S_IWUSR|S_IFIFO, 0);
        if (result < 0) {
            perror ("fifo_read:mknod");
        }

        /* create /dev/rfcomm0 device and call start motion control*/
        system("rfcomm bind hci0  EC:FE:7E:13:CA:FE 1");
        system("/media/linaro/DATA/Hikey/Hikey/motion_control/Debug/motion_control /dev/rfcomm0 &");

        /* create a pipe FIFO required for voice control*/
        result = mknod (FIFO_CMD,S_IRUSR| S_IWUSR|S_IFIFO, 0);
        if (result < 0) {
            perror ("fifo_read:mknod");
        }

        printf("start voice control\n");
        system("/media/linaro/DATA/Hikey/Hikey/voice_control/Debug/voice_control &");
    }
    else if(s16Value == 0)
    {
        /* stop robot application */
        //TODO
    }
}

void ActionButton12(__u8 u8Number, SI16 s16Value)
{

}

void ActionButton13(__u8 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton L joystick\n");
#endif
}

void ActionButton14(__u8 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("gamepad - Boutton R joystick\n");
#endif
}

void ActionButton15(__u8 u8Number, SI16 s16Value)
{

}

void ActionButton16(__u8 u8Number, SI16 s16Value)
{

}

void ActionButton17(__u8 u8Number, SI16 s16Value)
{

}

void ActionButton18(__u8 u8Number, SI16 s16Value)
{

}

void ActionButton19(__u8 u8Number, SI16 s16Value)
{

}

/*
 * List of all functions called when a gamepad axis changed his state
 */
void ActionAxis0(__u8 u8Number, SI16 s16Value)
{
    s16LeftJoystickPositionX = s16Value;
#ifdef GAMEPAD_LOG
    printf("gamepad - Left joystick x:%d y:%d\n",s16LeftJoystickPositionX,s16LeftJoystickPositionY);
#endif
}

void ActionAxis1(__u8 u8Number, SI16 s16Value)
{
    s16LeftJoystickPositionY = -s16Value;
#ifdef GAMEPAD_LOG
    printf("gamepad - Left joystick x:%d y:%d\n",s16LeftJoystickPositionX,s16LeftJoystickPositionY);
#endif
}

void ActionAxis2(__u8 u8Number, SI16 s16Value)
{
    s16RightJoystickPositionX = s16Value;
#ifdef GAMEPAD_LOG
    printf("gamepad - Right joystick x:%d y:%d\n",s16RightJoystickPositionX,s16RightJoystickPositionY);
#endif
    CalculateMotionCommandWithAnalogJoystickInfo(s16RightJoystickPositionX,s16RightJoystickPositionY,&gstMotionCommand);
    SendMotionCommandThroughFIFO(&gstMotionCommand);
}

void ActionAxis3(__u8 u8Number, SI16 s16Value)
{
    s16RightJoystickPositionY = -s16Value;
#ifdef GAMEPAD_LOG
    printf("gamepad - Right joystick x:%d y:%d\n",s16RightJoystickPositionX,s16RightJoystickPositionY);
#endif
    CalculateMotionCommandWithAnalogJoystickInfo(s16RightJoystickPositionX,s16RightJoystickPositionY,&gstMotionCommand);
    SendMotionCommandThroughFIFO(&gstMotionCommand);
}

void ActionAxis4(__u8 u8Number, SI16 s16Value)
{
    s16Right2ButtonX = -s16Value;
#ifdef GAMEPAD_LOG
    printf("gamepad - Right joystick x:%d y:%d\n",s16Right2ButtonX,s16Left2ButtonY);
#endif
}

void ActionAxis5(__u8 u8Number, SI16 s16Value)
{
    s16Left2ButtonY = -s16Value;
#ifdef GAMEPAD_LOG
    printf("gamepad - Right joystick x:%d y:%d\n",s16Right2ButtonX,s16Left2ButtonY);
#endif
}

void ActionAxis6(__u8 u8Number, SI16 s16Value)
{
    s16LeftDirectionalButtonX = s16Value;
#ifdef GAMEPAD_LOG
    printf("gamepad - Left directional button x:%d y:%d\n",s16LeftDirectionalButtonX,s16LeftDirectionalButtonY);
#endif
    ManageEventOfLeftDirectionalButton();
}

void ActionAxis7(__u8 u8Number, SI16 s16Value)
{
    s16LeftDirectionalButtonY = -s16Value;
#ifdef GAMEPAD_LOG
    printf("gamepad - Left directional button x:%d y:%d\n",s16LeftDirectionalButtonX,s16LeftDirectionalButtonY);
#endif
    ManageEventOfLeftDirectionalButton();
}

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
/* processus in charge to:
 * - wait the detect the first gamepad connected
 * - read gamepad description (name, number of buttons, numbers of axis..)
 * - in the infinite loop, the software call a user function for each game pad events
 */
int main()
{
    int joy_fd;
    int *axis=NULL;
    int num_of_axis=0;
    int num_of_buttons=0;
    int num_of_Version=0;
    int lReadStatus = sizeof(struct js_event);

    char *button=NULL;
    char name_of_joystick[80];
    struct js_event js;

    /** wait gamepad */
    while( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 )
    {
        printf( "Couldn't open joystick\n" );
        sleep(2);/* wait 2 s*/
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
printf("lReadStatus%d",lReadStatus);
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
                        //TODO add init function           pfGamePadAxisActionArray[ js.number ](js.number,js.value);
                        break;
                    case JS_EVENT_BUTTON:
                        button [ js.number ] = js.value;
                        //TODO add init function           pfGamePadButtonActionArray[ js.number ](js.number,js.value);
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
                        pfGamePadAxisActionArray[ js.number ](js.number,js.value);
                        break;
                    case JS_EVENT_BUTTON:
                        button [ js.number ] = js.value;
                        pfGamePadButtonActionArray[ js.number ](js.number,js.value);
                        break;
                }
            }
        }
    }
    close( joy_fd );
    return 0;
}
