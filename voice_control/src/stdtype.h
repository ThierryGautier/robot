
#ifndef CONTROL_PICO2WAVE_SRC_STDTYPE_H_
#define CONTROL_PICO2WAVE_SRC_STDTYPE_H_

/** list of type required to compile the hdlc.c */
typedef unsigned char  BOOL;
#define TRUE  (BOOL)0x1U
#define FALSE (BOOL)0x0U

#define AND &&
#define OR ||

typedef unsigned char  UI08;
typedef unsigned short UI16;
typedef unsigned long  UI32;

typedef signed char  SI08;
typedef signed short SI16;
typedef signed long  SI32;

typedef float        FL32;
typedef double       FL64;


#endif /* CONTROL_PICO2WAVE_SRC_STDTYPE_H_ */
