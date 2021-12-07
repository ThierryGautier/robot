#ifndef LOW_MIN_SQUARE_H_
#define LOW_MIN_SQUARE_H_
/* LMS */

#define LMS_U8_ORDER_OF_FILTER 16              //order of filter

typedef struct
{
    FL64 afl64Filter[LMS_U8_ORDER_OF_FILTER];
    FL64 afl64SignalInput[LMS_U8_ORDER_OF_FILTER];
    FL64 fl64SignalOutput;
    FL64 fl64Error;
}LMS_stCtxLMSFilter;

VOID LMS_vInitCtxOfLMSFilter(LMS_stCtxLMSFilter* pxLMSFilter);
FL64 LMS_fl64CalculateLMSFilter(LMS_stCtxLMSFilter* pxLMSFilter, FL64 fl64Signal);
VOID LMS_vPrintLMSFilter(LMS_stCtxLMSFilter* pxLMSFilter);
FL64 LMS_fl64GetCoef(LMS_stCtxLMSFilter* pxLMSFilter, UI16 ui16Index );

#endif /* LOW_MIN_SQUARE_H_ */
