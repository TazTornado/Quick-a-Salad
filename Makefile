#############################################################################
# Compilation Options:														#
# 	  Command					Action										#
# ------------------------------------------------------------------------- #
# >> make all	  		=> all source files and creates all executables		#
# >> make chef 			=> chef and its dependencies						#
# >> make saladmaker	=> Inode and its dependencies						#
# >> make whatever		=> only whatever									#
# >> make clean			=> removes all object and all executable files		#
#############################################################################

IDIR = ./inc
ODIR = ./obj
BDIR = ./bin
SDIR = ./src
CC = gcc
CFLAGS = -ggdb -Wall -I$(IDIR)


_DEPS = chef.h saladmaker.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_SRC = chef.c saladmaker.c
SRC = $(patsubst %,$(SDIR)/%,$(_SRC))

_OBJ = chef.o saladmaker.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: chef saladmaker

chef: $(ODIR)/chef.o
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)

saladmaker: $(ODIR)/prime1.o
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(BDIR)/*
