#!/usr/bin/env python
# 
# ci.py
# 
# (C) 2024.12.27 <hkdywg@163.com>
# 
# This program is free software; you can redistribute it and/r modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
# 
import os
import sys
import shutil
import platform
import requests
import time
import zipfile
class CI:
    def downloadFile(self, name, url):
        headers = {'Proxy-Connection':'keep-alive'}
        r = requests.get(url, stream=True, headers=headers)
        length = float(r.headers['content-length'])
        f = open(name, 'wb')
        count = 0
        count_tmp = 0
        time1 = time.time()
        for chunk in r.iter_content(chunk_size = 512):
            if chunk:
                f.write(chunk)
                count += len(chunk)
                if time.time() - time1 > 2:
                    p = count / length * 100
                    speed = (count - count_tmp) / 1024 / 1024 / 2
                    count_tmp = count
                    sys.stdout.write(f'\rDownload {name} progress: [{p:.2f}%]')
                    sys.stdout.flush()
                    time1 = time.time()
        f.close()
    def extractZipFile(self, zfile, folder):
        # self.delTree(folder)
        if not os.path.exists(folder):
            os.makedirs(folder)
        if platform.system() == 'Linux':
            if zfile.endswith('tar.xz'):
                os.system('tar xvf %s -C %s' % (zfile, folder))
            elif zfile.endswith('tar.bz2'):
                os.system('tar jxvf %s -C %s' % (zfile, folder))
            elif zfile.endswith('.zip'):
                os.system('unzip %s -d %s' % (zfile, folder))
        return
    def zipFolder(self, folder, zfile):
        zip_filename = os.path.join(folder)
        zip = zipfile.ZipFile(zfile, 'w', compression=zipfile.ZIP_BZIP2)
        pre_len = len(os.path.dirname(folder))
        for parent, dirnames, filenames in os.walk(folder):
            for filename in filenames:
                pathfile = os.path.join(parent, filename)
                arcname = pathfile[pre_len:].strip(os.path.sep)
                zip.write(pathfile, arcname)
        zip.close()
        return
    def touchDir(self, d):
        if not os.path.exists(d):
            os.makedirs(d)
if __name__ == '__main__':
    ci = CI()
    env_folder = os.path.abspath(os.path.join('.', 'env_test'))
    # ci.pkgsUpdate(env_folder)
    cmds = '''
        # test
        - dir
        - dir tools
    '''
    ci.run(cmds)
