

#include <stdlib.h> // For malloc/free
#include <string.h> // For memset

#include "stdtype.h"
#include "PassBandFilter.h"

#if 0
FL64 filter1_coefficients[20] =
{
// Scaled for floating point

    0.754636267903902, -1.509272535807804, 0.754636267903902, 1.9866588870556963, -0.9867836450957207,// b0, b1, b2, a1, a2
    1, -2, 1, 1.9962721032895083, -0.9963428369125584,// b0, b1, b2, a1, a2
    0.0001220703125, 0.000244140625, 0.0001220703125, 1.9748619915812424, -0.9753003006823227,// b0, b1, b2, a1, a2
    0.0001220703125, 0.000244140625, 0.0001220703125, 1.9870876560160382, -0.987869339822201// b0, b1, b2, a1, a2

};

//coef pass band 4 eme ordre start frequency = 0.02 end frequency = 0.04762
FL64 filter1_coefficients[20] =
{
// Scaled for floating point

    0.7474917547306749, -1.4949835094613497, 0.7474917547306749, 1.852550345808976, -0.8768323998456592,// b0, b1, b2, a1, a2
    0.5, -1, 0.5, 1.943615459461937, -0.9599088357601986,// b0, b1, b2, a1, a2
    0.015625, 0.03125, 0.015625, 1.7768970630881846, -0.8266664652325348,// b0, b1, b2, a1, a2
    0.0078125, 0.015625, 0.0078125, 1.8319535472187678, -0.9126264372964495// b0, b1, b2, a1, a2

};

//coef pass band 4 eme ordre start frequency = 0.02 end frequency = 0.08, OK signal atténué voie est ok
FL64 filter1_coefficients[20] =
{
// Scaled for floating point

    0.7947290615931687, -1.5894581231863374, 0.7947290615931687, 1.7700087760523282, -0.7970511807643551,// b0, b1, b2, a1, a2
    0.5, -1, 0.5, 1.924789701533293, -0.941041944034579,// b0, b1, b2, a1, a2
    0.0625, 0.125, 0.0625, 1.5163480024434135, -0.6226099930957523,// b0, b1, b2, a1, a2
    0.03125, 0.0625, 0.03125, 1.5951279533548002, -0.8025420051533357// b0, b1, b2, a1, a2

};

#endif
//coef pass band 4 eme ordre start frequency = 0.01 end frequency = 0.08, OK signal atténué voie est ok
FL64 filter1_coefficients[20] =
{
// Scaled for floating point

    0.725148784326529, -1.450297568653058, 0.725148784326529, 1.8859590515953104, -0.8931043991107828,// b0, b1, b2, a1, a2
    1, -2, 1, 1.9659944388186212, -0.9701233193914915,// b0, b1, b2, a1, a2
    0.015625, 0.03125, 0.015625, 1.759640019404347, -0.7892231662547744,// b0, b1, b2, a1, a2
    0.0078125, 0.015625, 0.0078125, 1.83681569460255, -0.8928560205995031// b0, b1, b2, a1, a2

};



filter1Type *filter1_create( void )
{
    filter1Type *result = (filter1Type *)malloc( sizeof( filter1Type ) );   // Allocate memory for the object
    filter1_init( result );                                         // Initialize it
    return result;                                                              // Return the result
}

void filter1_destroy( filter1Type *pObject )
{
    free( pObject );
}

 void filter1_init( filter1Type * pThis )
{
    filter1_reset( pThis );
}

 void filter1_reset( filter1Type * pThis )
{
    memset( &pThis->state, 0, sizeof( pThis->state ) ); // Reset state to 0
    pThis->output = 0;                                  // Reset output

}

 int filter1_filterBlock( filter1Type * pThis, FL64 * pInput, FL64 * pOutput, unsigned int count )
{
    filter1_executionState executionState;          // The executionState structure holds call data, minimizing stack reads and writes
    if( ! count ) return 0;                         // If there are no input samples, return immediately
    executionState.pInput = pInput;                 // Pointers to the input and output buffers that each call to filterBiquad() will use
    executionState.pOutput = pOutput;               // - pInput and pOutput can be equal, allowing reuse of the same memory.
    executionState.count = count;                   // The number of samples to be processed
    executionState.pState = pThis->state;                   // Pointer to the biquad's internal state and coefficients.
    executionState.pCoefficients = filter1_coefficients;    // Each call to filterBiquad() will advance pState and pCoefficients to the next biquad

    // The 1st call to filter1_filterBiquad() reads from the caller supplied input buffer and writes to the output buffer.
    // The remaining calls to filterBiquad() recycle the same output buffer, so that multiple intermediate buffers are not required.

    filter1_filterBiquad( &executionState );        // Run biquad #0
    executionState.pInput = executionState.pOutput;         // The remaining biquads will now re-use the same output buffer.

    filter1_filterBiquad( &executionState );        // Run biquad #1

    filter1_filterBiquad( &executionState );        // Run biquad #2

    filter1_filterBiquad( &executionState );        // Run biquad #3

    // At this point, the caller-supplied output buffer will contain the filtered samples and the input buffer will contain the unmodified input samples.
    return count;       // Return the number of samples processed, the same as the number of input samples

}

 void filter1_filterBiquad( filter1_executionState * pExecState )
{
    // Read state variables
    FL64 w0, x0;
    FL64 w1 = pExecState->pState[0];
    FL64 w2 = pExecState->pState[1];

    // Read coefficients into work registers
    FL64 b0 = *(pExecState->pCoefficients++);
    FL64 b1 = *(pExecState->pCoefficients++);
    FL64 b2 = *(pExecState->pCoefficients++);
    FL64 a1 = *(pExecState->pCoefficients++);
    FL64 a2 = *(pExecState->pCoefficients++);

    // Read source and target pointers
    FL64 *pInput  = pExecState->pInput;
    FL64 *pOutput = pExecState->pOutput;
    short count = pExecState->count;
    FL64 accumulator;

    // Loop for all samples in the input buffer
    while( count-- )
    {
        // Read input sample
        x0 = *(pInput++);

        // Run feedback part of filter
        accumulator  = w2 * a2;
        accumulator += w1 * a1;
        accumulator += x0 ;

        w0 = accumulator ;

        // Run feedforward part of filter
        accumulator  = w0 * b0;
        accumulator += w1 * b1;
        accumulator += w2 * b2;

        w2 = w1;        // Shuffle history buffer
        w1 = w0;

        // Write output
        *(pOutput++) = accumulator ;
    }

    // Write state variables
    *(pExecState->pState++) = w1;
    *(pExecState->pState++) = w2;
}
