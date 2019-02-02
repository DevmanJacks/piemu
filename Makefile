######################################
#
# Raspberry Pi Emulator for Model B+
#
# (c) Mark Jackson	2019
#
######################################

# Compiler flags
CC = cc
CFLAGS = -g
LFLAGS =

# Source and object directories
SRCDIR = src
OBJDIR = obj

# Object files
OBJECTS = $(OBJDIR)/error.o $(OBJDIR)/gpio.o $(OBJDIR)/memory.o $(OBJDIR)/cpu.o $(OBJDIR)/disassemble.o $(OBJDIR)/debugger.o $(OBJDIR)/piemu.o

.PHONY: all clean

all: piemu

piemu: $(OBJECTS)
	$(CC) -o piemu $^ $(LFLAGS)

$(OBJDIR)/error.o: $(SRCDIR)/error.c
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/gpio.o: $(SRCDIR)/gpio.c $(SRCDIR)/error.h $(SRCDIR)/gpio.h
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/memory.o: $(SRCDIR)/memory.c $(SRCDIR)/error.h $(SRCDIR)/gpio.h $(SRCDIR)/memory.h
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/cpu.o: $(SRCDIR)/cpu.c $(SRCDIR)/error.h $(SRCDIR)/memory.h
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/disassemble.o: $(SRCDIR)/disassemble.c  $(SRCDIR)/cpu.h  $(SRCDIR)/disassemble.c $(SRCDIR)/error.h $(SRCDIR)/memory.h
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/debugger.o: $(SRCDIR)/debugger.c $(SRCDIR)/cpu.h $(SRCDIR)/debugger.h
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/piemu.o: $(SRCDIR)/piemu.c $(SRCDIR)/cpu.h $(SRCDIR)/debugger.h $(SRCDIR)/disassemble.h $(SRCDIR)/memory.h 
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	@-rm -rf $(OBJDIR)
	@-rm -f piemu
