IDIR =src
CC=gcc
CFLAGS=-I $(IDIR)

ODIR=obj
LDIR =lib

LIBS=-lm -pthread

_DEPS = readersStarvation.h writersStarvation.h noStarvation.h priorityQ.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o readersStarvation.o writersStarvation.o noStarvation.o priorityQ.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

lib: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)
	gcc -o test/lib $^ $(CFLAGS) $(LIBS)

test: $(OBJ)
	gcc -o test/lib $^ $(CFLAGS) $(LIBS)

$(ODIR)/%.o: $(IDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 