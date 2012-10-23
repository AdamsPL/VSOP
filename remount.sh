#!/bin/bash

make umount
rm disk.img
./create_image.sh
sync
make mount
sync
make
sync
make run
sync
sudo dd if=disk.img of=/dev/sdc
sync

