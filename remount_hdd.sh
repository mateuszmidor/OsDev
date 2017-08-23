#!/bin/bash

sudo umount p1
sudo umount p2
mkdir -p p1
mkdir -p p2
modprobe nbd max_part=16 		# they say max_part=16 is crutial
qemu-nbd -d /dev/nbd0
qemu-nbd -c /dev/nbd0 build/hdd.vdi 	# attach /dev/nbd0 to hdd.vdi
partprobe 				# to discover partitions on nbd0
sudo mount -o,rw /dev/nbd0p1 p1
sudo mount -o,rw /dev/nbd0p2 p2