/*
 ============================================================================
 Name        : bluetooth.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : communication with FRDM KV31 board through a bluetooth connection
               lunix command:  'sudo ./bluetooth /dev/rfcomm0'
               make sure that the /dev/rfcomm0 is enabled before using it
 ============================================================================
 */

/* system include */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

/* project include */
#include "stdtype.h"

/** commun project variables */
extern UI08 u8MotorCommand;
extern UI16 u16PWMLevel;
extern float f32CompassCommand;
extern float f32RxeCompass;


static float CalculateNewCap(float f32Cap,float f32DeltaCap)
{
	f32Cap = f32Cap+f32DeltaCap;
	if(f32Cap >= 360.0) f32Cap = f32Cap-360.0;
	else if(f32Cap < 0) f32Cap = f32Cap+360.0;

	return(f32Cap);
}

static UI16 CalculateNewPWM(UI16 u16PWM,SI16 s16DeltaPWM)
{
	//negative value and abs value greater than u16PWM
	if((s16DeltaPWM < 0) && ((-s16DeltaPWM)>u16PWM))
    {
		u16PWM = 0;
    }
	else
	{
		u16PWM = u16PWM+s16DeltaPWM;
		if(u16PWM >= 2499) u16PWM = 2499;
	}
	return(u16PWM);
}

/* reads from keypress, doesn't echo */
static int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

/** thread in charge to get key board order to control ROBOT */
void* ThreadKeyBoard(void *arg)
{

   printf("Press a command:\n");
   printf("-: reduce motor velocity:\n");
   printf("+: increase motor velocity:\n");
   printf("7: run forward and turn on the left:\n");
   printf("8: run forward:\n");
   printf("9: run forward and turn on the right:\n");
   printf("5: stop:\n");
   printf("1: run reverse and turn on the left:\n");
   printf("2: run reverse:\n");
   printf("3: run reverse and turn on the right:\n");
   while(1)
   {
		int intKeyboardChar;

		intKeyboardChar = getch();
		//printf("ch:%x \n",intKeyboardChar);
		switch(intKeyboardChar)
		{
       	    case '-':
    		    u16PWMLevel = CalculateNewPWM(u16PWMLevel,-10);
		        break;

        	case '+':
       		     u16PWMLevel = CalculateNewPWM(u16PWMLevel,10);
			     break;

        	case '0':
				 break;

			case '1':
				 f32CompassCommand = CalculateNewCap(f32CompassCommand,+10.0);
				 u8MotorCommand = 3;
				 break;

			case '2':
				 f32CompassCommand = f32RxeCompass;
				 u8MotorCommand = 3;
				 break;

			case '3':
				 f32CompassCommand = CalculateNewCap(f32CompassCommand,-10.0);
				 u8MotorCommand = 3;
				 break;

			case '4':
				 break;

			case '5':
				 u8MotorCommand = 0;
				 break;

			case '6':
				 break;

			case '7':
				 f32CompassCommand = CalculateNewCap(f32CompassCommand,-10.0);
				 u8MotorCommand = 1;
				 break;

			case '8':
				 f32CompassCommand = f32RxeCompass;
				 u8MotorCommand = 1;
				 break;

			case '9':
				 f32CompassCommand = CalculateNewCap(f32CompassCommand,+10.0);
				 u8MotorCommand = 1;
				 break;

			default:
				break;
	 	}
		printf("f32CompassCommand:%f u8MotorCommand:%d u16PWMLevel:%d\n",f32CompassCommand,u8MotorCommand,u16PWMLevel);
	}
}
