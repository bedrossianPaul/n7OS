KERNEL=kernel.bin
PXE=boot.PXE
FSIMAGE=fs.img

include build/build_env.mak

OBJCOPY= objcopy

LIBS= lib/lib.o

QEMU = qemu-system-i386
QEMUOPTS = -cpu pentium -rtc base=localtime -k fr -m 256M -kernel kernel.bin -drive file=$(FSIMAGE),format=raw,if=ide,index=0,media=disk -device isa-debug-exit
QEMUGDB= -s -S -gdb tcp::1234

DIRS=--directory=kernel --directory=boot --directory=bin --directory=lib

EMACS=emacs

# cible principale, on nettoie tout avant
.PHONY: all
all: clean $(FSIMAGE) kernel.bin

$(FSIMAGE):
	@test -f $(FSIMAGE) || dd if=/dev/zero of=$(FSIMAGE) bs=1M count=8

$(LIBS):
	(cd lib ; make)

kernel/kernel.o:
	(cd kernel ; make kernel.o)

kernel/task_dump_screen.o:
	(cd kernel ; make task_dump_screen.o)

boot/crt0.o:
	(cd boot ; make)

bin/app.o:
	(cd bin ; make)

# generation du noyau
kernel.bin: boot/crt0.o $(LIBS) kernel/task_dump_screen.o bin/app.o kernel/kernel.o
	$(LD) $(LDFLAGS) -e entry -Tboot/kernel.lds $^ -o $@

clean:
	(cd bin ; make clean)
	(cd lib ; make clean)
	(cd kernel ; make clean)
	(cd boot ; make clean)
	rm -f $(KERNEL) core *~
	@# fs.img est conservé pour la persistance

dbg-qemu: kernel.bin
	$(QEMU) $(QEMUOPTS) $(QEMUGDB) &
	$(DEBUG) $(DIRS) $^
	pkill qemu

dbg-vscode: kernel.bin
	$(QEMU) $(QEMUOPTS) $(QEMUGDB) &

dbg: all
	$(EMACS) --eval '(progn (make-term "QEMU" "qemu-system-i386" nil "-s" "-S" "-m" "256M" "-kernel" "kernel.bin" "-display" "curses") (split-window-horizontally) (split-window-vertically) (balance-windows) (gdb "$(DEBUG) $(DIRS) -i=mi kernel.bin") (insert "target remote :1234") (other-window 2) (toggle-frame-fullscreen) (switch-to-buffer "*QEMU*") (other-window -2))'


run: all $(FSIMAGE)
	$(QEMU) $(QEMUOPTS)

archive: clean
	(cd .. ; tar cvf - n7OS | gzip > n7OS_`whoami`.tgz)

help:
	@echo Possible options:
	@echo    all: Build kernel 
	@echo	 run: Build and run 
	@echo    clean: Clean project
	@echo    dbg: gdb in Emacs 
	@echo    dbg-qemu: gdb connected to qemu
	@echo 	 dbg-vscode: gdb connected to qemu
	@echo 	 archive: Archive the project
