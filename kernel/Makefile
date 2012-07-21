include ../Makefile.common
LDFLAGS+=-Tlink.ld -nostdlib -nodefaultlibs
SRC=$(wildcard *.[c,s])
OBJ=$(subst .s,.o,$(SRC:.c=.o))
TGT=kernel.img

.PHONY: clean run

all: $(TGT)

$(TGT): $(OBJ)
	ld $(LDFLAGS) -o $@ $(OBJ)
clean:
	rm -rvf $(OBJ) $(TGT)

run: all
	make -C ..
