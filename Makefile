OBJS = kernel/entry.o\
	kernel/main.o\
	kernel/global.o\
	kernel/panic.o\
	lib/string.o\
	lib/fs.o\
	lib/mm.o\
	kernel/vectors.o\
	kernel/trap.o\
	kernel/i8259.o\
	kernel/proc.o\
	mm/vm.o\
	mm/main.o\
	mm/exec.o\
	mm/proc.o\
	sys/main.o\
	fs/main.o\
	fs/do_fs.o\
	fs/ide.o\
	fs/cache.o\
	fs/inode.o\
	fs/file.o\
	fs/pipe.o\
	fs/tty.o\
	fs/kbd.o\
	kernel/trapasm.o\
	kernel/syscall.o\
	kernel/msg.o\
	kernel/switch.o\

BINS = bin/init\
		bin/sh\
		bin/echo\
		bin/gobang\
		bin/ls\
		bin/cat\
		bin/cls\
		bin/grep\
		bin/mkdir\
		bin/rm
		

LIBS = 	lib/string.o\
		lib/fs.o\
		lib/mm.o\
		lib/vsprintf.o\
		lib/stdio.o\
		lib/malloc.o\
		kernel/syscall.o\
		lib/entry.o

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)as
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump
CFLAGS = -static -fno-builtin -fno-strict-aliasing -fno-stack-protector -Wall -c -ggdb -m32 -Werror -fno-omit-frame-pointer -nostdinc
ASFLAGS = -m32 -gdwarf-2 -Wa,-divide
LDFLAGS = -m elf_i386
INCLUDE = -Iinclude -I.

all: kernel/a.out boot/boot tool/mkfs $(BINS) a.img
	dd if=boot/boot of=a.img conv=notrunc
	dd if=kernel/a.out of=a.img seek=1 conv=notrunc
	./tool/mkfs  a.img $(BINS) bin/sometext
	rm -f a.vdi
	vboxmanage convertfromraw a.img a.vdi

a.img:
	dd if=/dev/zero of=a.img count=204800  # 100m 
	
tool/mkfs:	tool/mkfs.c
	$(CC) -g -o $@ $<

boot/boot: boot/bootloader.S boot/bootloader.c
	$(CC) $(CFLAGS)  -O  $(INCLUDE) -c boot/bootloader.c -oboot/c.o
	$(CC) $(CFLAGS)  $(INCLUDE) -c boot/bootloader.S -oboot/s.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o boot/bootblock.o boot/s.o boot/c.o
	$(OBJCOPY) -S -O binary -j .text boot/bootblock.o boot/bootblock
	boot/boot.py	


kernel/initcode: kernel/initcode.c kernel/syscall.o
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o kernel/initcode.o
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o kernel/initcode.out kernel/initcode.o  kernel/syscall.o kernel/panic.o
	#$(OBJDUMP) -S kernel/initcode.out > initcode.asm
	$(OBJCOPY) -S -O binary kernel/initcode.out kernel/initcode
	


kernel/a.out: $(OBJS)  kernel.ld kernel/initcode
	$(LD) $(LDFLAGS) -T kernel.ld -o $@ $(OBJS)  -b binary kernel/initcode
	#$(OBJDUMP) -S $@ > a.asm
	
lib/lib.a: $(LIBS)
	ar  rcs $@ $(LIBS)

kernel/entry.o: kernel/entry.S
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/main.o: kernel/main.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/global.o: kernel/global.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/panic.o: kernel/panic.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
lib/string.o: lib/string.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
lib/fs.o: lib/fs.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
lib/mm.o: lib/mm.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
lib/vsprintf.o: lib/vsprintf.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
lib/stdio.o: lib/stdio.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
lib/malloc.o: lib/malloc.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
lib/entry.o: lib/entry.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
mm/vm.o: mm/vm.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/vectors.o: kernel/vectors.S
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/trap.o: kernel/trap.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/i8259.o:	kernel/i8259.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/proc.o: kernel/proc.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/trapasm.o: kernel/trapasm.S
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/syscall.o: kernel/syscall.S
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/msg.o: kernel/msg.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
kernel/switch.o: kernel/switch.S
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<

mm/main.o: mm/main.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
mm/exec.o: mm/exec.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
mm/proc.o: mm/proc.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<

sys/main.o: sys/main.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<


fs/main.o: fs/main.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
fs/kbd.o: fs/kbd.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
fs/do_fs.o: fs/do_fs.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
fs/ide.o: fs/ide.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
fs/inode.o: fs/inode.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
fs/cache.o: fs/cache.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
fs/file.o: fs/file.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
fs/pipe.o: fs/pipe.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<
fs/tty.o: fs/tty.c
	$(CC) $(CFLAGS) $(INCLUDE)  -o $@ $<

bin/init: bin/init.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/init.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/init.o lib/lib.a  
	#$(OBJDUMP) -S $@ > bin/init.asm
bin/sh: bin/sh.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/sh.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/sh.o lib/lib.a  
	#$(OBJDUMP) -S $@ > bin/sh.asm
bin/echo: bin/echo.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/echo.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/echo.o lib/lib.a  
	#$(OBJDUMP) -S $@ > bin/sh.asm
bin/gobang: bin/gobang.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/gobang.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/gobang.o lib/lib.a 

bin/ls: bin/ls.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/ls.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/ls.o lib/lib.a 


bin/cat: bin/cat.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/cat.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/cat.o lib/lib.a 

bin/cls: bin/cls.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/cls.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/cls.o lib/lib.a 

bin/grep: bin/grep.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/grep.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/grep.o lib/lib.a 

bin/mkdir: bin/mkdir.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/mkdir.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/mkdir.o lib/lib.a 

bin/rm: bin/rm.c lib/lib.a
	$(CC) $(CFLAGS) $(INCLUDE)  -o bin/rm.o  $<
	$(LD) $(LDFLAGS) -N -Ttext 0 -o $@ bin/rm.o lib/lib.a 


clean:
	rm -f $(OBJS) $(BINS) $(LIBS)  a.img a.vdi
	
