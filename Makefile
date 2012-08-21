MEMORY=1024
CORES=4

DEV=/dev/loop1
IMG=disk.img
TMPFILE=$(IMG).tmp
TMPDIR=mnt/
VIRT=qemu-system-i386 -sdl
USER=adams
GROUP=users

.PHONY: run mount umount clean build

all: build

build:
	make -C kernel
	make -C libc
	make -C userspace

run: $(TMPDIR)/boot/grub/menu.lst build
	cp kernel/kernel.img $(TMPDIR)/
	cp userspace/srv* $(TMPDIR)
	sync
	$(VIRT) -cpu coreduo -m $(MEMORY) -smp ${CORES} -s $(IMG) -monitor stdio

$(TMPDIR)/boot/grub/menu.lst : $(TMPDIR)
	cp menu.lst $(TMPDIR)/boot/grub/
	cat userspace/modfile >> $(TMPDIR)/boot/grub/menu.lst
	echo -e "boot\n" >> $(TMPDIR)/boot/grub/menu.lst
	sync

$(TMPDIR):
	mkdir -p $(TMPDIR) || true
	sudo losetup $(DEV) $(IMG) --offset 1048576 || true
	sudo mount $(DEV) $(TMPDIR) || true
	sudo chown -R adams:users $(TMPDIR)

mount: $(TMPDIR)

umount:
	sudo umount $(DEV) || true
	sudo losetup -d $(DEV) || true
	rm -rvf $(TMPDIR)

$(TMPDIR)/kernel.img: kernel/kernel.img
	cp kernel/kernel.img $@
	sync

kernel/kernel.img:
	make -C kernel

clean:
	make -C kernel clean
	make -C libc clean
	make -C userspace clean

stats:
	 wc -l `find -regex '.*\.[chs]$$' -or -name "Makefile*"`
