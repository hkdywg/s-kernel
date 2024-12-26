#
# Global Makefile
# Author: hkdywg <hkdywg@163.com>
#
# Copyright (C) 2024 kevin
#
#

VERSION = 1
PATCHLEVEL = 0
SUBLEVEL = 0

MAKEFLAGS += -rR --no-print-directory

# Use `make V=1` to see full commands

ifeq ("$(origin V)", "command line")
	KBUILD_VERBOSE = $(V)
endif


# Default target
PHONY := _all
_all:

# if want to locate output file in a separate directory, can use `make O=outputdir`
ifeq ("$(origin O)", "command line")
	KBUILD_OUTPUT := $(O)
endif

ifneq ($(KBUILD_OUTPUT),)
saved-output := $(KBUILD_OUTPUT)
endif

srctree 	:= $(CURDIR)
objtree 	:= $(CURDIR)
src 		:= $(srctree)
obj 		:= $(objtree)

VPATH := $(srctree)

export srctree objtree VPATH

-include include/config/auto.conf

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

INCLUDE 	:= -Iinclude \
					$(if $(KBUILD_SRC), -I$(srctree)/include) \
					-include include/generated/autoconf.h


ARCH := $(subst ",,$(CONFIG_ARCH))

export ARCH SRCARCH CONFIG_SHELL HOSTCC HOSTCFLAGS CROSS_COMPILE AS LD CC
export CPP AR NM STRIP OBJCOPY OBJDUMP
export MAKE AWK GENKSYMS INSTALLKERNEL PERL UTS_MACHINE
export HOSTCXX HOSTCXXFLAGS

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

%config: scripts_basic outputmakefile FORCE
	$(Q)mkdir -p include/config
	$(Q)$(MAKE) $(build)=scripts/kconfig $@


_all:all

-include boot/Makefile
-include common/Makefile
-include driver/Makefile
-include fs/Makefile
-include init/Makefile
-include mem/Makefile
-include task/Makefile

# The all:target is the default when no target is given on the command line
all: $(SUB_TARGET)
	@[ -d $(srctree)/dl ] || mkdir -p $(srctree)/dl
	@echo "build default target: $(SUB_TARGET)"

dist_clean:
	@rm build_out/ -rf

clean:
	@rm build_out/ -rf

PHONY += help

help:
	@echo "srctree is $(srctree)"
	@echo "CONFIG_SHELL is $(CONFIG_SHELL)"


