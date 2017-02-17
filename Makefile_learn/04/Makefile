TGT = a.out
#指定交叉编译环境
SRCS = main.c tst.c foo.c
#CROSS_COMPILER = arm-linux-	
CC = $(CROSS_COMPILER)gcc
HEAD = /head
#指定当前头文件目录
CUR_DIR = $(shell pwd)
HEAD_DIR = $(CUR_DIR)$(HEAD)
#指定编译选项,这里指定的是头文件
CFLAGS = -I $(HEAD_DIR)
$(TGT): $(SRCS:.c=.o)
	$(CC) $^ $(CFLAGS) -o $@

%.o:%.c
	$(CC) $< -c

#下面的规则用来说明，头文件的更新应该使得依赖他的文件更新
%.d: %.c
	$(CC) -MM $< > $@
#下面三行功能一样，推荐使用sinclude，用来告诉make，将上面的模式规则中的命令执行的结果包含进当前文件
#include $(SRCS:.c=.d)
#-include $(SRCS:.c=.d)
sinclude $(SRCS:.c=.d)

clean:
	rm -f $(TGT) $(SRCS:.c=.o) $(SRCS:.c=.d)
#.PHONY:后面声明的是伪目标
#伪目标：不是一个真正的目标，仅仅是为了执行其下面的命令，不应该让make判断他是否存在，或者是否应该
#被执行
.PHONY: clean

