all:
	@echo usage: 
	@echo make linux or make clean-linux or run-linux for linux
	@echo make windows or make clean-windows for windows
	@echo make mac or make clean-mac for mac
linux:
	./Compile.sh
clean-linux:
	./Cleanup.sh
	rm -rf drive.img
windows:
	./Compile.bat
clean-windows:
	./Cleanup.bat
mac:
	./Compile-Mac.sh
clean-mac:
	./Cleanup-Mac.sh
test:
	nasm -f elf64 ../test/kernel.asm
	ld -nostdlib -static -pie \
   -T ../test/linker.ld \
   ../test/kernel.o \
   -o ../test/kernel.elf \
   --strip-all


img-linux: linux test
	dd if=/dev/zero of=drive.img bs=1M count=128
	sudo losetup /dev/loop0 drive.img
	sudo sgdisk -N 1 -t 1:EF00 -c 1:"EFI System" /dev/loop0
	sudo partprobe /dev/loop0
	sudo mkfs.fat -F 32 /dev/loop0p1
	mkdir -p mnt
	sudo mount /dev/loop0p1 mnt
	sudo mkdir -p mnt/EFI/BOOT
	sudo cp ../Backend/BOOTX64.EFI mnt/EFI/BOOT
	sudo cp ../Kernel64_sample.txt mnt/EFI/BOOT/kernel64.txt
	sudo cp ../test/kernel.elf mnt/test.elf
	sudo umount mnt
	sudo losetup -d /dev/loop0
run-linux: img-linux
	qemu-system-x86_64 -drive file=drive.img,format=raw -bios /usr/share/ovmf/x64/OVMF.4m.fd -m 1G -cpu host -enable-kvm
