# Name: Makefile
# Project: AVRUSBBoot
# Modified by: Thomas Fischl
# Modified: 2006-06-25

# Original file by: Christian Starkjohann
# Creation Date: 2004-12-29
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile 147 2006-03-01 17:33:03Z cs $

BOOTLOADER_ADDRESS = 0

F_CPU = 16500000
MCU = attiny861


LDFLAGS += -Wl,--section-start=.text-progmem=$(BOOTLOADER_ADDRESS)
LDFLAGS += -Wl,-Map=main.map,--cref


SERIAL = `echo /dev/tty.KeySerial*`
UISP = uisp -dprog=avr910 -dserial=$(SERIAL) -dpart=auto
# The two lines above are for "uisp" and the AVR910 serial programmer connected
# to a Keyspan USB to serial converter to a Mac running Mac OS X.
# Choose your favorite programmer and interface.

COMPILE = avr-gcc -gdwarf-2 -Wall -Os -I../usbdrv -I. -mmcu=attiny861  -DF_CPU=$(F_CPU)UL 
#-DDEBUG_LEVEL=2
# NEVER compile the final product with debugging! Any debug output will
# distort timing so that the specs can't be met.

OBJECTS = space.o ../usbdrv/usbdrv.o ../usbdrv/usbdrvasm.o ../usbdrv/oddebug.o main.o ../osccal/osccal.o
# Note that we link usbdrv.o first! This is required for correct alignment of
# driver-internal global variables!

HEXSIZE = @avr-size --target=ihex main.hex

# symbolic targets:
all:	main.hex sizeafter

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@


#---------------- Programming Options (avrdude) ----------------

AVRDUDE = avrdude

# Programming hardware
# Type: avrdude -c ?
# to get a full listing.
#
AVRDUDE_PROGRAMMER = usbtiny

# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT = lpt1    # programmer connected to serial device

AVRDUDE_WRITE_FLASH = -U flash:w:main.hex
#AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep


# Uncomment the following if you want avrdude's erase cycle counter.
# Note that this counter needs to be initialized first using -Yn,
# see avrdude manual.
#AVRDUDE_ERASE_COUNTER = -y

# Uncomment the following if you do /not/ wish a verification to be
# performed after programming the device.
#AVRDUDE_NO_VERIFY = -V

# Increase verbosity level.  Please use this when submitting bug
# reports about avrdude. See <http://savannah.nongnu.org/projects/avrdude> 
# to submit bug reports.
#AVRDUDE_VERBOSE = -v -v

AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)



flash:	all
	avrdude -u -pm8 -P lpt1 -c STK200 -U flash:w:main.hex
//	$(UISP) --erase --upload --verify if=main.hex

# Program the device.  
program: main.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)



avrdude: 
	avrdude -c avr910 -p atmega8 -U flash:w:main.hex

# Fuse low byte:
# 0xef = 1 1 1 0   1 1 1 1
#        ^ ^ \+/   \--+--/
#        | |  |       +------- CKSEL 3..0 (clock selection -> crystal @ 12 MHz)
#        | |  +--------------- SUT 1..0 (BOD enabled, fast rising power)
#        | +------------------ CKOUT (clock output on CKOUT pin -> disabled)
#        +-------------------- CKDIV8 (divide clock by 8 -> don't divide)
#
# Fuse high byte:
# 0xdb = 1 1 0 1   1 0 1 1
#        ^ ^ ^ ^   \-+-/ ^
#        | | | |     |   +---- RSTDISBL (disable external reset -> enabled)
#        | | | |     +-------- BODLEVEL 2..0 (brownout trigger level -> 2.7V)
#        | | | +-------------- WDTON (watchdog timer always on -> disable)
#        | | +---------------- SPIEN (enable serial programming -> enabled)
#        | +------------------ EESAVE (preserve EEPROM on Chip Erase -> not preserved)
#        +-------------------- DWEN (debug wire enable)

fuse_tiny2313:	# only needed for attiny2313
	$(UISP) --wr_fuse_l=0xef --wr_fuse_h=0xdb

# Fuse high byte:
# 0xc9 = 1 1 0 0   0 0 0 0 <-- BOOTRST (boot reset vector at 0x0C00)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   + --------- EESAVE (don't preserve EEPROM over chip erase)
#        | | | +-------------- CKOPT (full output swing)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ WDTON (WDT not always on)
#        +-------------------- RSTDISBL (reset pin is enabled)
# Fuse low byte:
# 0x9f = 1 0 0 1   1 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external >8M crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ BODEN (BrownOut Detector enabled)
#        +-------------------- BODLEVEL (2.7V)
fuse_mega8:
	avrdude -u -pm8 -P lpt1 -c STK200 -U lfuse:w:0x9f:m -U hfuse:w:0xc1:m

# Fuse extended byte:
# 0xfe = 1 1 1 1   1 1 1 0
#        \-+-----------/ ^
#          |             +---- SELFPGRM ENABLE
#          +------------------ not used
#
# Fuse high byte:
# 0xdd = 1 1 0 1   1 1 0 1
#        ^ ^ ^ ^   ^ \-+-/ 
#        | | | |   |   +------ BODLEVEL 2..0 (brownout trigger level -> 2.7V)
#        | | | |   +---------- EESAVE (preserve EEPROM on Chip Erase -> not preserved)
#        | | | +-------------- WDTON (watchdog timer always on -> disable)
#        | | +---------------- SPIEN (enable serial programming -> enabled)
#        | +------------------ DWEN (debug wire enable)
#        +-------------------- RSTDISBL (disable external reset -> enabled)
#
# Fuse low byte:
# 0xe1 = 1 1 1 0   0 0 0 1
#        ^ ^ \+/   \--+--/
#        | |  |       +------- CKSEL 3..0 (clock selection -> HF PLL)
#        | |  +--------------- SUT 1..0 (BOD enabled, fast rising power)
#        | +------------------ CKOUT (clock output on CKOUT pin -> disabled)
#        +-------------------- CKDIV8 (divide clock by 8 -> don't divide)
fuse_t861:
	avrdude -u -pt861  -c usbtiny -U hfuse:w:0xdd:m -U lfuse:w:0xe1:m -U efuse:w:0xfe:m 

# Display size of file.
sizebefore:
	@echo Size before:
	-$(HEXSIZE)

sizeafter:
	@echo Size after:
	$(HEXSIZE)

clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.elf default/main.elf *.o ../usbdrv/*.o main.s ../usbdrv/oddebug.s ../usbdrv/usbdrv.s ../osccal/osccal.o

# file targets:
main.elf:	$(OBJECTS) usbconfig.h bootloaderconfig.h
	$(COMPILE) -o main.elf $(OBJECTS) $(LDFLAGS)
	-cp main.elf default/main.elf

main.hex:	main.elf
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	-./checksize main.elf
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.
#	avr-objcopy -j .fuse -O binary main.elf main.hf

disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
