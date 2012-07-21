include ../Makefile.common
SRC=$(wildcard *.[c,s])
OBJ=$(subst .s,.o,$(SRC:.c=.o))
TGT=libc.a

.PHONY: clean run

all: $(TGT)

$(TGT): $(OBJ)
	ar -rcs $(TGT) $(OBJ)
clean:
	rm -rvf $(OBJ) $(TGT)

run: all
	make -C ..
