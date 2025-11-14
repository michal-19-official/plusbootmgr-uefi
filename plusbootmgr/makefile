all:
	@echo usage: 
	@echo make linux or make clean-linux or run-linux for linux
	@echo make windows or make clean-windows for windows
	@echo make mac or make clean-mac for mac
linux:
	./Compile.sh
clean-linux:
	./Cleanup.sh
windows:
	./Compile.bat
clean-windows:
	./Cleanup.bat
mac:
	./Compile-Mac.sh
clean-mac:
	./Cleanup-Mac.sh
run-linux: linux
	uefi-run ../Backend/BOOTX64.EFI -d -b /usr/share/ovmf/x64/OVMF.4m.fd
