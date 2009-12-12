#include <stdlib.h>
#include <stdio.h>

#ifdef linux
#   include <termios.h>
#   include <unistd.h>
#   include <fcntl.h>
#else 
#   ifdef windows
#       include <windows.h>
#   endif
#endif

#include "serial.h"

#ifdef linux
/**********************************************************
 *
 * Termios stuff
 *
 *********************************************************/

static struct termios old_settings;

int open_serport(char *port, int baudrate)
{
  int p;
  struct termios new_settings;
  if ((p = open(port, O_RDWR | O_NOCTTY | O_NDELAY))==-1)
    {
      fprintf(stderr, "\nUnable to open port : %s\n", port);
      return -1;
    }

  // set reading to blocking
  fcntl(p, F_SETFL,0);

  tcgetattr(p, &old_settings);

  cfsetispeed(&new_settings, baudrate);
  new_settings.c_cflag = CS8|CLOCAL|CREAD;
  new_settings.c_oflag = 0;
  new_settings.c_lflag = 0;
  new_settings.c_iflag = IGNPAR;
  new_settings.c_cc[VMIN]=1;
  new_settings.c_cc[VTIME]=0;

  tcsetattr(p, TCSANOW, &new_settings);
  return p;
}

int close_serport(int port)
{
  tcsetattr(port, TCSAFLUSH, &old_settings);
  close(port);
  return 1;
}

#endif
#ifdef windows
HANDLE open_serport(char *port, int baudrate) {
    HANDLE hSerial = CreateFile(port, GENERIC_READ | GENERIC_WRITE,
            0,
            0,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0);
    if (hSerial == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "\nUnable to open port: %s\n", port);
        return hSerial;
    }
    
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "\nUnable to get serial port state: %s\n", port);
        return hSerial;
    }
    
    dcbSerialParams.BaudRate = baudrate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        fprintf(stderr, "\nUnable to set serial port state: %s\n", port);
        return hSerial;
    }
    
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 16000;
    timeouts.ReadTotalTimeoutConstant = 10000;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        fprintf(stderr, "\nUnable to set serial port timeouts: %s\n", port);
    }
    
    return hSerial;
}

void close_serport(HANDLE hport) {
    CloseHandle(hport);
}
#endif

int serial_read(
#ifdef linux
                int fileid,
#endif
#ifdef windows
                HANDLE handle,
#endif
                unsigned char *buffer,
                int nbytes) {
#ifdef linux
    return read(fileid, buffer, nbytes);
#endif    
#ifdef windows
    DWORD dwBytesRead = 0;
    
    if (!ReadFile(handle, buffer, nbytes, &dwBytesRead, NULL)) {
        return -1;
    }
    return (int)dwBytesRead;
#endif
}

int serial_write(
#ifdef linux
                int 
#else
#   ifdef windows
                HANDLE
#   endif  
#endif
                        handle, unsigned char* buffer, int nbytes) {
#ifdef linux
    return write(handle, buffer, nbytes);
#else 
#   ifdef windows
    DWORD dwWritten;
    if (!WriteFile(handle, buffer, nbytes, &dwWritten, NULL)) {
        return -1;
    }
    return (int)dwWritten;
#   endif
#endif
}
                
int check_serport(
#ifdef linux
    int
#else
#   ifdef windows
    HANDLE
#   endif
#endif
    serport) {

    return serport
#ifdef linux  
                    >=1
#else 
#   ifdef windows
                    != INVALID_HANDLE_VALUE
#   endif  
#endif
                    ;
}


int serial_getspeed(const char *baudstring) {
	int baud_result;

	if (baudstring != NULL) {
		switch (atoi(baudstring)) {
		case 110:
			baud_result = BR110;
			break;
		case 300:
			baud_result = BR300;
			break;
		case 600:
			baud_result = BR600;
			break;
		case 1200:
			baud_result = BR1200;
			break;
		case 2400:
			baud_result = BR2400;
			break;
		case 4800:
			baud_result = BR4800;
			break;
		case 9600:
			baud_result = BR9600;
			break;
		case 19200:
			baud_result = BR19200;
			break;
		case 38400:
			baud_result = BR38400;
			break;
		case 57600:
			baud_result = BR57600;
			break;
		case 115200:
			baud_result = BR115200;
			break;
		default:
			baud_result = -1;
		}	
	}

	return baud_result;
}

int get_baud_int(int baudrate)
{
  switch (baudrate)
    {
    case BR110:    return 110;
    case BR300:    return 300;
    case BR600:    return 600;
    case BR1200:   return 1200;
    case BR2400:   return 2400;
    case BR4800:   return 4800;
    case BR9600:   return 9600;
    case BR19200:  return 19200;
    case BR38400:  return 38400;
    case BR57600:  return 57600;
    case BR115200: return 115200;
    default:return -1;
    }
  return -1;
}

