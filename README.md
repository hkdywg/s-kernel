<p align="center">
<img src="./doc/logo.png">
</p>

# s-kernel 说明
		更新日期: 2025-03-08

## 简介

**s-kernel** 是一个小型的操作系统内核，用于学习操作系统内核原理。

----------------------------------------------------------------------

**功能特性**

- 支持时间片轮转式线程切换

- 实现了大块内存页分配机制及小块内存slab机制

- 支持shell命令终端

- 支持kernel定时器等功能特性

- 支持mutex等IPC通信方式


## 使用

- 安装qemu

```
sudo apt update
sudo apt install qemu-system-aarch64
```

- 下载编译工具链

```
cd tools
python3 ./get_toolchain.py aarch64
source ./env_config.sh
```

- 编译s-kernel

```
make 
```

- 运行s-kernel

```
./run_os.sh
```

