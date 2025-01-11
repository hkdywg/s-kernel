#!/bin/bash

# Run os by qemu.
#
# (C) 2024.12.25 hkdywg <hkdywg@163.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.


QEMU=qemu-system-aarch64
KERNEL_IMAGE=build_out/kernel
RAM_SIZE=256
CORE_TYPE=cortex-a53
CORE_NUM=1

if [ -z $(which qemu-system-aarch64) ];then
	echo "please install qemu-system-aarch64 tools"
	echo "in ubuntu system, can execute the fllowing command to install it"
	echo "sudo apt update"
	echo "sudo apt install qemu-system-aarch64"
	exit 0
fi

#
# if use "-d" option, another terminal can use
# "aarch64-linux-gnu-gdb path/kernel" to debug kernel
# @note: use "target remote :1234" in the gdb terminal to connect kernel
#
if [ $# -eq 1 ];then
if [ $1 == "-d" ];then
sudo ${QEMU} \
    -M virt \
    -m ${RAM_SIZE} \
    -cpu ${CORE_TYPE} \
    -smp ${CORE_NUM} \
    -kernel ${KERNEL_IMAGE} \
    -nographic \
	-s -S
fi
else
sudo ${QEMU} \
    -M virt \
    -m ${RAM_SIZE} \
    -cpu ${CORE_TYPE} \
    -smp ${CORE_NUM} \
    -kernel ${KERNEL_IMAGE} \
    -nographic 
fi

