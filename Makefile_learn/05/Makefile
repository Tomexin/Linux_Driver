#主Makefile作用：在当前目录制定规则来说明如何在当前目录下生成终极目标文件a.out

TGT = a.out
#指定子目录(生成a.out会依赖这些目录)
SUB_DIR = main tst

#指定字母表
export SUB_TGT = built_in.o

#指定当前目录
export TOP_DIR = $(shell pwd)
export HEAD = /head
#指定头文件目录
export HEAD_DIR = $(TOP_DIR)$(HEAD)
#CROSS_COMPILER = arm-linux-
export CC = $(CROSS_COMPILER)gcc
#编译选项
export CFLAGS = -I $(HEAD_DIR) -Wall
#指定链接器
export LD = ld
#指定链接选项
export LD_FLAGS = 


# 终极目标所在规则
$(TGT) : $(SUB_DIR)
	$(CC) $(CFLAGS) $(^:=/$(SUB_TGT))

#下面的规则用来告诉make，需要进入到生成a.out所依赖的那些子目录中
$(SUB_DIR):
	make -C $@
#-C选项可以让make进入到后面指定的目录


clean:
	rm -vf $(TGT)
	for dir in $(SUB_DIR); do	\
		make -C $$dir clean; \
	done

.PHONY: clean $(SUB_DIR)

