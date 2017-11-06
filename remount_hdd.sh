#!/bin/bash

# copy hdd if doesnt exist, change owner/group to user
cp -n media/hdd.vdi build/  
chown 1000:1000 build/hdd.vdi

# unmount partitions if possible
sudo umount p1 || true
sudo umount p2 || true
sudo umount p3 || true
sudo umount p4 || true

# mount partitions
mkdir -p p1
mkdir -p p2
mkdir -p p3
mkdir -p p4
modprobe nbd max_part=16 		        # they say max_part=16 is crutial
qemu-nbd -d /dev/nbd0
qemu-nbd -c /dev/nbd0 build/hdd.vdi     # attach /dev/nbd0 to hdd.vdi
partprobe 				                # to discover partitions on nbd0
sudo mount -o,rw -ouser,umask=0000,uid=1000,gid=1000 /dev/nbd0p1 p1
sudo mount -o,rw -ouser,umask=0000 /dev/nbd0p2 p2
sudo mount -o,rw -ouser,umask=0000 /dev/nbd0p3 p3
sudo mount -o,rw -ouser,umask=0000 /dev/nbd0p4 p4