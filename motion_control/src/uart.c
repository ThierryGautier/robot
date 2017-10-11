#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "stdtype.h"
//warning printf reduce reactivity of the comunication
//#define DEBUG_UART
//#define DEBUG_UART
#ifdef DEBUG_UART
UI08 gu8Mode = 1;
#endif
static 	int	fd;

BOOL UART_bOpenDevice (char pcDeviceName[])
{
  struct termios	termios_p;
  /* Ouverture de la liaison serie ex"/dev/rfcomm1" */
  fd=open(pcDeviceName,O_RDWR);
  if (fd == -1 )
  {
    printf("not possible to open the device\n");
    return(FALSE);
  }
  else
  {
    printf("the device is open\n");
  }

  tcgetattr(fd, &termios_p);

  /* mode RAW, pas de mode canonique, pas d'echo */
  termios_p.c_iflag = IGNBRK;
  termios_p.c_lflag = 0;
  termios_p.c_oflag = 0;

  /* 1 caractère suffit */
  termios_p.c_cc[VMIN] = 1;

  /* Donnée disponible immédiatement */
  termios_p.c_cc[VTIME] = 0;

  /* Inhibe le controle de flux XON/XOFF */
  termios_p.c_iflag &= ~(IXON|IXOFF|IXANY);

  /* 8 bits de données, pas de parité */
  termios_p.c_cflag &= ~(PARENB | CSIZE);
  termios_p.c_cflag |= CS8;

  /* Gestion des signaux modem */
  termios_p.c_cflag &= ~CLOCAL;

  /*mise à jour config */
  tcsetattr(fd, TCSANOW, &termios_p);
  /** no error */
  return TRUE;
}

BOOL UART_bGetRxChar(UI08 *pu8RxChar)
{
  BOOL bCharIsRecieved;
  UI08 u8Counter;

  /**read one byte */
  u8Counter = read(fd,pu8RxChar,1);
#ifdef DEBUG_UART
  if(gu8Mode == 1)
  {
    printf("%2.2x ",*pu8RxChar);
  }
  else
  {
    printf("\n");
    printf("Bytes Recevied:%2.2x ",*pu8RxChar);
    gu8Mode = 1;
  }

#endif
  if(u8Counter == 1)
    {
      /** no error */
      bCharIsRecieved = TRUE;
    }
    else
    {
      /** error */
      bCharIsRecieved = FALSE;
    }
    return(bCharIsRecieved);
}

BOOL UART_bPutTxChar(UI08 u8TxChar)
{
    BOOL bCharIsSent;
    UI08 u8Counter;

    /** write one byte */
    u8Counter = write(fd,&u8TxChar,sizeof(u8TxChar));
#ifdef DEBUG_UART
    if(gu8Mode == 2)
    {
      printf("%2.2x ",u8TxChar);
    }
    else
    {
      printf("\n");
      printf("Bytes sent:%2.2x ",u8TxChar);
      gu8Mode = 2;
    }
#endif
    if(u8Counter == 1)
    {
        /** fflush */
       // fdatasync(fd);
        /** no error */
        bCharIsSent = TRUE;
    }
    else
    {
        /** no error */
        bCharIsSent = FALSE;
    }
    return(bCharIsSent);
}

BOOL UART_bCloseDevice (void)
{
  /* close device and driver */
  close(fd);
  return TRUE;
}
