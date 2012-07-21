MEMORY=1024
CORES=4

DEV=/dev/loop1
IMG=disk.img
TMPFILE=$(IMG).tmp
TMPDIR=mnt/
VIRT=qemu-system-i386
USER=adams
GROUP=users

.PHONY: run mount unmount clean

all: run

run: $(TMPDIR)/boot/grub/menu.lst
	make -C kernel
	cp kernel/kernel.img $(TMPDIR)/
	make -C libc
	make -C userspace
	cp userspace/srv* $(TMPDIR)
	cp menu.lst $(TMPDIR)/boot/grub/
	cat userspace/modfile >> $(TMPDIR)/boot/grub/menu.lst
	echo -e "boot\n" >> $(TMPDIR)/boot/grub/menu.lst
	sync
	$(VIRT) -cpu coreduo -m $(MEMORY) -smp ${CORES} -s $(IMG) -monitor stdio

$(TMPDIR)/boot/grub/menu.lst : mount

mount: $(IMG)
	sudo losetup $(DEV) $(IMG) --offset 1048576
	sudo mount $(DEV) $(TMPDIR)
	sudo chown -R adams:users $(TMPDIR)

umount:
	sudo umount $(DEV)
	sudo losetup -d $(DEV)

$(TMPDIR)/kernel.img: kernel/kernel.img
	cp kernel/kernel.img $@
	sync

kernel/kernel.img:
	make -C kernel

clean:
	make -C kernel clean
	make -C libc clean
	make -C userspace clean
