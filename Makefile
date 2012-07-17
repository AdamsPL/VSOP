MEMORY=1024
CORES=4

DEV=/dev/loop0
IMG=disk.img
TMPFILE=$(IMG).tmp
TMPDIR=mnt
VIRT=qemu-system-i386

.PHONY: run mount unmount

all: run

$(IMG):
	@echo "***creating temporary file...***"
	dd if=/dev/zero of=$(TMPFILE) bs=512 count=40920
	@echo "***partitioning...***"
	fdisk $(TMPFILE) < fdisk.in
	@echo "***mounting...***"
	losetup $(DEV) $(TMPFILE) --offset 1048576
	@echo "***creating filesystem...***"
	mkfs.ext2 $(DEV)
	mkdir -p $(TMPDIR)
	sudo mount $(DEV) $(TMPDIR)
	mkdir -p $(TMPDIR)/boot/grub/
	cp /boot/grub/stage1 /boot/grub/stage2 $(TMPDIR)/boot/grub/
	echo "device (hd0) "$(shell pwd)/$(TMPFILE) > grub.in
	echo "root (hd0,0)" >> grub.in
	echo "setup (hd0)" >> grub.in
	grub --device-map=/dev/null < grub.in
	@echo "***unmounting...***"
	sync
	sudo umount $(DEV)
	losetup -d $(DEV)
	@echo "**finishing and cleaning***"
	mv $(TMPFILE) $(IMG)
	rm grub.in

run: $(IMG)
	make -C kernel
	cp kernel/kernel.img $(TMPDIR)/
	make -C libc
	make -C userspace
	cp userspace/srv* $(TMPDIR)
	cp menu.lst $(TMPDIR)/boot/grub/
	cat userspace/modfile >> $(TMPDIR)/boot/grub/menu.lst
	echo "boot\n" >> $(TMPDIR)/boot/grub/menu.lst
	sync
	$(VIRT) -cpu coreduo -m $(MEMORY) -smp ${CORES} -s $(IMG) -monitor stdio

mount:
	losetup $(DEV) $(IMG) --offset 1048576
	sudo mount $(DEV) $(TMPDIR)

umount:
	sudo umount $(DEV)
	losetup -d $(DEV)

$(TMPDIR)/kernel.img: kernel/kernel.img
	cp kernel/kernel.img $@
	sync

kernel/kernel.img:
	make -C kernel
