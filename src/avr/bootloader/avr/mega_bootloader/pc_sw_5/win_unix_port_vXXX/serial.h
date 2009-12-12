#ifndef _SERIAL_H
#define _SERIAL_H

#ifdef linux
#include <termios.h>
#endif

#ifdef windows
#define BR110     CBR_110     
#define BR300     CBR_300     
#define BR600     CBR_600     
#define BR1200    CBR_1200    
#define BR2400    CBR_2400    
#define BR4800    CBR_4800    
#define BR9600    CBR_9600    
#define BR19200   CBR_19200   
#define BR38400   CBR_38400   
#define BR57600   CBR_57600   
#define BR115200  CBR_115200  
#else
#ifdef linux
#define BR110     B110
#define BR300     B300
#define BR600     B600
#define BR1200    B1200
#define BR2400    B2400
#define BR4800    B4800
#define BR9600    B9600
#define BR19200   B19200
#define BR38400   B38400
#define BR57600   B57600
#define BR115200  B115200
#endif
#endif

#ifdef linux
int open_serport(char *port, int baudrate);
int close_serport(int port);
#endif
#ifdef windows
HANDLE open_serport(char *port, int baudrate);
void close_serport(HANDLE port);
#endif

int serial_read(
#ifdef linux
                int,
#endif
#ifdef windows
                HANDLE,
#endif
                unsigned char *,
                int);

int serial_write(
#ifdef linux
                int,
#endif
#ifdef windows
                HANDLE,
#endif
                unsigned char *,
                int);


int check_serport(
#ifdef linux
    int
#else
#   ifdef windows
    HANDLE
#   endif
#endif
    );    


int serial_getspeed(const char *);

int get_baud_int(int baudrate);

#endif
