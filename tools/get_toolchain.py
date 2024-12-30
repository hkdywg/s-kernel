#!/usr/bin/env python
#
# get_toolchain.py
# 
# (C) 2024.12.27 <hkdywg@163.com>
# 
# This program is free software; you can redistribute it and/r modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

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

env_variables = {
    'COMPILE_TOOL_PATH' : 'toolchain_path',
    'PATH' : '$PATH:$COMPILE_TOOL_PATH',
    'CROSS_COMPILE' : 'cross_comile'
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
    ci.downloadFile(zfile, URL)
    ci.extractZipFile(zfile, toolchain_path)
    env_variables['COMPILE_TOOL_PATH'] = toolchain_path + '/gcc-linaro-7.4.1-2019.02-x86_64_aarch64-linux-gnu/bin'
    env_variables['CROSS_COMPILE'] = 'aarch64-linux-gnu-'
    ci.generate_env_file(env_variables,'env_config.sh')

