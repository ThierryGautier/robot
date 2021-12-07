#ifndef ENERGY_H_
#define ENERGY_H_
/* energy */

#define EGY_AVERAGE_FILTER_FIFO_SIZE 441 //44100 Hz => 441/44100 = 0.01 ms //TODO depend on frequency of the device

typedef struct
{
    FL64 fl64AverageEnergy;
    FL64 fl64SumEnergy;
    FL64 afl64EnergyFIFO[EGY_AVERAGE_FILTER_FIFO_SIZE];
    UI08 u8IndexFIFO;
}EGY_stCtxSignalEnergy;

/**************************************************************************************************************************************************/
/** calculate average of energy of analog signal                                                                                                  */
/**************************************************************************************************************************************************/
VOID EGY_InitCtxOfSignalEnergy(EGY_stCtxSignalEnergy* pxSignalEnergy);
FL64 EGY_fl64CalculateSignalEnergy(EGY_stCtxSignalEnergy* pxSignalEnergy, FL64 dSignal);


#endif /* ENERGY_H_ */
