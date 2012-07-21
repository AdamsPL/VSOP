include ../Makefile.common
SRC=$(wildcard *.c)
OBJ=$(subst .c,,$(SRC))
PROG=$(subst .o,,$(OBJ))
CFLAGS+=-I../libc/
LDFLAGS+=-L../libc/ -lc -nostdlib -static

.SUFFIXES:
.PHONY: clean all

all: $(PROG)

clean:
	rm -rvf $(OBJ)

%.o :: %.c ../libc/libc.a
	gcc -c $(CFLAGS) -o $@ $<

% : %.o
	ld $< -o $@ $(LDFLAGS)

