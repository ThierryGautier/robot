

#ifndef PASSBANDFILTER_H_
#define PASSBANDFILTER_H_

static const int filter1_numStages = 4;
static const int filter1_coefficientLength = 20;
extern FL64 filter1_coefficients[20];

typedef struct
{
	FL64 state[16];
	FL64 output;
} filter1Type;

typedef struct
{
	FL64 *pInput;
	FL64 *pOutput;
	FL64 *pState;
	FL64 *pCoefficients;
	short count;
} filter1_executionState;

filter1Type *filter1_create( void );
void filter1_destroy( filter1Type *pObject );
void filter1_init( filter1Type * pThis );
void filter1_reset( filter1Type * pThis );

#define filter1_writeInput( pThis, input )  \
	filter1_filterBlock( pThis, &(input), &(pThis)->output, 1 );

#define filter1_readOutput( pThis )  \
	(pThis)->output

int filter1_filterBlock( filter1Type * pThis, FL64 * pInput, FL64 * pOutput, unsigned int count );

#define filter1_outputToFloat( output )  \
	(output)

#define filter1_inputFromFloat( input )  \
	(input)

 void filter1_filterBiquad( filter1_executionState * pExecState );

 #endif /* PASSBANDFILTER_H_ */
	
