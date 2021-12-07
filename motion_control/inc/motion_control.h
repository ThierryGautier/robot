#ifndef MOTION_CONTROL_H_
#define MOTION_CONTROL_H_

typedef struct
{
    UI08  u8MotorCommand;  //from 0 to 3
    SI16  u16PWMLevel;     //from 0 to 2499
    FL32  f32DeltaCompass; //from 0 to 359.9 in degree
}MCL_stMotionCommand;

#endif /* MOTION_CONTROL_H_ */
