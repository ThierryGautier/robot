/*
 ============================================================================
 Name        : control_espeak.c
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
#include <sys/stat.h>

#include "stdtype.h"

#define FIFO_CMD  "/tmp/VoiceControl.fifo" //FIFO used to exchange sound command


typedef struct
{
    UI08  u8EspeakCommand;  //from 0 to 255
}stEspeakCommand;

/* processus in charge to:
 * - wait an external comman reveived through a pipe
 * - generate the espeak command
 */
int main()
{
    FILE* pFileFIFIO;

    stEspeakCommand lstEspeakCmd = {0};

    system("aplay  /media/linaro/DATA/Hikey/Hikey/voice_control/parameter/voice0.wav &");

    /* infinite loop */
    while( 1 )
    {
        /*get voice command from other processus through FIFO*/
        /* open FIFO read only */
        pFileFIFIO = fopen(FIFO_CMD, "r");

        /* read if new command is received */
        fread((void*)&lstEspeakCmd, sizeof(stEspeakCommand),1, pFileFIFIO);
        printf("Received cmd - u8EspeakCommand:%d\n",lstEspeakCmd.u8EspeakCommand);
        fclose(pFileFIFIO);

        switch(lstEspeakCmd.u8EspeakCommand)
        {
            case   1:
                system("aplay  /media/linaro/DATA/Hikey/Hikey/voice_control/parameter/klaxon.wav &");
                break;
            case   2:
                system("aplay  /media/linaro/DATA/Hikey/Hikey/voice_control/parameter/vache.wav &");
                break;
            case   3:
                system("aplay  /media/linaro/DATA/Hikey/Hikey/voice_control/parameter/voiture.wav &");
                break;
            case   4:
                break;
            case   5:
                break;
            default:
                break;
        }
    }
    return 0;
}
