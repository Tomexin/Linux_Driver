#子Makefile：用来告诉make如何生成当前目录下的子目标built-in.o

SRCS = tst.c
SUB_DIR = foo
SUB_TGT = built_in.o

#下面的规则说明如何生成当前目录下的子目标（是由当前目录下的.c生成的.o
#和当前目录下子目录中的子目标临时打包生成的）
$(SUB_TGT):$(SRCS:.c=.o) $(SUB_DIR)
	$(LD) $(LDFLAGS) $(SRCS:.c=.o) $(SUB_DIR:=/$(SUB_TGT)) -r -o $@


%.o:%.c
	$(CC) $(CFLAGS) $< -c

%.d:%.c
	$(CC) $(CFLAGS) $< -MM > $@
sinclude $(SRCS:.c=.d)

$(SUB_DIR):
	make -C $@

clean:
	rm -vf $(SUB_TGT) $(SRCS:.c=.o) $(SRCS:.c=.d) 
	for dir in $(SUB_DIR); do\
		make -C $$dir clean;\
	done

.PHONY:clean $(SUB_DIR)
