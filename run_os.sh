#!/bin/bash

# Run os by qemu.
#
# (C) 2024.12.25 hkdywg <hkdywg@163.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

LINK_PATH=/opt/poky/2.4.3/sysroots/aarch64-poky-linux
OS_KERNEL=build_out/init/kernel

qemu-aarch64 -L ${LINK_PATH} ${OS_KERNEL}
