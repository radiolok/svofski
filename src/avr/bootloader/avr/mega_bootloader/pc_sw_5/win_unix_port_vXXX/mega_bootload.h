#ifndef _MEGA_BOOTLOAD_H
#define _MEGA_BOOTLOAD_H

#ifdef windows
#	define DEFAULT_DEVICE "COM1"
#	define DEFAULT_SPEED  CBR_115200
#else
#	ifdef linux
#		define DEFAULT_DEVICE 	"/dev/ttyS2"
#		define DEFAULT_SPEED	B115200
#	endif
#endif


#define YES 1
#define NO 0
#define TRUE 1
#define FALSE 0

#endif
