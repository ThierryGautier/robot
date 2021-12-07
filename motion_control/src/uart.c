#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <unistd.h>


#include "stdtype.h"

static int fd=0;
static struct termios  termios_p;

static void PrintTermiosInfo(struct termios *ptermios)
{
    int i;
    printf("Termios information:\n");
    printf("- c_iflag:0x%x\n",ptermios->c_iflag);
    printf("- c_oflag:0x%x\n",ptermios->c_oflag);
    printf("- c_cflag:0x%x\n",ptermios->c_cflag);
    printf("- c_lflag:0x%x\n",ptermios->c_lflag);
    printf("- c_line :0x%x\n",ptermios->c_line);
    for(i=0;i<NCCS;i++)
        printf("- c_cc[%d]:0x%x ",i,ptermios->c_cc[i]);
    printf("\n");
    printf("- c_ispeed :0x%x\n",ptermios->c_ispeed);
    printf("- c_ospeed :0x%x\n",ptermios->c_ospeed);
}

// see https://www.cmrr.umn.edu/~strupp/serial.html#2_1
// see https://man7.org/linux/man-pages/man3/termios.3.html

BOOL UART_bOpenDevice (CHAR pcDeviceName[])
{
    int err,i;
    BOOL lbReturnValue = FALSE;

    printf("UART_bOpenDevice:%s\n",pcDeviceName);
    
    /* Open serial port */
    fd=open(pcDeviceName,O_RDWR);
    if (fd == -1 )
    {
        printf("not possible to open the device\n");
    }
    else
    {
        printf("the device is open:%d\n",fd);

        err = tcgetattr(fd, &termios_p);
        if(err!=0)
        {
            printf("tcgetattr -err: %x\n",err);
        }
        else
        {
            PrintTermiosInfo(&termios_p);

            /* mode RAW, pas de mode canonique, pas d'echo 
               see // see https://www.cmrr.umn.edu/~strupp/serial.html#2_1 raw mode */
            termios_p.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP| INLCR | IGNCR | ICRNL | IXON);
            termios_p.c_oflag &= ~OPOST;
            termios_p.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

            /* 8 bits de données, pas de parité */
            termios_p.c_cflag &= ~(PARENB | CSIZE);
            termios_p.c_cflag |= (CS8|CSTOPB);
 

            /* 1 caractère suffit */
            termios_p.c_cc[VMIN] = 1;

            /* pas de timeout */
            termios_p.c_cc[VTIME] = 0;

            /* On passe la vitesse à B115200 */
 //           err = cfsetospeed(&termios_p, B115200);
 //           err = cfsetospeed(&termios_p, B230400);
            err = cfsetospeed(&termios_p, B500000);
 
            if(err!=0) printf("cfsetospeed -err: %x\n",err);

            PrintTermiosInfo(&termios_p);
            
            /*mise à jour config */
            err = tcsetattr(fd, TCSANOW, &termios_p);
            if(err!=0) printf("tcsetattr -err: %x\n",err);

            /** no error */
            lbReturnValue = TRUE;
        }
    }
    return(lbReturnValue);
}

UI32 UART_s32GetRxData(UI08 *pu8RxBuffer, UI32 u32Size)
{
    BOOL bCharIsRecieved;
    SI32 s32NbBytesReveived;

    /**read u32Size bytes */
    s32NbBytesReveived = read(fd,(CHAR*)pu8RxBuffer,u32Size);
    return(s32NbBytesReveived);
}

UI32 UART_s32PutTxData(UI08* pu8TxBuffer, UI32 u32Size)
{
    SI32 s32Counter;
    /** write u32Size bytes */
    s32Counter = write(fd,(CHAR*)pu8TxBuffer,u32Size);
    return(s32Counter);
}

BOOL UART_bCloseDevice (void)
{
    /* close device and driver */
    if(fd!=0) close(fd);
    return TRUE;
}
