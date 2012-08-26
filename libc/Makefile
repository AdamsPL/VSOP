SRC=$(wildcard *.[c,s])
OBJ=$(subst .s,.o,$(SRC:.c=.o))
TGT=libc.a
RELPATH=..
include $(RELPATH)/Makefile.top

.PHONY: clean install build

all: build

build: $(TGT)

$(TGT): $(OBJ)
	ar -rcs $(TGT) $(OBJ)

clean:
	rm -rvf $(OBJ) $(TGT)

install:

include $(RELPATH)/Makefile.bot
