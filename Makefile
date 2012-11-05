MEMORY=512
CORES=2
SUBDIRS=kernel libc userspace
RELPATH=.
include $(RELPATH)/Makefile.top

.PHONY: run mount umount clean build install

all: build

build:
	$(foreach dir, $(SUBDIRS), make -C $(dir);)

install: build $(MOUNTCANARY)
	$(foreach dir, $(SUBDIRS), make -C $(dir) install;)
	sync

clean:
	$(foreach dir, $(SUBDIRS), make -C $(dir) clean;)

run: $(MOUNTCANARY) $(GRUBMENU) install
	qemu-system-i386 -sdl -cpu coreduo -m $(MEMORY) -smp $(CORES) -s $(IMG) -monitor stdio

$(GRUBMENU): $(MOUNTCANARY) userspace/modfile
	echo "" > $(GRUBMENU)
	echo "default 0" >> $(GRUBMENU)
	echo "timeout 5" >> $(GRUBMENU)
	echo "title VSOP" >> $(GRUBMENU)
	echo "root (hd0,0)" >> $(GRUBMENU)
	echo "kernel /kernel.img max_cpu=8" >> $(GRUBMENU)
	$(foreach mod, $(shell cat userspace/modfile), echo "modulenounzip /$(mod)" >> $(GRUBMENU);)
	echo -e "boot\n" >> $(GRUBMENU)
	sync

stats:
	 wc -l `find -regex '.*\.[chs]$$' -or -name "Makefile*"`
	 du -sh kernel/kernel.img

ctags:
	ctags -R --fields=+iaS --extra=+q .

usb: install
	sync
	sudo dd if=disk.img of=/dev/disk/by-id/usb-Kingston_DT_100_G2_001CC0C60DBBCBB024232591-0:0
	sync

include $(RELPATH)/Makefile.bot
