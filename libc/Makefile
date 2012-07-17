SRC=$(wildcard *.[c,s])
OBJ=$(subst .s,.o,$(SRC:.c=.o))
TGT=libc.a
OPTIMIZE=-O2
CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wall -pedantic -pipe -Werror -march=native ${OPTIMIZE}

.PHONY: clean run

all: $(TGT)

$(TGT): $(OBJ)
	ar -rcs $(TGT) $(OBJ)
clean:
	rm -rvf $(OBJ) $(TGT)

run: all
	make -C ..
