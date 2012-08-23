#!/bin/bash

DEV=/dev/loop0
PART=/dev/loop1

IMG=disk.img
TMPFILE=${IMG}.tmp
TMPDIR=mnt/
SIZE=59200

function create_loop_dev
{
	sudo losetup -d ${DEV}
	sudo losetup -d ${PART}
	sudo losetup ${DEV} ${TMPFILE}
	sudo losetup ${PART} ${TMPFILE} --offset 1048576
}

function close_loop_dev
{
	sudo losetup -d ${DEV}
	sudo losetup -d ${PART}
}


echo "***creating temporary file...***"
close_loop_dev
dd if=/dev/zero of=${TMPFILE} bs=512 count=${SIZE}
create_loop_dev
sync

echo "***partitioning...***"
echo -e "n\np\n\n\n\nw\n" | sudo fdisk ${DEV}
sudo partx -s ${DEV}
sync

echo "***creating filesystem***"
sudo mkfs.ext2 ${PART}
sync

echo "***mounting...***"
mkdir -p ${TMPDIR}
sudo umount ${PART}
sudo mount ${PART} ${TMPDIR}
sudo chown adams:users ${TMPDIR}
sync

echo "***installing grub***"
mkdir -p ${TMPDIR}/boot/grub/
cp /boot/grub/stage1 /boot/grub/stage2 ${TMPDIR}/boot/grub/
echo -e "device (hd0) "`pwd`"/${TMPFILE}\nroot (hd0,0)\nsetup (hd0)\n" | grub --device-map=/dev/null
sync

echo "***unmounting...***"
sudo umount ${PART}
sudo losetup -d ${PART}
sync

echo "**finishing and cleaning up***"
mv ${TMPFILE} ${IMG}
sudo chown adams:users ${IMG}
rm -rvf ${TMPDIR}
sync
