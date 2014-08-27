CC=gcc
RM=rm
CFLAGS=-I. -O3
HDRS = errors.h expr.h global.h output.h symbols.h utils.h
DEPS = $(HDRS)
_OBJS = errors.o expr.o main.o output.o symbols.o utils.o 
ODIR = obj
EXEC = asm65

OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)


.PHONY: clean

clean:
	$(RM) -f $(ODIR)/*.o $(EXEC)

