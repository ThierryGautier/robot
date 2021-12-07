
#ifndef STDTYPE_H_
#define STDTYPE_H_

/** list of type required to compile the hdlc.c */
#define TRUE  (BOOL)0x01U
#define FALSE (BOOL)0x00U

/* list of logical operator */
#define OR ||
#define AND &&

/* list of std type */
typedef unsigned char       BOOL;

typedef void                VOID;

typedef char                CHAR;

typedef unsigned char       UI08;
typedef unsigned short      UI16;
typedef unsigned long       UI32;
typedef unsigned long long  UI64;

typedef signed char         SI08;
typedef signed short        SI16;
typedef signed long         SI32;
typedef signed long long    SI64;

typedef float               FL32;
typedef double              FL64;



/* generic macro */
#define SIZE_OF_ARRAY(array) (sizeof(array)/sizeof(array[0]))

#endif /* STDTYPE_H_ */
