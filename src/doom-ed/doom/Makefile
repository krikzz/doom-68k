GCC= e:/devtools/m68k-3.4.6/
CC= $(GCC)bin/gcc
OBJC= $(GCC)bin/objcopy
OUTPUT= doom-ed
LIB=$(GCC)lib
BIN= $(GCC)bin
LD= $(GCC)bin/ld


MAKEFLAGS= -j16

BDIR		:=build
FLAGS		:= $(OPTION) -m68000 -Wall -O3 -fomit-frame-pointer $(INCS)

vpath %.o $(BDIR)/

SRC_S = sega.s
SRC_C = $(wildcard *.c)

OBJ = $(SRC_S:.s=.o)
OBJ += $(SRC_C:.c=.o)
OFILES = $(OBJ:%.o=$(BDIR)/%.o)


all: $(OBJ)
	$(LD) -Map memory.map -T rom.ld -nostdlib $(OFILES) $(LIB)/libgcc.a -o $(BDIR)/$(OUTPUT).elf 
	$(OBJC) --pad-to 0x80000 -O binary $(BDIR)/$(OUTPUT).elf $(OUTPUT).md
	


%.o: %.c
	$(CC) $(FLAGS) -c $< -o $(basename $(BDIR)/$<).o

%.o: %.s
	$(CC) $(FLAGS) -c $< -o $(basename $(BDIR)/$<).o

clean:
	$(RM) $(BDIR)/*.o
	$(RM) $(BDIR)/*.elf


