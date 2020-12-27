#############################################################################
# Compilation Options:														#
# 	  Command					Action										#
# ------------------------------------------------------------------------- #
# >> make all	  		=> all source files and creates all executables		#
# >> make chef 			=> chef and its dependencies						#
# >> make saladmaker	=> Inode and its dependencies						#
# >> make clean			=> removes all object and all executable files		#
#############################################################################

IDIR = ./inc
ODIR = ./obj
BDIR = ./bin
SDIR = ./src
CC = gcc
CFLAGS = -ggdb -Wall -lpthread -I$(IDIR)


_DEPS = utilities.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))


$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: chef saladmaker

chef: $(ODIR)/chef.o $(ODIR)/utilities.o
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)

saladmaker: $(ODIR)/saladmaker.o $(ODIR)/utilities.o
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(BDIR)/*
