###############################################################################
# Makefile for the project numishnik
###############################################################################

## General Flags
PROJECT = numishnik

MCU = atmega8
#MCU = atmega16
#MCU = atmega48
#MCU = atmega88
#MCU = atmega168
#MCU = atmega328p

CLK = 12800000UL
#CLK = 12000000UL
#CLK = 15000000UL
#CLK = 16000000UL
#CLK = 18000000UL
#CLK = 20000000UL

TARGET = numishnik.elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU) -DF_CPU=$(CLK)

## UART_INVERT enables software-inverter (PC0 -|>o- PB0, PC1 -|>o- PB1)
## to connect to RS-232C line directly. ( <= 2400 bps )
## atmega8 doesn't support this
#COMMON += -DUART_INVERT

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -Os -fsigned-char
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp 

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += 


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I".." -I"../../usbdrv" -I"../../libs-device"

## Objects that must be built in order to link
SOURCES := ../../usbdrv ../../libs-device ..
CFILES :=  $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES :=  $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
export OFILES   := $(addsuffix .o,$(BINFILES)) $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.S=.o)

OFILES = usbdrv.o oddebug.o usbdrvasm.o osccal.o cdcuart.o main.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
#gruu:
#	echo $(OFILES)
#	echo $(OBJECTS)

all: $(TARGET) numishnik.hex numishnik.eep numishnik.lss size

## Compile
usbdrvasm.o: ../../usbdrv/usbdrvasm.S
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

usbdrv.o: ../../usbdrv/usbdrv.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

oddebug.o:	../../usbdrv/oddebug.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

osccal.o:	../../libs-device/osccal.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

cdcuart.o: ../cdcuart.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

main.o: ../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OFILES)
	 $(CC) $(LDFLAGS) $(OFILES) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OFILES) numishnik.elf dep/* numishnik.hex numishnik.eep numishnik.lss

## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

