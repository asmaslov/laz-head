# Makefile
#
# targets:
#   all:          compile release version
#   debug:        compile debug version
#   test:         test the connection to the mcu
#   prog:         write compiled hex file to the mcu's flash memory
#   fuse:         write the fuse bytes to the MCU
#   disasm:       disassemble the code for debugging
#   clean:        remove all build files

TARGET = laz-head
MCU = atmega16
CLK = 16000000

CC      = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE    = avr-size --format=avr --mcu=$(MCU)

DUDE_PRG = usbasp
DUDE_MCU = m16
AVRDUDE = avrdude -c $(DUDE_PRG) -p $(DUDE_MCU)

# Lookup http://www.engbedded.com/fusecalc/ for fuse values
LFU = 0xFF
HFU = 0x99

$(info $(TARGET) firmware)

PWD := $(strip $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST)))))

BUILD = $(PWD)/build

HEX = $(BUILD)/$(TARGET).hex
ELF = $(BUILD)/$(TARGET).elf
MAP = $(BUILD)/$(TARGET).map

INCS = $(wildcard $(PWD)/*.h)
SRCS = $(wildcard $(PWD)/*.c)
OBJS = $(patsubst $(PWD)/%.c, $(BUILD)/%.o, $(SRCS))

INCLUDE = -I$(PWD)

LDFLAGS = -lm
CFLAGS = -Wall -DF_CPU=$(CLK) -mmcu=$(MCU) $(INCLUDE)

.PHONY: all

all: CFLAGS += -Os -fdata-sections -ffunction-sections -fomit-frame-pointer
all: $(BUILD)/$(TARGET).hex

debug: CFLAGS += -g -O0
debug: $(HEX)

$(HEX): $(ELF)
	rm -f $(HEX)
	$(OBJCOPY) -j .text -j .data -O ihex $(ELF) $(HEX)
	$(SIZE) $(ELF)

$(ELF): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(LDFLAGS) -Wl,-Map,$(MAP) -o $@

$(BUILD)/%.o: $(PWD)/%.c $(INCS)
	mkdir -p $(@D)
	$(CC) $< -c $(CFLAGS) $(INC_DIRS) -o $@

test:
	$(AVRDUDE) -v

prog: $(HEX)
	$(AVRDUDE) -U flash:w:$(HEX)

fuse:
	$(AVRDUDE) -U lfuse:w:$(LFU):m -U hfuse:w:$(HFU):m

disasm: $(ELF)
	$(OBJDUMP) -d $(ELF)

clean:
	rm -f $(HEX) $(ELF) $(MAP) $(OBJS)
	rm -r -f build
