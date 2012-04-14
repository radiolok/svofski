TARGET=3losh

MCU = atmega8
CLK = 20000000UL
CC = avr-gcc
CXX = avr-g++
OBJCOPY=avr-objcopy
#HEX_FLASH_FLAGS=--pad-to 0x2000

OPTIM=-O2
WARNINGS=-Wall

BUILD=build
SOURCES=src

INCLUDES=src

COMMON = -mmcu=$(MCU) -DF_CPU=$(CLK)
CFLAGS  = $(COMMON) -std=c99
CFLAGS += $(WARNINGS) $(OPTIM) $(WARNINGS) $(INCLUDE)
CFLAGS += -MD -MP -MT $(*F).o -MF $(DEPSDIR)/$(@F).d

CXXFLAGS = $(COMMON)
CXXFLAGS+= $(WARNINGS) $(OPTIM) $(WARNINGS) $(INCLUDE)
CXXFLAGS+= -MD -MP -MT $(*F).o -MF $(DEPSDIR)/$(@F).d
CXXFLAGS+= -ffunction-sections -Wl,--gc-sections

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=

ifneq ($(BUILD),$(notdir $(CURDIR)))

export MAKEFILE = $(CURDIR)/Makefile

export INCLUDE = $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
        $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
        -I$(CURDIR)/$(BUILD)

export OUTPUT   :=  $(CURDIR)/$(TARGET)
export VPATH    :=  $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
            $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR  :=  $(CURDIR)/$(BUILD)

export CFILES   = $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export SFILES   = $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
export CPPFILES = $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
export AUTOSRC = $(AUTOC) $(AUTOCPP)

#
# Define all object files.
#

export OFILES   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.S=.o)

.PHONY: $(BUILD) clean 

# ---------------------- 

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@#@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@make -C $(BUILD) -f $(MAKEFILE)

all:    $(BUILD) size

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).{hex,map,elf,lss} html

burn:   $(BUILD)
	avrdude -p m8 -c avrisp2 -P usb -U flash:w:"$(TARGET).hex":i -v -V -u

fuses:
	# for 12MHz crystal, no bootloader
	avrdude -p m8 -c pickit2 -U hfuse:w:0xcc:m -U lfuse:w:0xef:m

size:	$(BUILD)
	@echo
	@avr-size $(OUTPUT).elf


else

DEPENDS :=  $(OFILES:.o=.d)

bazola:	$(OUTPUT).hex $(OUTPUT).lss size

$(OUTPUT).lss:	$(OUTPUT).elf

$(OUTPUT).hex:  $(OUTPUT).elf

$(OUTPUT).elf:  $(OFILES)

size:	$(OUTPUT).elf
	@echo
	@avr-size $(OUTPUT).elf

-include $(DEPENDS)

endif

$(OUTPUT).hex:	$(OUTPUT).elf
	$(OBJCOPY) $(OUTPUT).elf -O ihex $(HEX_FLASH_FLAGS) $(OUTPUT).hex

$(OUTPUT).lss:	$(OUTPUT).elf
	avr-objdump -h -S $< > $@

$(OUTPUT).elf : $(OFILES) $(MAKEFILE)
	$(CXX) $(CXXFLAGS) $(OFILES) $(LINKER_FLAGS) -o$(OUTPUT).elf

%.o:    %.c
	@echo $(notdir $<)
	$(CC) $(CFLAGS) -c $< -o $@ $(ERROR_FILTER)

%.o:	%.S
	@echo $(notdir $<)
	$(CC) $(ASMFLAGS) -c $< -o $@ $(ERROR_FILTER)
