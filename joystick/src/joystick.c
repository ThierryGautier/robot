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

int main()
{
    int joy_fd;
    int *axis=NULL;
    int num_of_axis=0;
    int num_of_buttons=0;
    int num_of_Version=0;
    int x;
    char *button=NULL, name_of_joystick[80];
    struct js_event js;

    while( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 )
    {
        printf( "Couldn't open joystick\n" );
        //return -1;
    }

    ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
    ioctl( joy_fd, JSIOCGVERSION, &num_of_Version );
    ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
    ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );

    axis = (int *) calloc( num_of_axis, sizeof( int ) );
    button = (char *) calloc( num_of_buttons, sizeof( char ) );

    printf("Joystick detected: %s\n\t%d version\n\t%d axis\n\t%d buttons\n\n"
            , name_of_joystick
            , num_of_Version
            , num_of_axis
            , num_of_buttons );


    //fcntl( joy_fd, F_SETFL, O_NONBLOCK );   /* use non-blocking mode */

    while( 1 )      /* infinite loop */
    {
        int lReadStatus;

        /* read the joystick state */
        lReadStatus = read(joy_fd, &js, sizeof(struct js_event));

        /* joystick USB or bluetooth disconnected */
        if  (lReadStatus == -1)
        {
           close( joy_fd );
           return 0;
        }

	    if((js.type&JS_EVENT_INIT)==JS_EVENT_INIT)
        {
		    printf("type:%2.2x\n",js.type);
        }
        else
        {
        	printf("type:%2.2x\n",js.type);
            /* see what to do with the event */
            switch (js.type & ~JS_EVENT_INIT)
            {
                case JS_EVENT_AXIS:
                    axis   [ js.number ] = js.value;
                    break;
                case JS_EVENT_BUTTON:
                    button [ js.number ] = js.value;
                    break;
            }

            /* print the results */
            printf( "X: %6d  Y: %6d  ", axis[0], axis[1] );

            if( num_of_axis > 2 )
                printf("Z: %6d  ", axis[2] );

            if( num_of_axis > 3 )
                printf("R: %6d  ", axis[3] );

            for( x=0 ; x<num_of_buttons ; ++x )
                printf("B%d: %d  ", x, button[x] );

#if 0
            if(button[0] == 1)
            {
            	system("/usr/bin/espeak -v mb/mb-fr1 -f /media/linaro/GAUTIER/parameters/exemple0.txt");
            }

            if(button[1] == 1)
            {
            	system("/usr/bin/espeak -v mb/mb-fr1 -f /media/linaro/GAUTIER/parameters/exemple1.txt");
//            	system("/usr/bin/cvlc /media/linaro/GAUTIER/parameters/sf_pet_12.mp3");
            }
            if(button[2] == 1)
            {
            	system("/usr/bin/espeak -v mb/mb-fr1 -f /media/linaro/GAUTIER/parameters/exemple2.txt");
            }
            if(button[3] == 1)
            {
            	system("/usr/bin/espeak -v mb/mb-fr1 -f /media/linaro/GAUTIER/parameters/exemple3.txt");
            }
#endif

            printf("  \r");
            fflush(stdout);
        }
    }
    close( joy_fd );        /* too bad we never get here */
    return 0;
}
