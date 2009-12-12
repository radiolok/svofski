#ifndef _FLASH_H
#define _FLASH_H

#include <inttypes.h>

#ifdef mega8
#   define PAGESIZE   64
#elif defined mega168
#   define PAGESIZE   128
#elif defined mega644
#   define PAGESIZE   256
#endif

typedef struct {
  int page_number, empty;
  uint16_t flash_address;
  unsigned char data[PAGESIZE];
} flash_page;

flash_page* read_bin_file(int *, char *);

#endif
