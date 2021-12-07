/* system include */
#include <stdio.h>
#include <stdlib.h>

#ifdef SOUND_DEVICE
#include <alsa/asoundlib.h>
#endif

/* project include */
#include "stdtype.h"
/* sound device env variables */

#ifdef SOUND_DEVICE
snd_pcm_t *capture_handle;
snd_pcm_hw_params_t *hw_params;
UI32 u32frequencyMin = 0;
UI32 u32frequencyMax = 0;
#endif

SI32 SC_Initialize (CHAR pcDeviceName[],UI32* pu32MaxFrequency)
{
#ifdef SOUND_DEVICE
  SI32 s32err;
  SI32 s32Return=0;

    /* configure input sound device */
    if ((s32err = snd_pcm_open (&capture_handle, pcDeviceName, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",pcDeviceName,snd_strerror (s32err));
        return (1);
    }

    if ((s32err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",snd_strerror (s32err));
        return (1);
    }

    if ((s32err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",snd_strerror (s32err));
        return (1);
    }

    if ((s32err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",snd_strerror (s32err));
        return (1);
    }

    if ((s32err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",snd_strerror (s32err));
        return (1);
    }

    /* get the frequency Min */
    if ((s32err = snd_pcm_hw_params_get_rate_min (hw_params, (unsigned int*)&u32frequencyMin, 0)) < 0) {
      fprintf (stderr, "cannot get min sample rate (%s)\n",snd_strerror (s32err));
      return (1);
    }

    /* get the frequency Max */
    if ((s32err = snd_pcm_hw_params_get_rate_max (hw_params, (unsigned int*)&u32frequencyMax, 0)) < 0) {
      fprintf (stderr, "cannot get max sample rate (%s)\n",snd_strerror (s32err));
      return (1);
    }
    *pu32MaxFrequency = u32frequencyMax;
    fprintf(stdout,"frequency range: min:%ld, max:%ld\n",u32frequencyMin,u32frequencyMax);

    /* update the frequency  and fix it to max value */
    if ((s32err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, (unsigned int*)&u32frequencyMax, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",snd_strerror (s32err));
        return (1);
    }

    /* mono device , one channels */
    if ((s32err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n", snd_strerror (s32err));
        return (1);
    }

    if ((s32err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n", snd_strerror (s32err));
        return (1);
    }
    snd_pcm_hw_params_free (hw_params);

    /* */
    if ((s32err = snd_pcm_prepare (capture_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (s32err));
        return (1);
    }
    return(s32Return);
#else
    return(0);
#endif
}

VOID SC_vGetSignal(SI16* s16ReadSignal, UI32 u32NbElement)
{
#ifdef SOUND_DEVICE
    SI32 s32err;
    //get input signal from the device
    s32err = snd_pcm_readi(capture_handle, s16ReadSignal,sizeof(SI16)*u32NbElement);
    if(s32err != sizeof(SI16))
    {
        fprintf (stderr, "read from audio interface failed (%s)\n",snd_strerror (s32err));
    }
//    else
//    {
//        fprintf(stdout,"value:%d\n",*s16ReadSignal );
//    }
#endif
}

VOID SC_Release(VOID)
{
#ifdef SOUND_DEVICE
    snd_pcm_close (capture_handle);
#endif
}

