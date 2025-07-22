kernel8.img: 
	clang --target=aarch64-elf  -Iinclude -nostdlib -ffreestanding source/*.S source/*.c -T kernel.ld -o build/kernel8.elf

# 因为elf中包含有header、符号表等，在加载kernel的时候不会对这些进行处理，所以直接使用elf将不会正确运行，需要objcopy，最开始安装llvm时，因为objcopy没有软连接，使用mdfind -name objcopy找到正确的objcopy路径。
	/usr/local/Cellar/llvm/19.1.6/bin/llvm-objcopy -O binary build/kernel8.elf kernel8.img

# 	/Users/galax/Documents/qemu-9.2.2/build/qemu-system-aarch64 -M raspi4b -kernel kernel8.img -nographic -S -s
#(lldb) gdb-remote localhost:1234

# copy image to SD card
cp "/Users/galax/Library/Mobile Documents/com~apple~CloudDocs/gcgos/kernel8.img" /Volumes/bootfs/

clean:
	rm kernel8.img build/*.elf