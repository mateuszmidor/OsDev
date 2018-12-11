#!/bin/bash

HDD=build/hdd.vdi
ISO=build/kernel/phobos-x86_64.iso

qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb $HDD -cdrom $ISO -s -S -no-reboot # pcnet is AMD am79c973 network chip