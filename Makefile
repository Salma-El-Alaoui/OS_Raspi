ARMGNU ?= arm-none-eabi

CFLAGS = -Wall -nostdlib -fomit-frame-pointer -mno-apcs-frame -nostartfiles -ffreestanding -g -march=armv6z -marm -mthumb-interwork
ASFLAGS = -g -march=armv6z

C_FILES=kernel.c
SCHEDULER_FILES = $(addprefix preemptive-scheduler/,phyAlloc.c hw.c sched.c)
C_FILES+= $(SCHEDULER_FILES)
MMU_FILES = $(addprefix mmu/,vmem.c)
C_FILES+= $(MMU_FILES)
SYSCALLS_FILES = $(addprefix syscalls/,syscall.c)
C_FILES+= $(SYSCALLS_FILES)
SOUND_FILES = $(addprefix sound-system/,pwm.c tune.wav)
C_FILES+= $(SOUND_FILES)
AS_FILES=vectors.s

SRC_DIR = src/
BIN_DIR = bin/
OBJ = $(patsubst %.s,%.o,$(AS_FILES))
OBJ += $(patsubst %.wav,%.o,$(patsubst %.c,%.o,$(C_FILES)))
MEMMAP = $(SRC_DIR)memmap

OBJS = $(addprefix $(BIN_DIR),$(OBJ))

.PHONY: gcc clean

gcc : kernel

clean :
	@rm -R -f $(BIN_DIR)

$(BIN_DIR)%.o : $(SRC_DIR)%.wav
	$(ARMGNU)-ld -s -r -o $@ -b binary $^


$(BIN_DIR)%.o : $(SRC_DIR)%.c
	mkdir -p $(dir $@)
	$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@

$(BIN_DIR)%.o : $(SRC_DIR)%.s
	mkdir -p $(dir $@)
	$(ARMGNU)-as $(ASFLAGS) $< -o $@

kernel : $(MEMMAP) $(OBJS)
	$(ARMGNU)-ld $(OBJS) -T $(MEMMAP) -o $(BIN_DIR)kernel.elf
	$(ARMGNU)-objdump -D $(BIN_DIR)kernel.elf > $(BIN_DIR)kernel.list
	$(ARMGNU)-objcopy $(BIN_DIR)kernel.elf -O binary $(BIN_DIR)kernel.img
	$(ARMGNU)-objcopy $(BIN_DIR)kernel.elf -O ihex $(BIN_DIR)kernel.hex

