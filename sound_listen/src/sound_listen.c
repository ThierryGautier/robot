

/* system  include */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef SOUND_DEVICE
/* alsa include (required linux) */
#include <alsa/asoundlib.h>
#endif

/* project include */
#include "stdtype.h"
#include "energy.h"
#include "envelope.h"
#include "LMS.h"
#include "PassBandFilter.h"
#include "sound_capture.h"
#include "file_wav.h"

/* list of mode */
#define CMODE_ANALYSIS_WAV_FILE              (CHAR)1
#define CMODE_ANALYSIS_SOUND_FROM_DEVICE     (CHAR)2

/* list of state saved in the result wav file */
#define FL64_STATE_INIT            (FL64)-1.0f
#define FL64_STATE_NO_ENERGY       (FL64)-0.5f
#define FL64_STATE_LMS_IN_PROGRESS (FL64)+0.5f
#define FL64_STATE_LMS_OK          (FL64)+1.0f


VOID vPrintMenu(VOID)
{
    fprintf(stdout,"command line: -f <sound file name> only wav stereo 44100 file supported\n");
    fprintf(stdout,"- example: ./sound_listen -f toto.wav \n");
    fprintf(stdout,"command line: -d <sound device name> <duration in s>\n");
    fprintf(stdout,"- example: ./sound_listen -d hw:0,0 2\n ");
    fprintf(stdout,"           check path /proc/asound/card0\n");
    fprintf(stdout,"           cat stream0 return the description of device\n");

}

/**************************************************************************************************************************************************/
/** open wav file and apply LMS algo                                                                                                              */
/**************************************************************************************************************************************************/
int main (int argc, char *argv[])
{
    CHAR cMode;
    CHAR acWAVSourceFileName[512];
    CHAR acWAVResultFileName[512];
    CHAR acWAVSourceDeviceName[512];

    SI16 s16ReadSignalStereo[2] = {0,0};
    FL64 f64Write16OutputSignal[16] = {0,0,0,0,0,0,0,0,
                                       0,0,0,0,0,0,0,0};
    UI32 u32NbSample = 0;
    UI32 u32SampleIndex = 0;

    FL64 fl64InputSignal = 0;
    FL64 fl64InputSignalFiltered = 0;
    FL64 fl64Error;

    FL64 fl64SignalEnergy = 0;
    FL64 fl64ErrorEnergy = 0;
    FL64 fl64EnvelopeSignal = 0;
    FL64 fl64SignalLevel = 0;
    FL64 fl64ErrorLevel = 0;

    FL64 fl64State= FL64_STATE_INIT;
    UI32 ui32Tick = 0;

    EGY_stCtxSignalEnergy stInputSignalEnergy;
    EGY_stCtxSignalEnergy stLMSErrorlEnergy;
    EVP_stCtxSignalEnvelope stEnvelopeSignal;
    LMS_stCtxLMSFilter stLMSFilter;
    //filter1Type* stBandPassFilter;
    UI32 u32MaxFrequencyOfDevice = 0;

    FILE *pfDestinationSound = NULL;

    UI32 u32Length;
    SI32 s32NbSecond;
    WAV_stHeaderFile xHeaderFile;

    /* check arguments required */
    /* if -f option*/
    if(
       (argc==3) &&
       (strlen(argv[1]) == 2) &&  (strcmp(argv[1],"-f")==0) && // check "-f"
       (strlen(argv[2]) != 0) //check empty name
      )
    {
        //save the name of <sound file name>.wav file
        strcpy(acWAVSourceFileName,argv[2]);

        // initialize
        FW_Initialize(&xHeaderFile,acWAVSourceFileName);
        u32NbSample = FW_ui32GetNbSample(&xHeaderFile);

        //save the mode
        cMode = CMODE_ANALYSIS_WAV_FILE;
    }
    /* if -d option*/
    else if(
       (argc==4) &&
       (strlen(argv[1]) == 2) &&  (strcmp(argv[1],"-d")==0) && // check "-d"
       (strlen(argv[2]) != 0) && //check empty name
       (strlen(argv[3]) != 0)    //check empty number
      )
    {
        //save the name of <sound device name>
        strcpy(acWAVSourceDeviceName,argv[2]);

        //create the name of <sound file name>.wav file
        strcpy(acWAVSourceFileName,acWAVSourceDeviceName);
        u32Length = strlen(acWAVSourceDeviceName);
        strcpy(&acWAVSourceFileName[u32Length],".wav");
        printf("acWAVSourceFileName:%s\n",acWAVSourceFileName);

        // initialize sound device
        SC_Initialize (acWAVSourceDeviceName,&u32MaxFrequencyOfDevice);

        //save the number of samples per second
        if(1 == sscanf(argv[3],"%ld",&s32NbSecond)) u32NbSample = u32MaxFrequencyOfDevice*(UI32)s32NbSecond;

        //save the mode
        cMode = CMODE_ANALYSIS_SOUND_FROM_DEVICE;
    }
    else
    {
        vPrintMenu();
        exit(0);
    }

    //create the name of LMS<sound file name>.wav file result
    strcpy(acWAVResultFileName,"LMS");
    strcpy(&acWAVResultFileName[3],acWAVSourceFileName);
    printf("WAVResultFileName:%s\n",acWAVResultFileName);

    printf("number of sample:%ld\n",u32NbSample);

    /** open destination sound file */
    pfDestinationSound = fopen(acWAVResultFileName,"w++");
    if(pfDestinationSound == NULL)
    {
        fprintf(stderr,"Not able to open the destination wav file\n");
        return (0);
    }

    /* init struct in charge to calculate the input signal energy */
    EGY_InitCtxOfSignalEnergy(&stInputSignalEnergy);

    /* init struct in charge to calculate the error energy */
    EGY_InitCtxOfSignalEnergy(&stLMSErrorlEnergy);

    /* init struct in charge to calculate the envelope of the signal */
    EVP_InitCtxOfSignalEnvelope(&stEnvelopeSignal);

    /* init struct in charge to calculate the LMS filter coef */
    LMS_vInitCtxOfLMSFilter(&stLMSFilter);

    /* int Band pass filter */
    //stBandPassFilter = filter1_create();

    //calcul LMS for all points of the signal
    for (u32SampleIndex = 0; u32SampleIndex < u32NbSample; u32SampleIndex++)
    {
        /* stereo wav file */
        if(cMode == CMODE_ANALYSIS_WAV_FILE)
        {
            FW_vGetSignal(s16ReadSignalStereo,2);
        }
        /* mono device */
        else if( cMode == CMODE_ANALYSIS_SOUND_FROM_DEVICE)
        {
            SC_vGetSignal(s16ReadSignalStereo,1);
        }
        else
        {
            //do nothing
        }

        //fl64InputSignal new input sample for LMS filter
        fl64InputSignal = ((FL64)s16ReadSignalStereo[0])/32768U; // normalize to from 0.0 to 1.0 in FL64

        //pass band filter to only have voice signal
        //filter1_filterBlock( stBandPassFilter, &fl64InputSignal, &fl64InputSignalFiltered, 1);

        //calculate signal energy
        fl64SignalEnergy = EGY_fl64CalculateSignalEnergy(&stInputSignalEnergy, fl64InputSignal);

        //calculate signal level
        fl64SignalLevel = sqrt(fl64SignalEnergy);

        //calculate the envelope of the signal
        fl64EnvelopeSignal = EVP_fl64CalculateSignalEnvelope(&stEnvelopeSignal, fl64InputSignal);

        //calculate LMS filter
        fl64Error =  LMS_fl64CalculateLMSFilter(&stLMSFilter, fl64InputSignal);

        //calculate error energy
        fl64ErrorEnergy = EGY_fl64CalculateSignalEnergy(&stLMSErrorlEnergy, fl64Error);

        //calculate error level
        fl64ErrorLevel = sqrt(fl64ErrorEnergy);

        //calculate LMS filter if energy of the signal is too high soit 0.1 %
        if(fl64SignalLevel > 0.005)
        {
            //check if error is limited and the filter do his job
            if(fl64ErrorLevel < (0.5*fl64SignalLevel))
            {

                ui32Tick += 1;
                fl64State = FL64_STATE_LMS_OK;
            }
            else
            {
                //if previous state LMS OK and time in this state > 50 ms
                if(
                   (ui32Tick > 2205) &&
                   (fl64State == FL64_STATE_LMS_OK)
                   )
                {
                    LMS_vPrintLMSFilter(&stLMSFilter);
                }
                ui32Tick = 0;
                fl64State = FL64_STATE_LMS_IN_PROGRESS;
            }
        }
        //if energy of the signal is to low
        else
        {
            //if previous state LMS OK and time in this state > 50 ms
            if(
               (ui32Tick > 2205) &&
               (fl64State == FL64_STATE_LMS_OK)
               )
            {
                LMS_vPrintLMSFilter(&stLMSFilter);
            }
            ui32Tick = 0;
            fl64State = FL64_STATE_NO_ENERGY;

            //reset LMS filter
            LMS_vInitCtxOfLMSFilter(&stLMSFilter);
        }
        /**  update wav file used for analysis */
        f64Write16OutputSignal[ 0] = fl64InputSignal;
        f64Write16OutputSignal[ 1] = fl64InputSignalFiltered;
        f64Write16OutputSignal[ 2] = fl64SignalLevel;
        f64Write16OutputSignal[ 3] = fl64ErrorLevel;
        f64Write16OutputSignal[ 4] = fl64EnvelopeSignal;
        f64Write16OutputSignal[ 5] = fl64State;
        f64Write16OutputSignal[ 6] = 0;
        f64Write16OutputSignal[ 7] = 0;

        f64Write16OutputSignal[ 8] = LMS_fl64GetCoef(&stLMSFilter,0);
        f64Write16OutputSignal[ 9] = LMS_fl64GetCoef(&stLMSFilter,1);
        f64Write16OutputSignal[10] = LMS_fl64GetCoef(&stLMSFilter,2);
        f64Write16OutputSignal[11] = LMS_fl64GetCoef(&stLMSFilter,3);
        f64Write16OutputSignal[12] = LMS_fl64GetCoef(&stLMSFilter,4);
        f64Write16OutputSignal[13] = LMS_fl64GetCoef(&stLMSFilter,5);
        f64Write16OutputSignal[14] = LMS_fl64GetCoef(&stLMSFilter,6);
        f64Write16OutputSignal[15] = LMS_fl64GetCoef(&stLMSFilter,7);


        fwrite((void*)&f64Write16OutputSignal[0],sizeof(f64Write16OutputSignal),1,pfDestinationSound);

    }
    fclose(pfDestinationSound);
}

