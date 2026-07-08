# 定义最终生成的内核模块名称，构建结果为 procscope.ko。
MODULE_NAME := procscope

# 将属于同一个 procscope.ko 的多个目标文件列在这里，Kbuild 会把它们链接成一个模块。
# main.o : init/exit
# procfs.o: 负责procfs文件展示
# chrdev.o: 负责注册字符设备及其操作函数
# ioctl.o: 负责设备字符文件的ioctl.o命令
$(MODULE_NAME)-y := main.o procfs.o chrdev.o ioctl.o

# 告诉 Kbuild 将 procscope.o 作为外部模块目标进行构建。
obj-m := $(MODULE_NAME).o

# 为当前目录下的模块源码追加编译警告选项。
# ccflags-y += -Wall -Wextra
