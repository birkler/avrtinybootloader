# Object files directory
#     To put object files in current directory, use a dot (.), do NOT make
#     this an empty or blank macro!
OBJDIR = ./obj/obj


# List C source files here. (C dependencies are automatically generated.)
SRC = ./../usbdrv/usbdrv.c
SRC += main.c
SRC += ../osccal/osccal.c
SRC += ../usbdrv/oddebug.c
SRC += ../util/timers.c
SRC += ../util/stream.c
SRC += ../util/usbstream.c
#SRC += ../util/uartsw.c


# List C++ source files here. (C dependencies are automatically generated.)
CPPSRC = 


# List Assembler source files here.
#     Make them always end in a capital .S.  Files ending in a lowercase .s
#     will not be considered source files but generated files (assembler
#     output from the compiler), and will be deleted upon "make clean"!
#     Even though the DOS/Win* filesystem matches both .s and .S the same,
#     it will preserve the spelling of the filenames, and gcc itself does
#     care about how the name is spelled on its command-line.
ASRC = ./../usbdrv/usbdrvasm.S
 
# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRAINCDIRS =  ../usbdrv ../osccal ../util .. ../..
 
# Place -D or -U options here for C sources
CDEFS = -DUSBSTREAM_DISABLE_RX

include ../makefile.inc

