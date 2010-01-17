TARGET  = mega8
#TARGET  = mega168

PRG	= $(TARGET)_bootload
OBJ	= mega_bootload.o flash.o serial.o

CFLAGS	= -Wall -D $(TARGET) -D linux

ifeq ($(DEBUG), 1)
  CFLAGS+=-D DEBUG
endif

ifeq ($(DEBUG), 2)
  CFLAGS+=-D DEBUG2
endif


##########################################################

all: $(PRG)

$(PRG) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ 


clean:
	rm -f mega8_bootload mega168_bootload *.o *~
