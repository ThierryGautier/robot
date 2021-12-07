#ifndef SUPERVISOR_H_
#define SUPERVISOR_H_


/* list of captors able to send event to the supervisor */
typedef enum
{
    SPR_eProcessIdGamepad         = 1,
    SPR_eProcessIdVisionControl   = 2,
    SPR_eProcessIdSoundListen     = 3,
    SPR_eProcessIdMotionControl   = 4,
}SPR_eProcessId;


/* struct of data exchange between gamepad process and supervisor process through message queue */
/* game pad event part description */
typedef enum
{
    SPR_eGamepadButtonEvt = 1,
    SPR_eGamepadAxisEvt   = 2,
}SPR_eGamepadTypeEvent;

typedef struct
{
    SPR_eGamepadTypeEvent eTypeOfEvent; //from GAM_ButtonEvt or GAM_AxisEvt
    UI32 u32TimeOfGamepadEventIn_us;
    UI08 u8Number;                      //from 0 to 2499
    SI16 s16Value;                      //from 0 to 359.9 in degree
}SPR_stGamepadEvent;


/* struct of data exchange between motion_control process and supervisor process through message queue */
/* motion_control event part description */
typedef struct
{
    UI08 u8TxLife;
    UI08 u8RxLife;
    UI32 u32LatencyInus;
    FL32 f32Gx;
    FL32 f32Gy;
    FL32 f32Gz;
    FL32 f32Roll;
    FL32 f32Pitch;
    FL32 f32Compass;
}SPR_stMotionEvent;


/* struct of data exchange between sound_listem process and supervisor process through message queue */
/* sound event part description */
typedef enum
{
    SPR_eSoundEvt1 = 1,
    SPR_eSoundEv2  = 2,
}SPR_eSoundTypeEvent;

typedef struct
{
    SPR_eSoundTypeEvent eTypeOfEvent;
}SPR_stSoundEvent;

/* struct of data exchange between vision_control process and supervisor process through message queue */
/* vision event part description */
typedef enum
{
    SPR_eNoEvent = 0,
    SPR_eFaceDetection = 1,
    SPR_eBodyDetection = 2,
}SPR_eVisionTypeEvent;

typedef struct
{
    SPR_eVisionTypeEvent eTypeOfEvent;
    UI32 u32TimeOfVisionEventIn_us;
    FL32 f32X;
    FL32 f32Y;
    FL32 f32Width;
    FL32 f32Height;
}SPR_stVisionEvent;


/* struct to define an event between all capteur process and supervisor */
typedef struct
{
    SPR_eProcessId eProcessId;

    union
    {
        SPR_stGamepadEvent stGamepadEvent;
        SPR_stMotionEvent  stMotionEvent;
        SPR_stSoundEvent   stSoundEvent;
        SPR_stVisionEvent  stVisionEvent;
    }List;

}SPR_Event;

/** struct of data used to send events to the supervisor for all process */
typedef union
{
    SPR_Event stEvent;
    CHAR acBuffer[sizeof(SPR_Event)];
}SPR_ProcessEvent;


#define SPR_ucPathOfSupervisorMsgQueue "/MsgQueueSVR"
/* list of priority of SPR_ucPathOfSupervisorMsgQueue */
#define SPR_PRIORITY_LOW        (unsigned int)0U
#define SPR_PRIORITY_MOTION_EVT (unsigned int)1U


#define SPR_ucPathOfMotionControlMsgQueue "/MsgQueueMCL"

#endif /* SUPERVISOR_H_ */
