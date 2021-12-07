#ifndef ENVELOPE_H_
#define ENVELOPE_H_
/* Envelope */

#define EVP_AVERAGE_FILTER_FIFO_SIZE 441 //44100 Hz => 4410/44100 = 0.1 ms //TODO depend on frequency of the device

typedef struct
{
    FL64 fl64AverageEnvelope;
    FL64 fl64SumEnvelope;
    FL64 afl64EnvelopeFIFO[EVP_AVERAGE_FILTER_FIFO_SIZE];
    UI08 u8IndexFIFO;
}EVP_stCtxSignalEnvelope;

/**************************************************************************************************************************************************/
/** calculate average of Envelope of analog signal                                                                                                  */
/**************************************************************************************************************************************************/
VOID EVP_InitCtxOfSignalEnvelope(EVP_stCtxSignalEnvelope* pxSignalEnvelope);
FL64 EVP_fl64CalculateSignalEnvelope(EVP_stCtxSignalEnvelope* pxSignalEnvelope, FL64 dSignal);


#endif /* ENVELOPE_H_ */
