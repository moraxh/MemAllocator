CC = gcc
CFLAGS = -g -Wall
SRCDIR = src
BINDIR = bin
OBJDIR = $(BINDIR)/obj

PROGRAM = main

main: $(SRCDIR)/$(PROGRAM).c $(OBJDIR)/alloc.o
	$(CC) $(CFLAGS) -o $(BINDIR)/$(PROGRAM) $(OBJDIR)/alloc.o $(SRCDIR)/$(PROGRAM).c

$(OBJDIR)/alloc.o: $(SRCDIR)/alloc.h $(SRCDIR)/alloc.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $(OBJDIR)/alloc.o -c $(SRCDIR)/alloc.c


clean:
	rm -rf $(BINDIR)
