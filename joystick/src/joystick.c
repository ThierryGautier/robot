/*
 ============================================================================
 Name        : joystick.c
 Author      : GAUTIER
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Joystick control in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/ioctl.h>

#include <linux/joystick.h>

#define JOY_DEV "/dev/input/js1"


__s16 s16LeftJoystickPositionX;
__s16 s16LeftJoystickPositionY;

__s16 s16RightJoystickPositionX;
__s16 s16RightJoystickPositionY;

__s16 s16LeftDirectionalButtonX;
__s16 s16LeftDirectionalButtonY;

__s16 s16Right2ButtonX;
__s16 s16Left2ButtonY;

typedef void (*pfGamePadActionFunction)(__u8 u8Number, __s16 s16Value);

/*
 * List of all functions called when a gamepad button changed his state
 */
void ActionButton0(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton A\n");
}

void ActionButton1(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton B\n");
}

void ActionButton2(__u8 u8Number, __s16 s16Value)
{

}

void ActionButton3(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton X\n");
}

void ActionButton4(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton Y\n");
}

void ActionButton5(__u8 u8Number, __s16 s16Value)
{

}

void ActionButton6(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton L1\n");
}

void ActionButton7(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton R1\n");
}

void ActionButton8(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton L2\n");
}

void ActionButton9(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton R2\n");
}

void ActionButton10(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton Select\n");
}

void ActionButton11(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton Start\n");
}

void ActionButton12(__u8 u8Number, __s16 s16Value)
{

}

void ActionButton13(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton L joystick\n");
}

void ActionButton14(__u8 u8Number, __s16 s16Value)
{
    printf("Boutton R joystick\n");
}

void ActionButton15(__u8 u8Number, __s16 s16Value)
{

}

void ActionButton16(__u8 u8Number, __s16 s16Value)
{

}

void ActionButton17(__u8 u8Number, __s16 s16Value)
{

}

void ActionButton18(__u8 u8Number, __s16 s16Value)
{

}

void ActionButton19(__u8 u8Number, __s16 s16Value)
{

}

/*
 * List of all functions called when a gamepad axis changed his state
 */
void ActionAxis0(__u8 u8Number, __s16 s16Value)
{
    s16LeftJoystickPositionX = s16Value;
    printf("Left joystick x:%d y:%d\n",s16LeftJoystickPositionX,s16LeftJoystickPositionY);
}

void ActionAxis1(__u8 u8Number, __s16 s16Value)
{
    s16LeftJoystickPositionY = -s16Value;
    printf("Left joystick x:%d y:%d\n",s16LeftJoystickPositionX,s16LeftJoystickPositionY);
}

void ActionAxis2(__u8 u8Number, __s16 s16Value)
{
    s16RightJoystickPositionX = s16Value;
    printf("Right joystick x:%d y:%d\n",s16RightJoystickPositionX,s16RightJoystickPositionY);
}

void ActionAxis3(__u8 u8Number, __s16 s16Value)
{
    s16RightJoystickPositionY = -s16Value;
    printf("Right joystick x:%d y:%d\n",s16RightJoystickPositionX,s16RightJoystickPositionY);
}

void ActionAxis4(__u8 u8Number, __s16 s16Value)
{
	s16Right2ButtonX = -s16Value;
    printf("Right joystick x:%d y:%d\n",s16Right2ButtonX,s16Left2ButtonY);
}

void ActionAxis5(__u8 u8Number, __s16 s16Value)
{
	s16Left2ButtonY = -s16Value;
    printf("Right joystick x:%d y:%d\n",s16Right2ButtonX,s16Left2ButtonY);
}

void ActionAxis6(__u8 u8Number, __s16 s16Value)
{
	s16LeftDirectionalButtonX = s16Value;
    printf("Left directional button x:%d y:%d\n",s16LeftDirectionalButtonX,s16LeftDirectionalButtonY);
}

void ActionAxis7(__u8 u8Number, __s16 s16Value)
{
	s16LeftDirectionalButtonY = -s16Value;
    printf("Left directional button x:%d y:%d\n",s16LeftDirectionalButtonX,s16LeftDirectionalButtonY);
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

    char *button=NULL, name_of_joystick[80];
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

    while( 1 )      /* infinite loop */
    {
        int lReadStatus;

        /* wait a joystick event */
        lReadStatus = read(joy_fd, &js, sizeof(struct js_event));

        /* joystick USB or bluetooth disconnected */
        if(lReadStatus == -1)
        {
           close( joy_fd );
           return 0;
        }
        /* print the event occurred */
        printf("Event=number:%2.2x, time:%d, type:%2.2x, value:%d \n",js.number,js.time,js.type,js.value);
        fflush(stdout);

        /*init state of each input from gamepad */
        if((js.type & JS_EVENT_INIT)==JS_EVENT_INIT)
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
    close( joy_fd );
    return 0;
}
