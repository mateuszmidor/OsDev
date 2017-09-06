arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso
hdd := build/hdd.vdi

GCCPARAMS = -std=c++11 -mno-red-zone -fno-use-cxa-atexit -fno-rtti -fno-exceptions -ffreestanding -O0 -g3
GCCINCLUDES = -Isrc -Isrc/cpu -Isrc/cpuexceptions -Isrc/drivers -Isrc/filesystem -Isrc/filesystem/fat32 -Isrc/hardware -Isrc/multitasking \
			  -Isrc/memory -Isrc/utils -Isrc/utils/terminal

linker_script := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/grub.cfg
assembly_source_files := $(wildcard src/arch/$(arch)/*.S)
assembly_object_files := $(patsubst src/arch/$(arch)/%.S, \
	build/arch/$(arch)/%.o, $(assembly_source_files))
	
c_source_files :=  $(wildcard src/*.cpp) \
                   $(wildcard src/cpu/*.cpp) \
                   $(wildcard src/drivers/*.cpp) \
                   $(wildcard src/cpuexceptions/*.cpp) \
                   $(wildcard src/filesystem/*.cpp) \
                   $(wildcard src/filesystem/fat32/*.cpp) \
                   $(wildcard src/hardware/*.cpp) \
                   $(wildcard src/multitasking/*.cpp) \
                   $(wildcard src/memory/*.cpp) \
                   $(wildcard src/utils/*.cpp) \
                   $(wildcard src/utils/terminal/*.cpp) \
                   $(wildcard src/utils/terminal/cmds/*.cpp) \
                   $(wildcard src/_demos/*.cpp)
                   
c_object_files := $(patsubst src/%.cpp, \
	build/%.o, $(c_source_files))

.PHONY: all clean run iso

all: $(kernel)

clean:
	@rm -rf build

install: $(kernel)
	sudo cp $< /boot/PhobOS.bin
	
run: $(iso) $(hdd)
	@qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb $(hdd) -cdrom $(iso) -d int -no-reboot # pcnet is AMD am79c973 network chip

rungdb: $(iso) $(hdd)	
	@qemu-system-x86_64 -net nic,model=pcnet -boot d -hdb $(hdd) -cdrom $(iso) -s -S -no-reboot
	
iso: $(iso)

# build bootable iso image
$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@grub-mkrescue -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles

# copy beforehand-prepared hdd drive image
# to prepare hdd.vdi: qemu-img create -f vdi hdd.vdi 64M, then create DOS partition table and 2 FAT32 partitions
hdd: $(hdd)
$(hdd):
	@cp media/hdd.vdi $(hdd)
	
# bulid kernel binary
$(kernel): $(assembly_object_files) $(c_object_files) $(linker_script)
	@ld -n -T $(linker_script) -o $(kernel) $(assembly_object_files) $(c_object_files)

# compile assembly files
build/arch/$(arch)/%.o: src/arch/$(arch)/%.S
	@mkdir -p $(shell dirname $@)
	@as --64 $< -o $@
	
# compile c++ files
build/%.o: src/%.cpp
	@mkdir -p $(shell dirname $@)
	@g++ $(GCCPARAMS) $(GCCINCLUDES) -c  $< -o $@	