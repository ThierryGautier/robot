#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stdtype.h"

#include "file_wav.h"

FILE *pfSourceSound = NULL;

VOID FW_CreateHeaderFile(WAV_stHeaderFile* pHeader, UI32 u32NbSamples)
{
    const WAV_stHeaderFile xHeaderTemplate =
    {{
      // Declaration Area
    'R','I','F','F',     //acFileTypeBlocID
    0,0,0,0,             //u32FileSize
    'W','A','V','E',     //acFileFormatID
    // Audio Format
    'f','m','t',' ',    //acFormatBlocID
    0x10,0x00,0x00,0x00, //u32BlocSize
    0x01,0x00,           //u16AudioFormat
    0x02,0x00,           //u16NbrOfChannel
    0x44,0xAC,0x00,0x00, //u32Frequency
    0x10,0xB1,0x02,0x00, //u32BytePerSec
    0x04,0x00,           //u16BytePerBloc
    0x10,0x00,           //u16BitsPerSample
    // Data Block
    'd','a','t','a',     //acDataBlocID
    0x00,0x00,0x00,0x00, //u32DataSize
    }};

    UI32 u32FileSize = u32NbSamples*4+36;
    UI32 u32DataSize = u32NbSamples*4;

    memcpy(pHeader,xHeaderTemplate.u8Buffer,sizeof(WAV_stHeaderFile));

    pHeader->u8Buffer[ 4]= (u32FileSize & 0x000000FF);
    pHeader->u8Buffer[ 5]= (u32FileSize & 0x0000FF00)>>8;
    pHeader->u8Buffer[ 6]= (u32FileSize & 0x00FF0000)>>16;
    pHeader->u8Buffer[ 7]= (u32FileSize & 0xFF000000)>>24;

    pHeader->u8Buffer[40]= (u32DataSize & 0x000000FF);
    pHeader->u8Buffer[41]= (u32DataSize & 0x0000FF00)>>8;
    pHeader->u8Buffer[42]= (u32DataSize & 0x00FF0000)>>16;
    pHeader->u8Buffer[43]= (u32DataSize & 0xFF000000)>>24;

    printf("FW_CreateHeaderFile:%ld\n",u32NbSamples);
    for(int i=0;i<sizeof(WAV_stHeaderFile);i++)
    {
      if((i%8 == 0)&&(i!=0)) printf("\n");
      printf("0x%2.2x ",pHeader->u8Buffer[i]);
    }
    printf("\n");
}

SI32 FW_Initialize (WAV_stHeaderFile* pHeader,CHAR pcWAVSourceFileName[])
{
    SI32 s32Return=0;

    /** open source sound file */
    pfSourceSound = fopen(pcWAVSourceFileName,"r");
    if(pfSourceSound == NULL)
    {
        fprintf(stderr,"Not able to open the source wav file\n");
        return (0);
    }

    /** read the header of source wav file */
    s32Return = fread((void*)pHeader->u8Buffer,WAV_HEADER_SIZE_IN_BYTES,1,pfSourceSound);
    if(s32Return != 1)
    {
        fprintf(stderr,"%ld\n",s32Return);
        fprintf(stderr,"Not able to read the header of the wav file\n");
        return (0);
    }
    printf("FW_Initialize:\n");
    for(int i=0;i<sizeof(WAV_stHeaderFile);i++)
    {
        if((i%8 == 0)&&(i!=0)) printf("\n");
        printf("0x%2.2x ",pHeader->u8Buffer[i]);
    }
    printf("\n");
    return(s32Return);
}

UI32 FW_ui32GetNbSample(WAV_stHeaderFile* pHeader)
{
    UI32 u32NbSample;
    /** calculate the number of samples */
    u32NbSample =
        (((UI32)pHeader->u8Buffer[40]) << 0)|
        (((UI32)pHeader->u8Buffer[41]) << 8)|
        (((UI32)pHeader->u8Buffer[42]) << 16)|
        (((UI32)pHeader->u8Buffer[43]) << 24);

    u32NbSample = u32NbSample / 4; //header give the number of bytes stereo 16 bits

    return(u32NbSample);
}

VOID FW_vGetSignal(SI16* s16ReadSignal, UI32 u32NbElement)
{
    //get input signal
    fread((void*)s16ReadSignal,sizeof(SI16),u32NbElement,pfSourceSound);
}

VOID FW_Release(VOID)
{
    fclose(pfSourceSound);
}
