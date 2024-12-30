#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2022, RT-Thread Development Team
#
# SPDX-License-Identifier: GPL-2.0
#
# Change Logs:
# Date           Author       Notes
# 2022-02-1      Bernard      The first version
#
import os
import sys
import platform
from ci import CI
toolchains_config = {
    'aarch64':
    {
        'Linux' : 'gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu.tar.xz',
    },
}
if __name__ == '__main__':
    # download toolchain
    if len(sys.argv) > 1:
        target = sys.argv[1]
    else:
        target = 'aarch64'
    ci = CI()
    toolchain_path = os.path.join(os.path.abspath('.'), 'gnu_gcc')
    platform = platform.system()
    try:
        zfile = toolchains_config[target][platform]
        URL = 'https://releases.linaro.org/components/toolchain/binaries/7.4-2019.02/aarch64-linux-gnu/' + zfile
    except:
        print('not found target')
        exit(0)
    #ci.downloadFile(zfile, URL)
    ci.extractZipFile(zfile, toolchain_path)
    print("{zfile}")
    ci.delFile(zfile)

