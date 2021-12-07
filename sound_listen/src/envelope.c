
/* system include */
#include <stdio.h>
#include <stdlib.h>

/* project include */
#include "stdtype.h"
#include "envelope.h"


/**************************************************************************************************************************************************/
/** calculate envelope of analog signal                                                                                                          */
/**************************************************************************************************************************************************/
VOID EVP_InitCtxOfSignalEnvelope(EVP_stCtxSignalEnvelope* pxSignalEnvelope)
{
    pxSignalEnvelope->fl64AverageEnvelope = 0.0;
    for(int i=0;i<EVP_AVERAGE_FILTER_FIFO_SIZE;i++)
    {
        pxSignalEnvelope->afl64EnvelopeFIFO[i]=0.0;
    }
    pxSignalEnvelope->u8IndexFIFO = 0;
}

FL64 EVP_fl64CalculateSignalEnvelope(EVP_stCtxSignalEnvelope* pxSignalEnvelope, FL64 f64Signal)
{

    FL64 f64Envelope ;

    //take the abslolute value
    if(f64Signal >= 0)
        f64Envelope= f64Signal;
    else
        f64Envelope= -f64Signal;

    //calculate sum Envelope
    pxSignalEnvelope->fl64SumEnvelope += f64Envelope -
                                         (pxSignalEnvelope->afl64EnvelopeFIFO[pxSignalEnvelope->u8IndexFIFO]);

    //calculate average Envelope
    pxSignalEnvelope->fl64AverageEnvelope = pxSignalEnvelope->fl64SumEnvelope/EVP_AVERAGE_FILTER_FIFO_SIZE;

    //update FIFO
    pxSignalEnvelope->afl64EnvelopeFIFO[pxSignalEnvelope->u8IndexFIFO] = f64Envelope;

    //update u8IndexFIFO
    if(pxSignalEnvelope->u8IndexFIFO >= EVP_AVERAGE_FILTER_FIFO_SIZE)
        pxSignalEnvelope->u8IndexFIFO = 0;
    else
        pxSignalEnvelope->u8IndexFIFO +=1;

    return(pxSignalEnvelope->fl64AverageEnvelope);
}
