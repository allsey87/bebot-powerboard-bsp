# Toolchain installation
TOOLCHAIN = ../toolchain/install

# MCU name
MCU = atmega2560

# Processor frequency.
#     This will define a symbol, F_CPU, in all source code files equal to the 
#     processor frequency. You can then use this symbol in your source code to 
#     calculate timings. Do NOT tack on a 'UL' at the end, this will be done
#     automatically to create a 32-bit value in your source code.
F_CPU = 8000000

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Target file name (without extension).
TARGET = firmware

# List source files here.
SRC =  Firmware.cpp Timer.cpp Stream.cpp HardwareSerial.cpp String.cpp Print.cpp Microcontroller.cpp

# Compiler flags
CSTANDARD = -std=c++11
CDEBUG = -gdwarf-2
COPTIMIZE = -Os
CTUNING = -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CWARNINGS = -Wall
#CEXTRA = -Wa,-adhlns=$(<:.c=.lst)
CINCS = -I$(TOOLCHAIN)/avrlibc/avr/include


CXXFLAGS = $(CSTANDARD) $(CDEBUG) $(COPTIMIZE) $(CTUNING) $(CWARNINGS) $(CEXTRA) $(CINCS) -DF_CPU=$(F_CPU)UL

#---------------- Linker Options ----------------
#  -Wl,...:   Tell GCC to pass this to linker.
#  -Map:      Create map file
#  --cref:    Add cross reference to  map file

# Maths Library
MATH_LIB = -lm
# AVR LIBC
AVR_LIBC = -Wl,-L$(TOOLCHAIN)/avrlibc/avr/lib/avr6

LDFLAGS = -Wl,-Map=$(TARGET).map,--cref $(AVR_LIBC) $(MATH_LIB)

# Define programs and commands.
SHELL = sh
CC = $(TOOLCHAIN)/gcc/bin/avr-g++
OBJCOPY = $(TOOLCHAIN)/gcc/bin/avr-objcopy
OBJDUMP = $(TOOLCHAIN)/gcc/bin/avr-objdump
SIZE = $(TOOLCHAIN)/gcc/bin/avr-size
NM = $(TOOLCHAIN)/gcc/bin/avr-nm
REMOVE = rm -f
COPY = cp

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_COFF = Converting to AVR COFF:
MSG_EXTENDED_COFF = Converting to AVR Extended COFF:
MSG_FLASH = Creating load file for Flash:
MSG_EEPROM = Creating load file for EEPROM:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:


# Define all object files.
OBJ = $(SRC:.c=.o) $(CXXSRC:.cpp=.o) $(ASRC:.S=.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(CXXSRC:.cpp=.lst) $(SRC:.c=.lst)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CXXFLAGS = -mmcu=$(MCU) -I. $(CXXFLAGS)
ALL_ASFLAGS = -mmcu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)

all: build

build: elf hex eep

elf: $(TARGET).elf
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss 
sym: $(TARGET).sym

.SUFFIXES: .elf .hex .eep .lss .sym

.elf.hex:
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

.elf.eep:
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
.elf.lss:
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
.elf.sym:
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
$(TARGET).elf: $(OBJ)
	$(CC) $(ALL_CXXFLAGS) $(OBJ) --output $@ $(LDFLAGS)

# Compile: create object files from C++ source files.
.cpp.o:
	$(CXX) -c $(ALL_CXXFLAGS) $< -o $@ 


# Target: clean project.
clean:
	$(REMOVE) $(TARGET).hex $(TARGET).eep $(TARGET).cof $(TARGET).elf \
	$(TARGET).map $(TARGET).sym $(TARGET).lss \
	$(OBJ) $(LST) $(SRC:.c=.s) $(SRC:.c=.d) $(CXXSRC:.cpp=.s) $(CXXSRC:.cpp=.d)


# Listing of phony targets.
.PHONY : all build elf hex eep lss sym clean 

