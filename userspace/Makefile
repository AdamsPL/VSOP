MODULES=$(shell cat modfile)
CFLAGS+=-I../libc/
LDFLAGS+=-L../libc/ -lc -nostdlib -static
RELPATH=..
include $(RELPATH)/Makefile.top

.SUFFIXES:
.PHONY: clean all install build

all: build

build: $(MODULES)

install: build
	$(foreach file, $(MODULES), cp -v $(file) $(DIST)/$(file);)

clean:
	rm -rvf $(subst .c,,$(wildcard *.c))

%.o : %.c $(RELPATH)/libc/libc.a
	$(CC) -c $(CFLAGS) -o $@ $<

% : %.o
	ld $< -o $@ $(LDFLAGS)

include $(RELPATH)/Makefile.bot
