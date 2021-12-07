/*
 ============================================================================
 Name        : supervisor.c
 Author      : GAUTIER
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : received all info from captors and take a decision
 ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <sys/ioctl.h>
#include <errno.h>
#include <mqueue.h>
#include <sched.h>

/* project include */
#include "stdtype.h"
#include "motion_control.h"
#include "supervisor.h"


//#define GAMEPAD_LOG ///used to log all gamepad events

/* time min between two motion control command
   gamepad is able to generate an event at 10 ms
   but the communication HDLC between linux board and FRDMKV31 is no able to manage this frequency of events */
#define U32_TIME_MIN_BETWEEN_TWO_MOTION_CONTROL_CMD_IN_1MS 20U

/* time min between two voice command
   gamepad is able to generate an event at 10 ms
   but it' stupid to send voice command every 10 ms so the software limit it at 1 s  */
#define U32_TIME_MIN_BETWEEN_TWO_VOICE_CONTROL_CMD_IN_1MS 1000U

typedef void (*pfGamePadActionFunction)(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);

typedef struct
{
    UI08  u8EspeakCommand;  //from 0 to 255
}stEspeakCommand;

/***/
void ActionButton0(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton1(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton2(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton3(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton4(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton5(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton6(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton7(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton8(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton9(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton10(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton11(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton12(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton13(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton14(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton15(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton16(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton17(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionButton18(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);

void ActionAxis0(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionAxis1(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionAxis2(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionAxis3(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionAxis4(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionAxis5(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionAxis6(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);
void ActionAxis7(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value);

SI16 s16LeftJoystickPositionX;
SI16 s16LeftJoystickPositionY;

SI16 s16RightJoystickPositionX;
SI16 s16RightJoystickPositionY;

SI16 s16LeftDirectionalButtonX;
SI16 s16LeftDirectionalButtonY;

SI16 s16Right2ButtonX;
SI16 s16Left2ButtonY;

MCL_stMotionCommand gstMotionCommand = {0,0,0.0f};
UI32 u32PreviousTimeOfVoiceCommandIn_us = 0;

/* list of msg queue */
mqd_t stMsgQueueRequest = 0;
mqd_t stMsgQueueSupervisorToMotionControl = 0;

static UI64 u64ListOfDeltaTimeInms[256];

/* list of all gamepad function link to logical buttons */
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

/* list of all gamepad function link to analog button */
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


/** get time in nano second
 */
static UI64 u64GetTimeInns(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return((UI64)spec.tv_nsec);
}

/*
 * function used to send motion command through msg queue
 */
static void SendMotionCommand(MCL_stMotionCommand* lstMotionCmd)
{
    SI32 ls32Return;

    // send the motion cmd
    ls32Return = mq_send(stMsgQueueSupervisorToMotionControl,(CHAR*)lstMotionCmd,sizeof(MCL_stMotionCommand),0);
    if(ls32Return!=0)
    {
        printf("supervisor mq_send:%s\n",strerror(errno));
    }
}

UI16 CalculatePWM(SI16 ls16JoystickX, SI16 ls16JoystickY, UI16 lu16MaxJoystickValue, UI16 lu16MaxPWM)
{
    FL32  lf32PWM = 0;
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
    else
    {
        lf32PWM = 0;
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

void CalculateMotionCommandWithAnalogJoystickInfo(SI16 s16JoystickX , SI16 s16JoystickY, MCL_stMotionCommand* pstMotionCmd)
{
    UI16 u16PWM;
    FL32 f32DeltaCompass,f32DeltaCompassLimited;

    if((s16JoystickX !=0) || (s16JoystickY!=0))
    {
        u16PWM = CalculatePWM(s16JoystickX,s16JoystickY,32767,2499);
        // calculate delta compass from 0 to 180 on the right side and from 0 to -180 on the left side
        f32DeltaCompass = f32CalculateDeltaCompass(s16JoystickX,s16JoystickY,32767);
#ifdef GAMEPAD_LOG
        printf("X:%5.5d Y:%5.5d u16PWM:%4.4d f32DeltaCompass:%f ", s16JoystickX,s16JoystickY,u16PWM,f32DeltaCompass);
#endif
        //calculate motor order, PWM, and DeltaCompass
        if((-90.0<=f32DeltaCompass) && (f32DeltaCompass<=90.0)) // front area
        {
            //limit delta compass   -90 <= Delta Compass <=90
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
        else if((+90.0< f32DeltaCompass) && (f32DeltaCompass<=180.0)) //area 90 < Delta Compass <= 180
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
        else if((-180.0<=f32DeltaCompass) && (f32DeltaCompass<-90.0)) //area -180 <= Delta Compass < -90
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
    }
    else /*x and y = 0*/
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
}

/*
 * List of all functions called when a gamepad button changed his state
 */
void ActionButton0(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton A\n");
#endif
    if(s16Value == 1)
    {
        if(gstMotionCommand.u16PWMLevel <= (2449-10)) 
            gstMotionCommand.u16PWMLevel+=10;
        else
            gstMotionCommand.u16PWMLevel=2449;            
    }
}

void ActionButton1(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton B\n");
#endif
    if(s16Value == 1)
    {
        if(gstMotionCommand.u16PWMLevel >= 10)
            gstMotionCommand.u16PWMLevel-=10;
        else
            gstMotionCommand.u16PWMLevel=0;            
    }
}

void ActionButton2(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{

}

void ActionButton3(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton X\n");
#endif
    gstMotionCommand.u8MotorCommand = 0;
    gstMotionCommand.u16PWMLevel = 0;
    gstMotionCommand.f32DeltaCompass = 0.0;
}

void ActionButton4(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton Y\n");
#endif

}

void ActionButton5(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{

}

void ActionButton6(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton L1\n");
#endif
}

void ActionButton7(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton R1\n");
#endif

}

void ActionButton8(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton L2\n");
#endif
}

void ActionButton9(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton R2\n");
#endif
}

void ActionButton10(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton Select\n");
#endif
}

void ActionButton11(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton Start\n");
#endif
}

void ActionButton12(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{

}

void ActionButton13(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton L joystick\n");
#endif
}

void ActionButton14(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
#ifdef GAMEPAD_LOG
    printf("supervisor - Boutton R joystick\n");
#endif
}

void ActionButton15(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{

}

void ActionButton16(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{

}

void ActionButton17(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{

}

void ActionButton18(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{

}

void ActionButton19(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{

}

/*
 * List of all functions called when a gamepad axis changed his state
 */
void ActionAxis0(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    s16LeftJoystickPositionX = s16Value;
#ifdef GAMEPAD_LOG
    printf("supervisor - Left joystick x:%d y:%d\n",s16LeftJoystickPositionX,s16LeftJoystickPositionY);
#endif
}

void ActionAxis1(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    s16LeftJoystickPositionY = -s16Value;
#ifdef GAMEPAD_LOG
    printf("supervisor - Left joystick x:%d y:%d\n",s16LeftJoystickPositionX,s16LeftJoystickPositionY);
#endif
}

void ActionAxis2(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    s16RightJoystickPositionX = s16Value;
#ifdef GAMEPAD_LOG
    printf("supervisor - Right joystick x:%d y:%d\n",s16RightJoystickPositionX,s16RightJoystickPositionY);
#endif
    CalculateMotionCommandWithAnalogJoystickInfo(s16RightJoystickPositionX,s16RightJoystickPositionY,&gstMotionCommand);
}

void ActionAxis3(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    s16RightJoystickPositionY = -s16Value;
#ifdef GAMEPAD_LOG
    printf("supervisor - Right joystick x:%d y:%d\n",s16RightJoystickPositionX,s16RightJoystickPositionY);
#endif
    CalculateMotionCommandWithAnalogJoystickInfo(s16RightJoystickPositionX,s16RightJoystickPositionY,&gstMotionCommand);
}

void ActionAxis4(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    s16Right2ButtonX = -s16Value;
#ifdef GAMEPAD_LOG
    printf("supervisor - Right joystick x:%d y:%d\n",s16Right2ButtonX,s16Left2ButtonY);
#endif
}

void ActionAxis5(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    s16Left2ButtonY = -s16Value;
#ifdef GAMEPAD_LOG
    printf("supervisor - Right joystick x:%d y:%d\n",s16Right2ButtonX,s16Left2ButtonY);
#endif
}

void ActionAxis6(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    s16LeftDirectionalButtonX = s16Value;
#ifdef GAMEPAD_LOG
    printf("supervisor - Left directional button x:%d y:%d\n",s16LeftDirectionalButtonX,s16LeftDirectionalButtonY);
#endif
    ManageEventOfLeftDirectionalButton();
}

void ActionAxis7(UI32 u32TimeOfGamepadEventIn_us, UI08 u8Number, SI16 s16Value)
{
    s16LeftDirectionalButtonY = -s16Value;
#ifdef GAMEPAD_LOG
    printf("supervisor - Left directional button x:%d y:%d\n",s16LeftDirectionalButtonX,s16LeftDirectionalButtonY);
#endif
    ManageEventOfLeftDirectionalButton();
}

/**
 manage gamepad event */
void ManageGamepadEvent(SPR_stGamepadEvent lstGamepadEvent)
{
    /* see what to do with the event */
    switch (lstGamepadEvent.eTypeOfEvent)
    {
        case SPR_eGamepadAxisEvt:
            if(lstGamepadEvent.u8Number < SIZE_OF_ARRAY(pfGamePadAxisActionArray))
            {
                pfGamePadAxisActionArray[lstGamepadEvent.u8Number](lstGamepadEvent.u32TimeOfGamepadEventIn_us,lstGamepadEvent.u8Number,lstGamepadEvent.s16Value);
            }
            break;
        case SPR_eGamepadButtonEvt:
            if(lstGamepadEvent.u8Number < SIZE_OF_ARRAY(pfGamePadButtonActionArray))
            {
                pfGamePadButtonActionArray[lstGamepadEvent.u8Number](lstGamepadEvent.u32TimeOfGamepadEventIn_us,lstGamepadEvent.u8Number,lstGamepadEvent.s16Value);
            }
            break;
        default:
            break;
    }
}

/**
 manage motion event */
void ManageMotionEvent(SPR_stMotionEvent lstMotionEvent)
{
#if 1 ///TODO TG
    static UI64 u64CurrentTimeInns;
    static UI64 u64PrevIousTimeInns; 
    static UI64 u64DeltaTimeInns;
    static UI32 u32Index;
    
    u64CurrentTimeInns = u64GetTimeInns();


    if(u64PrevIousTimeInns>=u64CurrentTimeInns)
        u64DeltaTimeInns = u64PrevIousTimeInns-u64CurrentTimeInns;
    else
        u64DeltaTimeInns = u64CurrentTimeInns-u64PrevIousTimeInns;

    u64PrevIousTimeInns = u64CurrentTimeInns;
 
    //save value
    u64ListOfDeltaTimeInms[u32Index]= u64DeltaTimeInns;
    u32Index++;
    if(u32Index>=256) u32Index = 0;
#endif
}

/**
  manage vision event */
void ManageVisionEvent(SPR_stVisionEvent lstVisionEvent)
{
#define MIDDLE_SCEEN (FL32)0.5f
    
    FL32 f32DeltaX, f32DeltaY = 0;

    if(lstVisionEvent.eTypeOfEvent == SPR_eFaceDetection)
    {
        //calculate relative distance between center of screen and the center of face
        f32DeltaX = lstVisionEvent.f32X - MIDDLE_SCEEN;
        f32DeltaY = lstVisionEvent.f32Y - MIDDLE_SCEEN;


        gstMotionCommand.f32DeltaCompass = f32DeltaX*90;
        gstMotionCommand.u16PWMLevel     = 10;
        if(f32DeltaX > 0.1)
            gstMotionCommand.u8MotorCommand  = 1;
        else if (f32DeltaX < -0.1)
            gstMotionCommand.u8MotorCommand  = 1;
        else
            gstMotionCommand.u8MotorCommand = 0;

    }
    else if(lstVisionEvent.eTypeOfEvent == SPR_eBodyDetection)
    {
    }
    else
    {
        gstMotionCommand.f32DeltaCompass = 0;
        gstMotionCommand.u16PWMLevel     = 0;
        gstMotionCommand.u8MotorCommand  = 0;
    }
    printf("f32DeltaX:%f f32DeltaY:%f f32DeltaCompass:%f u16PWMLevel:%d u8MotorCommand:%d\n",f32DeltaX,f32DeltaY,
                                                                    gstMotionCommand.f32DeltaCompass,
                                                                    gstMotionCommand.u16PWMLevel,
                                                                    gstMotionCommand.u8MotorCommand);

    
}


/* catch Signal to interrupt the gamepad process */
static void sig_handler(int signo)
{
    if (signo == SIGTERM)
    {
        int i;
        /* print all statistic*/
        printf("supervisor - list of delta time in ns");
        for(i=0;i<256;i++)
             printf("delta time:%lld \n",u64ListOfDeltaTimeInms[i]);
        
        
        /*close all msg queue opened */
        if(stMsgQueueSupervisorToMotionControl!=0) mq_close(stMsgQueueSupervisorToMotionControl);
        if(stMsgQueueRequest!=0) mq_close(stMsgQueueRequest);
        printf("received SIGTERM\n");
        exit(0);
    }
}

/* processus in charge to:
 * - in the infinite loop, the software call a user function for each game pad events
 * - arg 0 path of process
 */
int main(int argc, CHAR *argv[])
{
    struct sched_param param;
    struct mq_attr    attr;
    ssize_t       len_recv;

    /* check arguments */
    if(argc!=1)
    {
        printf("no enough arguments\n");
        exit(-1);
    }

    /* catch SIGTERM signal */
    if (signal(SIGTERM, sig_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGTERM\n");
    }
    
    /* set process in RT */
    param.sched_priority = 48;/*priotity less than IRQ priority on board */
    if(sched_setscheduler(0,SCHED_FIFO,& param)!=0)
    {
        perror("sched_setscheduler !\n");
        return(-1);
    }
    
    /* open the message queue required to get event from all process (gamepad, vision, sound listen) */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(SPR_ProcessEvent);
    attr.mq_curmsgs = 0;

    printf("Length of SPR_ProcessEvent:%ld\n",sizeof(SPR_ProcessEvent));
    
    stMsgQueueRequest = mq_open(SPR_ucPathOfSupervisorMsgQueue, O_RDWR | O_CREAT, 0644, &attr);
    if ((int) stMsgQueueRequest == -1)
    {
        perror("Supervisor error : mq_open failed !\n");
        return(-1);
    }

    /* open the message queue required to communicate with motion_control */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(MCL_stMotionCommand);
    attr.mq_curmsgs = 0;

    printf("Length of MCL_stMotionCommand:%ld\n",sizeof(MCL_stMotionCommand));

    stMsgQueueSupervisorToMotionControl = mq_open(SPR_ucPathOfMotionControlMsgQueue, 
                                                  O_RDWR | O_CREAT |O_NONBLOCK, 
                                                  0644, 
                                                  &attr);
    if ((int) stMsgQueueSupervisorToMotionControl == -1)
    {
      perror("Supervisor error : mq_open failed !\n");
      return(-1);
    }

    printf("Supervisor is operational, wait events");
    
    /* infinite loop until device close or an error occurred */
    while(1)
    {
        SPR_ProcessEvent lstProcessEvent;
        unsigned int MsgPriority; 
        
        /* wait event from gamepad, motion_control and vision_control process */
        len_recv = mq_receive(stMsgQueueRequest, lstProcessEvent.acBuffer, sizeof(SPR_ProcessEvent), &MsgPriority);

        /* if length of msg match */
        if(len_recv == sizeof(SPR_ProcessEvent))
        {            
            switch (lstProcessEvent.stEvent.eProcessId)
            {
                case SPR_eProcessIdGamepad:
                    /** manage gamepad event received from bluetooth gamepad device */
                    ManageGamepadEvent(lstProcessEvent.stEvent.List.stGamepadEvent);
                    break;

                case SPR_eProcessIdMotionControl:
                    /** manage motion event received periodicaly from motion_control process ( FRDM-KV31 motherboard ) */
                    ManageMotionEvent(lstProcessEvent.stEvent.List.stMotionEvent);
                    
                    /** do the regulation */
                    //TODO
                    
                    /** send periodicaly an answerd*/
                    SendMotionCommand(&gstMotionCommand);
                    break;

                case SPR_eProcessIdVisionControl:
                    /** manage vision event received periodicaly from vision_control process (webcam and OpenCV appli */
                    ManageVisionEvent(lstProcessEvent.stEvent.List.stVisionEvent);
                    break;
                
                case SPR_eProcessIdSoundListen:
                    break;
                
                default:
                    printf("supervisor unknown process Id %d\n",lstProcessEvent.stEvent.eProcessId);
                    break;
            }
        }
        else
        {
            printf("supervisor unknown event size %ld\n",len_recv);
        }
    }
    return 0;
}

