SRC=$(wildcard *.c)
OBJ=$(subst .c,,$(SRC))
PROG=$(subst .o,,$(OBJ))
OPTIMIZE=-O2
CFLAGS=-I../libc/ -fno-builtin -fno-stack-protector -Wall -pedantic -pipe -Werror -march=native ${OPTIMIZE}
LDFLAGS=-L../libc/ -lc -nostdlib -static

.SUFFIXES:
.PHONY: clean run all clean_modfile

all: $(PROG)

clean:
	rm -rvf $(OBJ)

%.o :: %.c ../libc/libc.a
	gcc -c $(CFLAGS) -o $@ $<

% : %.o
	ld $< -o $@ $(LDFLAGS)

