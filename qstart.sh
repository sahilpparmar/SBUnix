/home/facfs1/mferdman/cse506-tools/bin/qemu-system-x86_64 -curses -cdrom $USER.iso -drive id=disk,file=$USER.img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 -gdb tcp::12345
