#a.out:test.c tst.c
#	gcc test.c tst.c

#变量定义
TGT = a.out
SRCS = test.c tst.c

$(TGT):$(SRCS)
	@gcc $^ -o $@

#下面的规则用来清理之前生成的文件，以便下一次编译
clean:
	@rm -f $(TGT)
