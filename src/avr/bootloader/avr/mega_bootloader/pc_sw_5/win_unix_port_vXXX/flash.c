#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mega_bootload.h"
#include "flash.h"

/**********************************************************
 *
 * Read binary page and create page array
 *
 *********************************************************/

flash_page *read_bin_file(int *n_pages, char *filename)
{
  FILE *binfile;
  struct stat fileinfo;
  unsigned char *binbuf;
  flash_page *flash_pages;
  int i, nbytes, npages, pageno;
  
  if ((binfile = fopen(filename, "rb")) == NULL)
    {
      fprintf(stderr, "\nUnable to open %s\n", filename);
      *n_pages = -1;
      return NULL;
    }

  /* Get filesize in bytes */
  stat(filename, &fileinfo);
  nbytes = fileinfo.st_size;
  if(nbytes==0)
    {
      fprintf(stderr, "\n%s seems to be empty ... \n", filename);
      fclose(binfile);
      *n_pages = -2;
      return NULL;
    }
  /* Get number of PAGESIZE sized pages + "leftovers" ...*/
  npages = nbytes/PAGESIZE;
  if((nbytes%PAGESIZE)>0)
    {
      npages++;
    }

  /* Read entire file into binbuf */
  binbuf=(unsigned char*)malloc(npages*PAGESIZE);
  memset(binbuf, 0xFF, npages*PAGESIZE);
  fread(binbuf, nbytes, 1, binfile);
  fclose(binfile);

  /* Copy binbuf in PAGESIZE chunks into flash_pages. */
  /* And set page number & flash address ... */
  flash_pages = malloc(npages*sizeof(flash_page));

  for(i=0; i<npages; i++)
    {
      flash_pages[i].page_number=i;
      flash_pages[i].flash_address=i*PAGESIZE;
      memcpy(flash_pages[i].data, binbuf+(i*PAGESIZE), PAGESIZE);
    }
  free(binbuf);

  /* Find empty pages ... ie. pages with only 0xFF*/
  for(pageno=0; pageno<npages; pageno++)
    {
      flash_pages[pageno].empty = YES;
      for(i=0; i<PAGESIZE; i++)
        {
          if(flash_pages[pageno].data[i]!=0xFF)
            {
              flash_pages[pageno].empty = NO;
              i = PAGESIZE;
            }
        }
    }
  *n_pages = npages;
  return flash_pages;
}

