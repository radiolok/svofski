/**************************************************************************
 *
 * Bootloader "front end" SW
 * Splits a binary file (avr-objcopy -O binary) into 'flash_page' structs, 
 * and sends them to the AVR.
 * Lots of hardcoded stuff, long TBD/TODO list and so on ...
 * But it works ...
 *
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef windows
#   include <windows.h>
#endif


#include "mega_bootload.h"
#include "serial.h"
#include "flash.h"

char id_string[] = "\nJussishow bootloader... version XXX\nBuild date: "__DATE__ "  "__TIME__"\n";

#if (!defined(mega8) && !defined(mega168) && !defined(mega644))
#   error Invalid or missing target
#endif

#ifdef mega8
  char target_string[] = "ATMega8";
#elif defined mega168
  char target_string[] = "ATMega168";
#elif defined mega644
  char target_string[] = "ATMega644";
#endif



int main(int argc, char *argv[])
{
  char *serial_device = DEFAULT_DEVICE;
  int   serial_speed  = DEFAULT_SPEED;

  int byteno, npages, pageno;
#ifdef linux  
  int serport;
#else 
#   ifdef windows
  HANDLE serport;
#   endif  
#endif

  flash_page *flash_pages;
  
  unsigned char cin, cout, errtype, QUIT;
  printf("%s", id_string);
  printf("\nBuilt for %s Pagesize %d bytes\n", target_string, PAGESIZE);
  if(argc<2)
    {
      fprintf(stderr, "\nUsage: %s <firmware.bin> [<serial_port> [<baudrate>]] \n", *argv);
      fprintf(stderr, "Default serial port: %s\n", serial_device);
      fprintf(stderr, "Default baud rate:   %d\n\n", get_baud_int(serial_speed));
      return -1;
    }
  flash_pages = read_bin_file(&npages, argv[1]);
  
  /*
    At his stage we've got npages flash_page structs in the flash_pages array.
  */

  /* Find out serial port parameters */
  if (argc > 2) {
		serial_device = argv[2];
  }
  if (argc > 3) {
  		serial_speed = serial_getspeed(argv[3]);
  }

  if (serial_speed == -1) {
  	fprintf(stderr, "Illegal serial port speed ...\n");
  	return -1;
  }

  printf("Using %s at %d baud.\n", serial_device, get_baud_int(serial_speed));
  
  /*
    Start communicating with AVR ...
  */
  serport = open_serport(serial_device, serial_speed);

  printf("Opened %s\n", serial_device);

  if (check_serport(serport))
    {

      QUIT = TRUE;
      errtype = 0;

      /* AVR sends 'O' to tell PC that we're alive ... */
      /* Anything else --> Quit ... */

      printf("\nWaiting for AVR...\n");
      fflush(stdout);

      serial_read(serport, &cin, 1);
      if(cin == 'O')
        {
          printf("\nOk here we go ...\n");
          fflush(stdout);
          /* Send 'S' to start the operation ...*/
          cout = 'S';
          serial_write(serport, &cout, 1);
          QUIT = FALSE;
        }
      else
        {
          printf("\nNot this time ...\n");
          fflush(stdout);
          QUIT = TRUE;
        }


      /*
        "Main loop"
        Read flash_pages, send address + data for pages that are not Empty
        "Protocol" Send byte -> Receive byte, if not the same -> Quit ...
        After last page, send address 0xFF -> AVR "gets the point" and jumps to 0x00 ...
        Variables ...:
        ----------------------------------------------------------------
        |  serport: file descriptor for serial port ...
        |  npages : Number of pagestructs in flash_pages
        |  pageno : Keeps track of current page
        |  byteno : Keeps track of current byte within page (PAGESIZE ...)
        |  cin, cout : bytes to/from AVR
        ----------------------------------------------------------------
      */


      for(pageno=0; pageno<npages && !QUIT; pageno++)
        {
          /* Check if page contains data */
          if((flash_pages[pageno].empty == NO) && !QUIT)
            {

              /* Send 'P' and wait for the AVR to reply with 'p' to start page transfer
                 Gotto come up with something better than that,
                 but, that'll have to do for now ... flow control ... protocol ...
              */
              cout = 'P';
              serial_write(serport, &cout, 1);
#ifdef DEBUG
              printf("\nWating for 'clear to transfer page' from AVR...\n");
              fflush(stdout);
#endif
              cin = 0;
              while(cin != 'p')
                {
                  serial_read(serport, &cin, 1);
                }
          
              
              /* Send & receive flash address */
              if(!QUIT)
                {
                  /* high byte of address */
                  cout = (flash_pages[pageno].flash_address & 0xFF00)>>8;
                  serial_write(serport, &cout, 1);
                  serial_read(serport, &cin, 1);
#ifdef DEBUG
                  printf("\nAddress:");
                  printf("\nSent:%02x Rec.:%02x", cout, cin);
                  fflush(stdout);
#endif
                  if(cin!=cout)
                    QUIT = TRUE;
                }
              if(!QUIT)
                {
                  /* low byte of address */
                  cout = flash_pages[pageno].flash_address & 0x00FF;
                  serial_write(serport, &cout, 1);
                  serial_read(serport, &cin, 1);
#ifdef DEBUG
                  printf("\nSent:%02x Rec.:%02x\nData:", cout, cin);
                  fflush(stdout);
#endif
                  if(cin!=cout)
                    QUIT = TRUE;
                }
              for(byteno=0; byteno<PAGESIZE && !QUIT; byteno++)
                {
                  /* send & receive databytes */
                  cout = flash_pages[pageno].data[byteno];
                  serial_write(serport, &cout, 1);
                  serial_read(serport, &cin, 1);
#ifdef DEBUG
                  printf("\nSent: %d %02x Rec.:%02x", byteno, cout, cin);
                  fflush(stdout);
#endif
                  if(cin!=cout)
                    QUIT = TRUE;
                }
            }

        }

      if (QUIT == FALSE)
        {
          /* Send 0xFF to tell AVR that all pages have been transferred */
          /* AVR receives 0xFF for flash_addr_h and jumps to 0x00 ... */

#ifdef DEBUG
          printf("\nWating for 'clear to transfer page' from AVR for the last time ...\n");
          fflush(stdout);
#endif
          cout = 'P';
          serial_write(serport, &cout, 1);
          cin = 0;
          while(cin != 'p')
            {
              serial_read(serport, &cin, 1);
            }
      
          cout = 0xFF;
          serial_write(serport, &cout, 1);
          serial_read(serport, &cin, 1);

          /* The AVR waits for a 'G' before it gets lost in code space ... */
          cout = 'G';
          serial_write(serport, &cout, 1);

          printf("\nTransfer complete ...\n");
          fflush(stdout);    
        }
      /*
        close serial port
      */
      close_serport(serport);
    }
  
  free(flash_pages);
  printf("\nDone ... \n");
  return 1;
}

