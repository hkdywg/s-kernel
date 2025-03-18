#
# Global Makefile
# Author: hkdywg <hkdywg@163.com>
#
# Copyright (C) 2024 kernel
#
#

VERSION = 1
PATCHLEVEL = 0
SUBLEVEL = 0

# Default target
PHONY := _all
_all:

MAKEFLAGS += -rR --no-print-directory

# Use `make V=1` to see full commands

ifeq ("$(origin V)", "command line")
	KBUILD_VERBOSE = $(V)
endif

# if want to locate output file in a separate directory, can use `make O=outputdir`
ifeq ($(KBUILD_SRC),)
ifeq ("$(origin O)", "command line")
	KBUILD_OUTPUT := $(O)
endif
endif

ifneq ($(KBUILD_OUTPUT),)
saved-output := $(KBUILD_OUTPUT)
abs_objtree := $(shell mkdir -p $(KBUILD_OUTPUT) && cd $(KBUILD_OUTPUT) && pwd)
$(if $(abs_objtree),,\
	$(error failed to create output directory "$(KBUILD_OUTPUT)"))
_all:
	$(if $(KBUILD_VERBOSE:1=),@,@)$(MAKE) -C $(KBUILD_OUTPUT) \
	KBUILD_SRC=$(KBUILD_OUTPUT)  KBUILD_VERBOSE=$(KBUILD_VERBOSE) \
    -f $(CURDIR)/Makefile $@
skip-makefile := 1
endif

ifeq ($(skip-makefile),)

CUR_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

srctree 	:= $(CUR_DIR)
objtree 	:= $(CUR_DIR)
src 		:= $(srctree)
obj 		:= $(objtree)
VPATH := $(srctree)


export srctree objtree VPATH MAKEFILE_DIR

# SHELL config
CONFIG_SHELL := $(shell if [ -x "$$BASH" ];then echo $$BASH;\
	else if [ -x /bin/bash ];then echo /bin/bash; \
	else echo sh; fi; fi)

# Host make variables ...
HOSTCC 		= gcc
HOSTCXX 	= g++
HOSTCFLAGS 	= -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer
HOSTCXXFLAGS = -O2

# Crosscompile variables ...

CC 			= $(CROSS_COMPILE)gcc
AS 			= $(CROSS_COMPILE)as
LD 			= $(CROSS_COMPILE)ld
CPP 		= $(CC) -E
AR 			= $(CROSS_COMPILE)ar
NM 			= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY 	= $(CROSS_COMPILE)objcopy
OBJDUMP 	= $(CROSS_COMPILE)objdump
AWK			= awk
INSTALLKERNEL 	:= installkernel
PERL 		= perl

CHECKFLAGS 	:= -D__linux__ -Dlinux -D__STDC__ -Dunix -D__unix__ \
				-Wbitwise -Wno-return-void $(CF)
CFLAGS_KERNEL 	=
AFLAGS_KERNEL 	=

KERNEL_INCLUDE 	:= -I$(srctree)/include/config \
					-I$(srctree)/include/kernel \
					-I$(srctree)/include/driver \
					-I$(srctree)/include/common \
					-I$(srctree)/include/fs \
					-I$(srctree)/bsp/include \
					-I$(srctree)/arch/aarch64/include

NOSTDINC_FLAGS = -nostdinc -isystem $(shell $(CC) -print-file-name=include)


ARCH := $(subst ",,$(CONFIG_ARCH))

export ARCH SRCARCH CONFIG_SHELL HOSTCC HOSTCFLAGS CROSS_COMPILE AS LD CC
export CPP AR NM STRIP OBJCOPY OBJDUMP
export MAKE AWK GENKSYMS INSTALLKERNEL PERL UTS_MACHINE
export HOSTCXX HOSTCXXFLAGS NOSTDINC_FLAGS KERNEL_INCLUDE

ifeq ($(KBUILD_VERBOSE),1)
	quiet =
	Q = 
else
	quiet = quiet_
	Q = @
endif

export quiet Q KBUILD_VERBOSE

# Look for make include files relative to root fo kernel src
#MAKEFLAGS += --include-dir=$(srctree)

PHONY += FORCE

FORCE:

SUB_TARGET := 

export SUB_TARGET

_all:all

init-y 		:= init/ arch/
core-y 		:= thread/ mem/ ipc/
fs-y 		:= fs/
driver-y 	:= driver/
common-y 	:= common/
user-y 		:= user/
bsp-y 		:= bsp/

build-dirs := $(patsubst %/,%,$(filter %/, $(init-y) $(core-y) $(fs-y) $(driver-y) $(common-y) $(user-y) $(bsp-y)))

init-y 		:= $(patsubst %/, %/built-in.a, $(init-y))
core-y 		:= $(patsubst %/, %/built-in.a, $(core-y))
fs-y 		:= $(patsubst %/, %/built-in.a, $(fs-y))
driver-y 	:= $(patsubst %/, %/built-in.a, $(driver-y))
common-y 	:= $(patsubst %/, %/built-in.a, $(common-y))
user-y 		:= $(patsubst %/, %/built-in.a, $(user-y))
bsp-y 		:= $(patsubst %/, %/built-in.a, $(bsp-y))

export KBUILD_KERNEL_OBJS := $(init-y) $(core-y) $(fs-y) $(driver-y) $(common-y) $(user-y) $(bsp-y)

# Shorthand for $(Q)$(MAKE) -f scripts/Makefile.build obj=dir
# Usage:
# $(Q)$(MAKE) $(build)=dir
build := -f $(srctree)/scripts/Makefile.build obj

# Link scripts
link_script := $(srctree)/scripts/link.lds

kernel-deps := $(KBUILD_KERNEL_OBJS)

cmd_link_kernel = $(LD)  -o $@ $^ -T $(link_script) -Map=kernel.map

kernel: $(kernel-deps) 
	$(Q)$(cmd_link_kernel)
	@echo "build kernel done"

# The actual objects are generate
$(sort $(kernel-deps)): $(build-dirs)

# Handle descending into subdirectories listed in $(build-dirs)
# Preset locale variables to speed up the build process. Limit locale
# tweaks to this spot to avoid wrong language settings when running
# Error messages still appears in the original language
$(build-dirs): FORCE
	@$(Q)$(MAKE) $(build)=$@ need-builtin=1

# The all:target is the default when no target is given on the command line
all: kernel $(SUB_TARGET)

# Files to ignore in find ... statements
FIND_IGNORE := ! -path "./tools/*"

clean:
	@find . $(FIND_IGNORE) -name '*.[oas]' -type f -print | xargs rm -f

PHONY += help kernel

help:
	@echo 'clean 	- Remove all generate files'
	@echo 'all 		- Build all targets'
	@echo 'kernel 	- Build the kernel'

endif
