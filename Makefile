B = build
K = kernel
U = user
G = build/kernel
H = build/user
O = $G/boot.o \
	$G/main.o \
	$G/mini_uart.o \
	$G/pl011_uart.o \
	$G/printf.o \
	$G/console.o \
	$G/kalloc.o \
	$G/vm.o \
	$G/mmu.o \
	$G/proc.o \
	$G/trap.o \
	$G/trampoline.o \
	$G/kernelvec.o \
	$G/swtch.o \
	$G/syscall.o \
	$G/exec.o \
	$G/gicv2.o \
	$G/timer.o \
	$G/spinlock.o \
	$G/fd.o \
	$G/pipe.o \
	$G/elf.o \
	$G/sdhci.o \
	$G/bio.o \
	$G/fs.o

C = clang
A = llvm-mc
L = ld.lld

D = -g -O0
D += --target=aarch64-elf
D += -nostdlib -ffreestanding
# D += -Iinclude

M =
# M = --aarch64-elf=aarch64-elf



all: $B/kernel8.img $B/fs.img
# echo -n "HELLO " | dd of=$B/fs.img bs=1M seek=0 count=1 conv=sync

# all: clean kernel8.img
# qemu-system-aarch64 -M raspi4b -kernel build/kernel8.img  -nographic -S -s 


$B/kernel8.img: $O $K/kernel.ld uprog
# $C $D -T $K/kernel.ld -o $B/kernel8.elf $O $P
	$L -m aarch64elf -T $K/kernel.ld -o $B/kernel8.elf $O -b binary $P

# 因为elf中包含有header、符号表等，在加载kernel的时候不会对这些进行处理，所以直接使用elf将不会正确运行，需要objcopy，最开始安装llvm时，因为objcopy没有软连接，使用mdfind -name objcopy找到正确的objcopy路径。
	llvm-objcopy -O binary build/kernel8.elf build/kernel8.img
#默认已stdio，如果使用mini_uart要添加选项：-serial null -serial mon:stdio 

$G/%.o: $K/%.S | dir
	echo $@
	echo $<
	$C $D -Iinclude -c -o $@ $<
$G/%.o: $K/%.c
	echo $@
	echo $<
	$C $D -Iinclude -c -o $@ $<
dir:
	mkdir -p $G $H

P = $H/user1 \
	$H/user2 
I = $H/ulib.o 
uprog: $P $(I)
# $H/%: $H/%.bin
# 	$L -m aarch64elf -r -b binary $< -o $@
# $H/%.bin: $H/%.elf
# 	llvm-objcopy -O binary $< $@
# $H/%.elf: $H/%.o $(I)
$H/%: $H/%.o $(I)
	$L $M  -T $U/user.ld -o $@ $< $(I)
$H/%.o: $U/%.c
	echo "Compiling $@ from $<"
	echo "$(I)"
	$C $D -c -o $@ $<

#使用lldb时，使用gdb-remote host：port
#(lldb) gdb-remote localhost:1234

# copy image to SD card
# cp "/Users/galax/Library/Mobile Documents/com~apple~CloudDocs/gcgos/kernel8.img" /Volumes/bootfs/
$B/mkfs: mkfs/mkfs.c include/fs.h
	$C -o $@ $<

$B/fs.img: $B/mkfs
	$< $@

clean:
	rm $P $O $B/kernel8* $B/mkfs $B/fs.img
