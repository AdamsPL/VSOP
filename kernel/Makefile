LDFLAGS=-Tlink.ld -nostdlib -nodefaultlibs
SRC=$(wildcard *.[c,s])
OBJ=$(subst .s,.o,$(SRC:.c=.o))
TGT=kernel.img
OPTIMIZE=-ggdb
CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -pipe -march=native ${OPTIMIZE}

.PHONY: clean run

all: $(TGT)

$(TGT): $(OBJ)
	ld $(LDFLAGS) -o $@ $(OBJ)
clean:
	rm -rvf $(OBJ) $(TGT)

run: all
	make -C ..
