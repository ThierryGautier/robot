/* energy */

/* system include */
#include <stdio.h>
#include <stdlib.h>

/* project include */
#include "stdtype.h"
#include "energy.h"

/**************************************************************************************************************************************************/
/** calculate average of energy of analog signal                                                                                                  */
/**************************************************************************************************************************************************/
VOID EGY_InitCtxOfSignalEnergy(EGY_stCtxSignalEnergy* pxSignalEnergy)
{
    pxSignalEnergy->fl64AverageEnergy = 0.0;
    for(int i=0;i<EGY_AVERAGE_FILTER_FIFO_SIZE;i++)
    {
    	pxSignalEnergy->afl64EnergyFIFO[i]=0.0;
    }
    pxSignalEnergy->u8IndexFIFO = 0;
}

FL64 EGY_fl64CalculateSignalEnergy(EGY_stCtxSignalEnergy* pxSignalEnergy, FL64 f64Signal)
{
    FL64 f64Energy = f64Signal*f64Signal;
    //calculate sum energy
    pxSignalEnergy->fl64SumEnergy += f64Energy -
                                     (pxSignalEnergy->afl64EnergyFIFO[pxSignalEnergy->u8IndexFIFO]);

    //calculate average energy
    pxSignalEnergy->fl64AverageEnergy = pxSignalEnergy->fl64SumEnergy/EGY_AVERAGE_FILTER_FIFO_SIZE;

    //update FIFO
    pxSignalEnergy->afl64EnergyFIFO[pxSignalEnergy->u8IndexFIFO] = f64Energy;

    //update u8IndexFIFO
    if(pxSignalEnergy->u8IndexFIFO >= EGY_AVERAGE_FILTER_FIFO_SIZE)
        pxSignalEnergy->u8IndexFIFO = 0;
    else
        pxSignalEnergy->u8IndexFIFO +=1;

    return(pxSignalEnergy->fl64AverageEnergy);
}

