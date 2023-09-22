# Makefile to build doom64

N64_SDK       ?= /opt/crashsdk
N64_LIBGCCDIR ?= $(N64_SDK)/lib/gcc/mips64-elf/12.2.0
N64_BINDIR    ?= $(N64_SDK)/bin

TARGET_STRING := doom64
TARGET := $(TARGET_STRING)

DEBUG ?= 0

DEFINES :=
OPTIONS :=

ifdef GDB
    OPTIONS += USB_GDB
    USB = 1
    DEBUG = 1
endif
ifdef DEBUGOPT
    DEFINES += DEBUGOPT=1
    DEBUG = 1
endif
ifdef DEBUG_DISPLAY
    OPTIONS += DEBUG_DISPLAY=$(DEBUG_DISPLAY)
endif
ifdef DEBUG_MEM
    DEBUG = 1
    DEFINES += DEBUG_MEM
endif
ifeq ($(DEBUG),0)
  DEFINES += NDEBUG=1
endif
ifeq ($(SOUND),0)
  OPTIONS += SOUND=0
endif
ifdef FORCE_NO_EXPANSION_PAK
  DEFINES += FORCE_NO_EXPANSION_PAK
endif
ifneq ($(REQUIRE_EXPANSION_PAK),0)
  OPTIONS += REQUIRE_EXPANSION_PAK
endif
ifdef USB
    OPTIONS += USB
endif
ifeq ($(INTRO),0)
  OPTIONS += SKIP_INTRO
endif
ifdef WARP
  OPTIONS += DEVWARP="$(shell printf "%02d" $(WARP))"
endif
ifdef SKILL
  OPTIONS += DEVSKILL=$(SKILL)
endif
ifdef CHEATS
  OPTIONS += DEVCHEATS=$(CHEATS)
endif

ifeq ($(DEBUGOPT),1)
  LIBULTRA_VER=libultra
else
    ifeq ($(DEBUG),1)
      LIBULTRA_VER=libultra_d
    else
      LIBULTRA_VER=libultra_rom
    endif
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
SRC_DIRS += src
ASM_DIR += src/asm
ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS) $(ASM_DIR))

C_FILES           := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES           := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))
BOOT_S_FILES      := $(wildcard $(ASM_DIR)/*.s)

# Files compiled with -Ofast
FAST_C_FILES      := src/i_sram.c
# Files compiled with -Os
SIZE_C_FILES      := $(filter-out $(FAST_C_FILES),$(C_FILES))

# Object files
FAST_O_FILES  := $(foreach file,$(FAST_C_FILES),$(BUILD_DIR)/$(file:.c=.o))
SIZE_O_FILES  := $(foreach file,$(SIZE_C_FILES),$(BUILD_DIR)/$(file:.c=.o))
O_FILES := $(FAST_O_FILES) $(SIZE_O_FILES) \
           $(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
           $(BUILD_DIR)/DOOM64.WAD.o $(BUILD_DIR)/DOOM64.WDD.o \
           $(BUILD_DIR)/DOOM64.WMD.o $(BUILD_DIR)/DOOM64.WSD.o \
           $(BOOT_OBJ)
BOOT_O_FILES := $(foreach file,$(BOOT_S_FILES),$(BUILD_DIR)/$(file:.s=.o))

# Automatic dependency files
DEP_FILES := $(O_FILES:.o=.d) $(BUILD_DIR)/$(LD_SCRIPT).d

LIBDOOM64 = $(BUILD_DIR)/libdoom64.a
LIBULTRA = libultra_modern/build/$(LIBULTRA_VER)/$(LIBULTRA_VER).a

# Make sure build directory exists before compiling anything
$(shell mkdir -p $(ALL_DIRS))
# Ensure submodules are checked out
$(shell if [ ! -d libultra_modern/src ]; then git submodule update -i -r; fi)

define nl


endef

# cache build options
CONFIG_H := $(BUILD_DIR)/config.h
C_OPTIONS = $(foreach d,$(OPTIONS),#define $(subst =, ,$(d))$(nl))

ifeq (,$(wildcard $(CONFIG_H)))
    $(file > $(CONFIG_H),$(C_OPTIONS))
endif
ifneq ($(strip $(file < $(CONFIG_H))),$(strip $(C_OPTIONS)))
    $(file > $(CONFIG_H),$(C_OPTIONS))
endif

DEFINES_TXT := $(BUILD_DIR)/defines.txt

ifeq (,$(wildcard $(DEFINES_TXT)))
    $(file > $(DEFINES_TXT),$(DEFINES))
endif
ifneq ($(strip $(file < $(DEFINES_TXT))),$(strip $(DEFINES)))
    $(file > $(DEFINES_TXT),$(DEFINES))
endif

#==============================================================================#
# Compiler Options                                                             #
#==============================================================================#

AS        := $(N64_BINDIR)/mips-n64-as
CC        := $(N64_BINDIR)/mips-n64-gcc
CPP       := cpp
LD        := $(N64_BINDIR)/mips-n64-ld
AR        := $(N64_BINDIR)/mips-n64-gcc-ar
OBJDUMP   := $(N64_BINDIR)/mips-n64-objdump
OBJCOPY   := $(N64_BINDIR)/mips-n64-objcopy

INCLUDE_DIRS += libultra_modern/include libultra_modern/include/PR $(BUILD_DIR) $(BUILD_DIR)/include src src/asm

DEFINES += _FINALROM=1 F3DEX_GBI_2=1
C_DEFINES := $(foreach d,$(DEFINES),-D$(d))
LD_DEFINES := $(C_DEFINES) $(foreach d,$(OPTIONS),"-D$(d)") -DLIBULTRA=$(LIBULTRA_VER)
DEF_INC_CFLAGS := $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(C_DEFINES)

CFLAGS = -Wall -mno-check-zero-division -march=vr4300 -mtune=vr4300 \
         -D_LANGUAGE_C -D_ULTRA64 -D__EXTENSIONS__ \
         -fno-common -G0 -D_MIPS_SZLONG=32 -D_MIPS_SZINT=32 -g -ggdb -mabi=32 \
         -ffreestanding -fuse-linker-plugin -mfix4300 $(DEF_INC_CFLAGS)
ASFLAGS := -mno-check-zero-division -march=vr4300 -mabi=32 $(foreach i,$(INCLUDE_DIRS),-I$(i))
LDFLAGS :=

SIZE_CFLAGS :=
FAST_CFLAGS :=
LIBULTRA_CFLAGS :=

# $(foreach d,$(DEFINES),--defsym $(d))
#
ifneq (,$(filter 0,$(DEBUG))$(filter 1,$(DEBUGOPT)))
    FAST_CFLAGS += -Ofast -fno-unroll-loops -fno-peel-loops -flto=auto --param case-values-threshold=20 \
                   -fno-inline -finline-functions-called-once --param max-completely-peeled-insns=8
    SIZE_CFLAGS += -Oz -finline-functions-called-once -ffast-math -falign-functions=32 -flto=auto
    LIBULTRA_CFLAGS += -Oz -ffast-math -falign-functions=32 -flto=auto -fuse-linker-plugin
    LDFLAGS += -Wl,--gc-sections -flto=auto -fuse-linker-plugin -Oz
else
    CFLAGS += -Og
    LIBULTRA_CFLAGS += -Og
endif

OPT_CFLAGS :=

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
default: libultra $(ROM)

clean:
	$(RM) -r $(BUILD_DIR) libultra_modern/build

#==============================================================================#
# Compilation Recipes                                                          #
#==============================================================================#

$(BUILD_DIR)/DOOM64.%.o: data/DOOM64.%
	$(call print,Packing data:,$<,$@)
	$(V)$(LD) -r -b binary $< -o $@

# Compile C code

$(FAST_O_FILES): OPT_CFLAGS := $(FAST_CFLAGS)
$(SIZE_O_FILES): OPT_CFLAGS := $(SIZE_CFLAGS)

$(BUILD_DIR)/%.o: %.c $(DEFINES_TXT)
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) $(OPT_CFLAGS) -MMD -MF $(BUILD_DIR)/$*.d  -o $@ $<
$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c $(DEFINES_TXT)
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) $(FAST_CFLAGS) -MMD -MF $(BUILD_DIR)/$*.d  -o $@ $<

# Assemble assembly code
$(BUILD_DIR)/%.o: %.s $(DEFINES_TXT)
	$(call print,Assembling:,$<,$@)
	$(V)$(CC) -c $(ASFLAGS) $(foreach i,$(INCLUDE_DIRS),-Wa,-I$(i)) -x assembler-with-cpp -MMD -MF $(BUILD_DIR)/$*.d  -o $@ $<

# Run linker script through the C preprocessor
$(BUILD_DIR)/$(LD_SCRIPT): src/$(LD_SCRIPT) $(CONFIG_H) $(DEFINES_TXT)
	$(call print,Preprocessing linker script:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -DBUILD_DIR=$(BUILD_DIR) $(LD_DEFINES) -MMD -MP -MT $@ -MF $@.d -o $@ $<

$(BOOT_OBJ): $(BOOT)
	$(V)$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

# Link final ELF file
$(LIBDOOM64): $(O_FILES)
	$(call print,Archiving:,$@)
	$(V)$(AR) rcs -o $@ $(O_FILES)

libultra:
	$(V)$(MAKE) -s -C libultra_modern VERSION=$(LIBULTRA_VER) "EXT_CFLAGS=$(LIBULTRA_CFLAGS)"

$(LIBULTRA): libultra

# Link final ELF file
$(ELF): $(LIBULTRA) $(BOOT_O_FILES) $(LIBDOOM64) $(BUILD_DIR)/$(LD_SCRIPT)
	@$(PRINT) "$(GREEN)Linking ELF file: $(BLUE)$@ $(NO_COL)\n"
	$(V)$(CC) $(LDFLAGS) "-L$(BUILD_DIR)" "-Wl,-T,$(BUILD_DIR)/$(LD_SCRIPT)" "-Wl,-Map,$(BUILD_DIR)/$(TARGET).map" \
		-o $@ $(BOOT_O_FILES) $(LIBDOOM64) $(LIBULTRA) -L$(N64_LIBGCCDIR)

# Build ROM
$(ROM): $(ELF)
	$(call print,Building ROM:,$<,$@)
	$(V)$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $< $@ -O binary
	$(V)makemask $@

.PHONY: clean default libultra
# with no prerequisites, .SECONDARY causes no intermediate target to be removed
.SECONDARY:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
