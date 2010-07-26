include $(DEVKITARM)/base_rules

TARGET=rtosbot

USE_THUMB_MODE=YES
DEBUG=
OPTIM=-Os
RUN_MODE=RUN_FROM_ROM
LPC21ISP=~/src/lpc2000/lpc21isp -hex -term -control $(OUTPUT).hex /dev/tty.usbserial-A1000acS 230400 14746

WARNINGSCXX=-Wall -Wextra -Wshadow -Wpointer-arith -Wcast-align -Wsign-compare \
		-Waggregate-return -Wmissing-declarations -Wunused

WARNINGSC=-Wall -Wextra -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare \
		-Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused

BUILD 		= build
SOURCES 	= source \
    source/serial \
    source/lcd \
    source/freertos \
    source/freertos/portable/MemMang \
    source/freertos/portable/GCC/ARM7_LPC2000

INCLUDES 	= ../FreeRTOS/Source/include source source/lcd

#
# CFLAGS common to both the THUMB and ARM mode builds
#
CBASE=-D$(RUN_MODE) -DGCC_ARM7 $(INCLUDE) \
		 $(DEBUG) -mcpu=arm7tdmi -T$(LDSCRIPT) \
		 $(OPTIM) -fomit-frame-pointer -fno-strict-aliasing -fno-dwarf2-cfi-asm

ifeq ($(USE_THUMB_MODE),YES)
	CBASE += -mthumb-interwork -DTHUMB_INTERWORK
	THUMB_FLAGS=-mthumb
endif

CFLAGS=$(WARNINGSC) $(CBASE) -std=c99

CXXFLAGS=$(WARNINGSCXX) $(CBASE) -fno-exceptions -fno-rtti

LINKER_FLAGS=-Xlinker -o$(OUTPUT).elf -Xlinker -M -Xlinker -Map=$(OUTPUT).map

RTOS_SOURCE_DIR=source/freertos


ifneq ($(BUILD),$(notdir $(CURDIR)))

export LDSCRIPT=$(CURDIR)/lpc2104-rom2.ld
export MAKEFILE = $(CURDIR)/Makefile

export INCLUDE = $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
        $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
        -I$(CURDIR)/$(BUILD)

export OUTPUT   :=  $(CURDIR)/$(TARGET)
export VPATH    :=  $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
            $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR  :=  $(CURDIR)/$(BUILD)

export CRT0=$(CURDIR)/boot/boot.s

export CFILES   = $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export CPPFILES = $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
export AUTOSRC = $(AUTOC) $(AUTOCPP)

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

#export OFILES = $(ARM_OBJ) $(THUMB_OBJ)

export OFILES   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@#@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@make -C $(BUILD) -f $(MAKEFILE)

all:	$(BUILD)

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).{hex,map,elf}

burn:	$(BUILD)
	$(LPC21ISP)


else

DEPENDS :=  $(OFILES:.o=.d)

$(OUTPUT).hex:	$(OUTPUT).elf

$(OUTPUT).elf:	$(OFILES)

-include $(DEPENDS)

endif


$(OUTPUT).hex : $(OUTPUT).elf
	$(OBJCOPY) $(OUTPUT).elf -O ihex $(OUTPUT).hex

$(OUTPUT).elf : $(OFILES) $(CRT0) $(MAKEFILE)
	$(CC) $(CFLAGS) $(OFILES) -nostartfiles $(CRT0) $(LINKER_FLAGS)

#$(OFILES):	%.o : %.c %(LDSCRIPT) %(MAKEFILE)

%.o:    %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -mthumb -c $< -o $@ $(ERROR_FILTER)







	


