##
# By default everything is silent. If you want to change this behavior, simply
# assign V=1 when calling make.

V =
ifeq ($(strip $(V)),)
	E = @echo
	Q = @
else
	E = @\#
	Q =
endif

##
# Paths.

SRC            = $(wildcard src/*.S src/*.c)
OBJ            = $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(SRC)))
EXE            = init
ARCH           = riscv
LINUX_SOURCE  ?=
KERNEL         = $(LINUX_SOURCE)/arch/$(ARCH)/boot/Image

##
# Compile options. You can use CROSS_COMPILE just like on the Linux Kernel. You
# can also determine the ISA to be used, which defaults to my VisionFive2 board.
# I did not go too much into the rabbit hole of platform-specific flags because
# everything is in assembly anyways and everything is statically linked. Hence
# no `-mcpu`, no `-mtune`, no funny business.

CC        = $(CROSS_COMPILE)gcc
LD        = $(CROSS_COMPILE)gcc
ISA      ?= rv64imafdc_zicntr_zicsr_zifencei_zihpm_zca_zcd_zba_zbb
INCLUDES  = -Iinclude/
ASFLAGS   = -march=$(ISA) -mabi=lp64d $(INCLUDES)
CCFLAGS   = $(ASFLAGS) -std=gnu17
WARNINGS  = -Werror -Wall -Wextra -Wcast-align -Wcast-qual -Winit-self \
            -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wsign-conversion \
            -Wswitch-default -Wundef -Wunreachable-code
CCFLAGS  += $(WARNINGS)
LDFLAGS   = -static $(INCLUDES)

##
# Optional parameters for QEMU and gdb.

QEMU_FLAGS      ?=
GDB_EXTRA_FLAGS ?=

# You can pass an optional `DEBUG` variable to manipulate the build type.
DEBUG ?=
ifeq ($(strip $(DEBUG)),)
	CCFLAGS += -O3
else
	CCFLAGS += -g -O0 -D__DEBUG__
endif

# Image being mounted for QEMU's consumption.
MOUNT_DEST_DIR ?= /mnt/pudc
MOUNT_DEST_IMG ?= disk.img

##
# Targets

.PHONY: all
all: clean $(EXE)

.PHONY: $(EXE)
$(EXE): $(OBJ)
	$(E) "	LD	" $@
	$(Q) $(LD) $(LDFLAGS) $+ -o $(EXE)

.c.o:
	$(E) "	CC	" $(basename $@)
	$(Q) $(CC) $(CCFLAGS) -c $< -o $@

.S.o:
	$(E) "	CC	" $(*F)
	$(Q) $(CC) $(ASFLAGS) -c $< -o $@

# Ensure that we have LINUX_SOURCE defined.
.PHONY: qemu_env
qemu_env:
	$(if $(wildcard $(LINUX_SOURCE)),,$(error The 'LINUX_SOURCE' environment variable must be set to a non-empty value.))

# Insert the built $(EXE) into the image to be mounted via QEMU.
.PHONY: qemu_prepare_disk
qemu_prepare_disk:
	$(E) "	DISK	 $(MOUNT_DEST_IMG)"
	$(Q) sudo mkdir -p $(MOUNT_DEST_DIR)
	$(Q) sudo mount -o loop $(MOUNT_DEST_IMG) $(MOUNT_DEST_DIR)
	$(Q) sudo mkdir -p $(MOUNT_DEST_DIR)/bin/ $(MOUNT_DEST_DIR)/dev/ \
		$(MOUNT_DEST_DIR)/etc/ $(MOUNT_DEST_DIR)/proc/ $(MOUNT_DEST_DIR)/run/pudc/ \
		$(MOUNT_DEST_DIR)/sbin/ $(MOUNT_DEST_DIR)/sys/ $(MOUNT_DEST_DIR)/tmp/
	$(Q) sudo cp $(EXE) $(MOUNT_DEST_DIR)/sbin/$(EXE)
	$(Q) sudo umount $(MOUNT_DEST_DIR)

# Run QEMU by using the built $(KERNEL) and the $(MOUNT_DEST_IMG), which
# contains the $(EXE) binary as '/sbin/init'.
.PHONY: qemu
qemu: qemu_env $(EXE) qemu_prepare_disk
	$(E) "	QEMU	 kernel=$(KERNEL)"
	$(Q) qemu-system-riscv64 -machine virt -smp 4 -m 8G -kernel $(KERNEL) \
		-nographic $(QEMU_FLAGS) \
		-drive file=$(MOUNT_DEST_IMG),format=raw,if=virtio \
		-append "root=/dev/vda rw console=ttyS0"

.PHONY: clean
clean:
	$(Q) rm -f $(OBJ) $(EXE)

.PHONY: lint
lint:
	$(Q) git ls-files *.c *.h | xargs clang-format --dry-run --Werror
