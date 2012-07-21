MEMORY=1024
CORES=4

DEV=/dev/loop0
IMG=disk.img
TMPFILE=$(IMG).tmp
TMPDIR=mnt/
VIRT=qemu-system-i386
USER=adams
GROUP=users

.PHONY: run mount unmount

all: run

$(IMG):
	@echo "***creating temporary file...***"
	dd if=/dev/zero of=$(TMPFILE) bs=512 count=409200
	sync
	@echo "***partitioning...***"
	echo -e "p\n\n\nw\n" | fdisk $(TMPFILE)
	sync
	@echo "***creating filesystem...***"
	sudo losetup $(DEV) $(TMPFILE) --offset 1048576
	sync
	sudo mkfs.ext3 $(DEV)
	sync
	@echo "***mounting...***"
	mkdir -p $(TMPDIR)
	sudo mount $(DEV) $(TMPDIR)
	sudo chown -R $(USER):$(GROUP) $(TMPDIR)
	mkdir -p $(TMPDIR)/boot/grub/
	cp /boot/grub/stage1 /boot/grub/stage2 $(TMPDIR)/boot/grub/
	echo -e "device (hd0) "$(shell pwd)/$(TMPFILE)"\nroot (hd0,0)\nsetup (hd0)\n" | grub --device-map=/dev/null
	@echo "***unmounting...***"
	sync
	sudo umount $(DEV)
	sudo losetup -d $(DEV)
	@echo "**finishing and cleaning***"
	mv $(TMPFILE) $(IMG)

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
	sudo losetup $(DEV) $(IMG) --offset 1048576
	sudo mount $(DEV) $(TMPDIR)

umount:
	sudo umount $(DEV)
	sudo losetup -d $(DEV)

$(TMPDIR)/kernel.img: kernel/kernel.img
	cp kernel/kernel.img $@
	sync

kernel/kernel.img:
	make -C kernel
