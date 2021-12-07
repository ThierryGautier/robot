#ifndef SOUND_CAPTURE_H_
#define SOUND_CAPTURE_H_

SI32 SC_Initialize (CHAR pcDeviceName[],UI32* pu32MaxFrequency);
VOID SC_vGetSignal(SI16* s16ReadSignal, UI32 u32NbElement);
VOID SC_Release(VOID);

#endif /* SOUND_CAPTURE_H_ */
