# Makefile to build doom64

N64_SDK       ?= /opt/crashsdk
N64_LIBGCCDIR ?= $(N64_SDK)/lib/gcc/mips64-elf/12.2.0
N64_BINDIR    ?= $(N64_SDK)/bin

TARGET_STRING := doom64
TARGET := $(TARGET_STRING)

DEBUG ?= 0

DEFINES := _FINALROM=1 F3DEX_GBI_2=1
ifeq ($(DEBUG),0)
  DEFINES += NDEBUG=1
endif

ifneq ($(REQUIRE_EXPANSION_PAK),0)
  DEFINES += REQUIRE_EXPANSION_PAK
endif
ifdef SKIP_INTRO
  DEFINES += SKIP_INTRO
endif
ifdef DEVWARP
  DEFINES += DEVWARP=$(DEVWARP)
endif
ifdef DEVSKILL
  DEFINES += DEVSKILL=$(DEVSKILL)
endif

SRC_DIRS :=

# Whether to hide commands or not
VERBOSE ?= 0
ifeq ($(VERBOSE),0)
  V := @
endif

# Whether to colorize build messages
COLOR ?= 1

 #ifeq (, $(shell which lzop))
     #$(error "No lzop in $(PATH), consider doing apt-get install lzop")
      #endif


#==============================================================================#
# Target Executable and Sources                                                #
#==============================================================================#
# BUILD_DIR is the location where all build artifacts are placed
BUILD_DIR ?= build
ROM            := $(BUILD_DIR)/$(TARGET_STRING).z64
ELF            := $(BUILD_DIR)/$(TARGET_STRING).elf
LD_SCRIPT      := $(TARGET_STRING).ld
BOOT		:= /usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ	:= $(BUILD_DIR)/boot.6102.o

# Directories containing source files
SRC_DIRS += src src/buffers src/asm

C_FILES           := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES           := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))

# Object files
O_FILES := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
           $(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
		   $(BUILD_DIR)/DOOM64.WAD.o $(BUILD_DIR)/DOOM64.WDD.o \
			 $(BUILD_DIR)/DOOM64.WMD.o $(BUILD_DIR)/DOOM64.WSD.o \
		   $(BOOT_OBJ)

# Automatic dependency files
DEP_FILES := $(O_FILES:.o=.d) $(ASM_O_FILES:.o=.d)  $(BUILD_DIR)/$(LD_SCRIPT).d

#==============================================================================#
# Compiler Options                                                             #
#==============================================================================#

AS        := $(N64_BINDIR)/mips-n64-as
CC        := $(N64_BINDIR)/mips-n64-gcc
CPP       := cpp
LD        := $(N64_BINDIR)/mips-n64-ld
AR        := $(N64_BINDIR)/mips-n64-ar
OBJDUMP   := $(N64_BINDIR)/mips-n64-objdump
OBJCOPY   := $(N64_BINDIR)/mips-n64-objcopy

INCLUDE_DIRS += /usr/include/n64 /usr/include/n64/PR $(BUILD_DIR) $(BUILD_DIR)/include src src/asm

C_DEFINES := $(foreach d,$(DEFINES),-D$(d))
DEF_INC_CFLAGS := $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(C_DEFINES)

CFLAGS = -Wall -mno-check-zero-division -march=vr4300 -mtune=vr4300 \
         -D_LANGUAGE_C -D_ULTRA64 -D__EXTENSIONS__ \
         -fno-common -G0 -D_MIPS_SZLONG=32 -D_MIPS_SZINT=32 -g -mabi=32 \
         -ffreestanding -mfix4300 $(DEF_INC_CFLAGS)
ASFLAGS     := -mno-check-zero-division -march=vr4300 -mabi=32 $(foreach i,$(INCLUDE_DIRS),-I$(i))
# $(foreach d,$(DEFINES),--defsym $(d))
#
ifeq ($(DEBUG),0)
  #CFLAGS += -Ofast -fno-unroll-loops -fno-peel-loops --param case-values-threshold=20 -fno-inline -finline-functions-called-once --param max-completely-peeled-insns=8
  CFLAGS += -Os -finline-functions-called-once -ffast-math -falign-functions=32
endif

# C preprocessor flags
CPPFLAGS := -P -Wno-trigraphs $(DEF_INC_CFLAGS)

# tools
PRINT = printf

ifeq ($(COLOR),1)
NO_COL  := \033[0m
RED     := \033[0;31m
GREEN   := \033[0;32m
BLUE    := \033[0;34m
YELLOW  := \033[0;33m
BLINK   := \033[33;5m
endif

# Common build print status function
define print
  @$(PRINT) "$(GREEN)$(1) $(YELLOW)$(2)$(GREEN) -> $(BLUE)$(3)$(NO_COL)\n"
endef

#==============================================================================#
# Main Targets                                                                 #
#==============================================================================#

# Default target
default: $(ROM)

clean:
	$(RM) -r $(BUILD_DIR)

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))

# Make sure build directory exists before compiling anything
DUMMY != mkdir -p $(ALL_DIRS)

#==============================================================================#
# Compilation Recipes                                                          #
#==============================================================================#

$(BUILD_DIR)/DOOM64.%.o: data/DOOM64.%
	$(call print,Packing data:,$<,$@)
	$(V)$(LD) -r -b binary $< -o $@

# Compile C code
$(BUILD_DIR)/%.o: %.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) -MMD -MF $(BUILD_DIR)/$*.d  -o $@ $<
$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) -MMD -MF $(BUILD_DIR)/$*.d  -o $@ $<

# Assemble assembly code
$(BUILD_DIR)/%.o: %.s
	$(call print,Assembling:,$<,$@)
	$(V)$(CC) -c $(ASFLAGS) $(foreach i,$(INCLUDE_DIRS),-Wa,-I$(i)) -x assembler-with-cpp -MMD -MF $(BUILD_DIR)/$*.d  -o $@ $<

# Run linker script through the C preprocessor
$(BUILD_DIR)/$(LD_SCRIPT): src/$(LD_SCRIPT)
	$(call print,Preprocessing linker script:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -DBUILD_DIR=$(BUILD_DIR) $(C_DEFINES) -MMD -MP -MT $@ -MF $@.d -o $@ $<

$(BOOT_OBJ): $(BOOT)
	$(V)$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

# Link final ELF file
$(ELF): $(O_FILES) $(BUILD_DIR)/$(LD_SCRIPT)
	@$(PRINT) "$(GREEN)Linking ELF file: $(BLUE)$@ $(NO_COL)\n"
	$(V)$(LD) -L $(BUILD_DIR) -T $(BUILD_DIR)/$(LD_SCRIPT) -Map $(BUILD_DIR)/$(TARGET).map -o $@ $(O_FILES) \
        --no-check-sections -L/usr/lib/n64 -lultra_rom -L$(N64_LIBGCCDIR) -lgcc

# Build ROM
$(ROM): $(ELF)
	$(call print,Building ROM:,$<,$@)
	$(V)$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $< $@ -O binary
	$(V)makemask $@

.PHONY: clean default
# with no prerequisites, .SECONDARY causes no intermediate target to be removed
.SECONDARY:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
