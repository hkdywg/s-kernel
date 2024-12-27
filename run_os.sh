#!/bin/bash

# Run os by qemu.
#
# (C) 2024.12.25 hkdywg <hkdywg@163.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.


QEMU=/home/yinwg/ywg_workspace/prj/LinuxLab/workspace/qemu/build/aarch64-softmmu/qemu-system-aarch64
KERNEL_IMAGE=build_out/init/kernel
ROOT_FS_IMG=build_out/root_fs.img
RAM_SIZE=256
CORE_TYPE=cortex-a53
CORE_NUM=2

if [ ! -f ${ROOT_FS_IMG} ];then
	dd if=/dev/zero of=${ROOT_FS_IMG} bs=1M count=128
	mkfs.fat ${ROOT_FS_IMG}
fi

sudo ${QEMU} \
    -M virt \
    -m ${RAM_SIZE} \
    -cpu ${CORE_TYPE} \
    -smp ${CORE_NUM} \
    -kernel ${KERNEL_IMAGE} \
    -nographic 

