#/*
#    FreeRTOS V6.0.5 - Copyright (C) 2010 Real Time Engineers Ltd.
#
#    ***************************************************************************
#    *                                                                         *
#    * If you are:                                                             *
#    *                                                                         *
#    *    + New to FreeRTOS,                                                   *
#    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
#    *    + Looking for basic training,                                        *
#    *    + Wanting to improve your FreeRTOS skills and productivity           *
#    *                                                                         *
#    * then take a look at the FreeRTOS eBook                                  *
#    *                                                                         *
#    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
#    *                  http://www.FreeRTOS.org/Documentation                  *
#    *                                                                         *
#    * A pdf reference manual is also available.  Both are usually delivered   *
#    * to your inbox within 20 minutes to two hours when purchased between 8am *
#    * and 8pm GMT (although please allow up to 24 hours in case of            *
#    * exceptional circumstances).  Thank you for your support!                *
#    *                                                                         *
#    ***************************************************************************
#
#    This file is part of the FreeRTOS distribution.
#
#    FreeRTOS is free software; you can redistribute it and/or modify it under
#    the terms of the GNU General Public License (version 2) as published by the
#    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
#    ***NOTE*** The exception to the GPL is included to allow you to distribute
#    a combined work that includes FreeRTOS without being obliged to provide the
#    source code for proprietary components outside of the FreeRTOS kernel.
#    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
#    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#    more details. You should have received a copy of the GNU General Public 
#    License and the FreeRTOS license exception along with FreeRTOS; if not it 
#    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
#    by writing to Richard Barry, contact details for whom are available on the
#    FreeRTOS WEB site.
#
#    1 tab == 4 spaces!
#
#    http://www.FreeRTOS.org - Documentation, latest information, license and
#    contact details.
#
#    http://www.SafeRTOS.com - A version that is certified for use in safety
#    critical systems.
#
#    http://www.OpenRTOS.com - Commercial support, development, porting,
#    licensing and training services.
#*/

include $(DEVKITARM)/base_rules

TARGET=rtosbot

LPC21ISP=~/src/lpc2000/lpc21isp -hex -term -control $(OUTPUT).hex /dev/tty.usbserial-A1000acS 230400 14746

USE_THUMB_MODE=YES
DEBUG=
OPTIM=-Os
RUN_MODE=RUN_FROM_ROM

WARNINGS=-Wall -Wextra -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare \
		-Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused

BUILD 		= build
SOURCES 	= source source/serial source/freertos source/freertos/portable/MemMang source/freertos/portable/GCC/ARM7_LPC2000 
INCLUDES 	= ../FreeRTOS/Source/include source

#
# CFLAGS common to both the THUMB and ARM mode builds
#
CFLAGS=$(WARNINGS) -D$(RUN_MODE) -DGCC_ARM7 $(INCLUDE) \
		 $(DEBUG) -mcpu=arm7tdmi -T$(LDSCRIPT) \
		 $(OPTIM) -fomit-frame-pointer -fno-strict-aliasing -fno-dwarf2-cfi-asm

ifeq ($(USE_THUMB_MODE),YES)
	CFLAGS += -mthumb-interwork -DTHUMB_INTERWORK
	THUMB_FLAGS=-mthumb
endif


LINKER_FLAGS=-Xlinker -o$(OUTPUT).elf -Xlinker -M -Xlinker -Map=$(OUTPUT).map

RTOS_SOURCE_DIR=source/freertos


ifneq ($(BUILD),$(notdir $(CURDIR)))

export LDSCRIPT=$(CURDIR)/lpc2104-rom.ld
export MAKEFILE = $(CURDIR)/Makefile

export INCLUDE = $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
        $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
        -I$(CURDIR)/$(BUILD)

export OUTPUT   :=  $(CURDIR)/$(TARGET)
export VPATH    :=  $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
            $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR  :=  $(CURDIR)/$(BUILD)

export CRT0=$(CURDIR)/boot/boot.s

export AUTOSRC = $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))

export THUMB_SRC = $(AUTOSRC) 
#\
#	serial/serial.c \
#	$(RTOS_SOURCE_DIR)/tasks.c \
#	$(RTOS_SOURCE_DIR)/queue.c \
#	$(RTOS_SOURCE_DIR)/list.c \
#	$(RTOS_SOURCE_DIR)/portable/MemMang/heap_2.c \
#	$(RTOS_SOURCE_DIR)/portable/GCC/ARM7_LPC2000/port.c 

#
# Source files that must be built to ARM mode.
#
#ARM_SRC = \
#	$(RTOS_SOURCE_DIR)/portable/GCC/ARM7_LPC2000/portISR.c \
#	serial/serialISR.c

#
# Define all object files.
#
export ARM_OBJ = $(ARM_SRC:.c=.o)
export THUMB_OBJ = $(THUMB_SRC:.c=.o)
export OFILES = $(ARM_OBJ) $(THUMB_OBJ)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@#@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@make -C $(BUILD) -f $(MAKEFILE)

all:	$(BUILD)

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).{hex,map,elf}

burn:	$(TARGET).hex
	$(LPC21ISP)

else

#DEPENDS :=  $(OFILES:.o=.d)
#-include $(DEPENDS)

endif


$(OUTPUT).hex : $(OUTPUT).elf
	$(OBJCOPY) $(OUTPUT).elf -O ihex $(OUTPUT).hex

$(OUTPUT).elf : $(OFILES) $(CRT0) $(MAKEFILE)
	$(CC) $(CFLAGS) $(OFILES) -nostartfiles $(CRT0) $(LINKER_FLAGS)

#$(OFILES):	%.o : %.c %(LDSCRIPT) %(MAKEFILE)

%.o:    %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -mthumb -c $< -o $@ $(ERROR_FILTER)







	


