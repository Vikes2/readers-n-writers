IDIR =../src
CC=gcc
CFLAGS=-I $(IDIR)

ODIR=../obj
LDIR =lib

LIBS=-lm

_DEPS =
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

lib: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

$(ODIR)/%.o: $(IDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 