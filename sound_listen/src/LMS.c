/*  see
    https://www.codeproject.com/Articles/1000084/Least-Mean-Square-Algorithm-using-Cplusplus */

/* system include */
#include <stdio.h>
#include <stdlib.h>

/* project include */
#include "stdtype.h"
#include "LMS.h"

const FL64 fl64ConvergenceRate = (double)0.05f; //convergence rate

/**************************************************************************************************************************************************/
/** calculate LMS of analog signal                                                                                                                */
/**************************************************************************************************************************************************/
VOID LMS_vInitCtxOfLMSFilter(LMS_stCtxLMSFilter* pxLMSFilter)
{
    //reset filter coefficients
    for (int i = 0; i < LMS_U8_ORDER_OF_FILTER; i++)
    {
        pxLMSFilter->afl64Filter[i] = 0.0;
        pxLMSFilter->afl64SignalInput[i] = 0.0;
    }
    pxLMSFilter->fl64Error = 0;
}

FL64 LMS_fl64CalculateLMSFilter(LMS_stCtxLMSFilter* pxLMSFilter, FL64 fl64Signal)
{
    /* fix output to 0 */
    pxLMSFilter->fl64SignalOutput = 0;

    //calculate filter output
    for (int i = 0; i < LMS_U8_ORDER_OF_FILTER; i++)
    {
        pxLMSFilter->fl64SignalOutput += (pxLMSFilter->afl64Filter[i] * pxLMSFilter->afl64SignalInput[i]);
    }

    //calculate the error between signal output (prediction) and real signal
    pxLMSFilter->fl64Error = fl64Signal - pxLMSFilter->fl64SignalOutput;

    //update filter coefficients
    for (int i = 0; i < LMS_U8_ORDER_OF_FILTER; i++)
    {
        pxLMSFilter->afl64Filter[i] = pxLMSFilter->afl64Filter[i] + (fl64ConvergenceRate * pxLMSFilter->fl64Error * pxLMSFilter->afl64SignalInput[i]);
    }

    //shift input signal on the left required for LMS
    for(int i=LMS_U8_ORDER_OF_FILTER-1;i>0;i--)
    {
        pxLMSFilter->afl64SignalInput[i] = pxLMSFilter->afl64SignalInput[i-1];
    }

    //save the signal t=0
    pxLMSFilter->afl64SignalInput[0] = fl64Signal;

    return(pxLMSFilter->fl64Error);
}

VOID LMS_vPrintLMSFilter(LMS_stCtxLMSFilter* pxLMSFilter)
{
    //print all coefs
    printf("\nLMS  filter coef:");
    for (int i = 0; i < LMS_U8_ORDER_OF_FILTER; i++)
    {
        printf("[%d]:%4.4f ",i,pxLMSFilter->afl64Filter[i]);
    }
}

FL64 LMS_fl64GetCoef(LMS_stCtxLMSFilter* pxLMSFilter, UI16 ui16Index )
{
    return(pxLMSFilter->afl64Filter[ui16Index]);
}

