LDFLAGS+=-Tlink.ld -nostdlib -nodefaultlibs
SRC=$(wildcard *.[c,s])
OBJ=$(subst .s,.o,$(SRC:.c=.o))
TGT=kernel.img
RELPATH=..
include $(RELPATH)/Makefile.top

.PHONY: clean install build

all: build

build: $(TGT)

$(TGT): $(OBJ)
	ld $(LDFLAGS) -o $@ $(OBJ)

install: $(TGT)
	cp $< $(DIST)/$<

clean:
	rm -rf $(OBJ) $(TGT)

include $(RELPATH)/Makefile.bot
