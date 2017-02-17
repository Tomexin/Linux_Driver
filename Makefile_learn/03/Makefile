TGT = a.out
SRCS = test.c tst.c
#OBJ = test.o tst.o
CC = gcc

#$(TGT):$(OBJ)
#	$(CC) $^ -o $@

$(TGT):$(SRCS:.c=.o)
	$(CC) $^ -o $@

#test.o: test.c
#	$(CC) $^ -c -o $@

#tst.o: tst.c
#	$(CC) $^ -c -o $@

#test.o tst.o: test.c tst.c

#以下为模式规则
#%.o: %.c
#	$(CC) $< -c 


clean:
	rm -f $(TGT) $(SRCS:.c=.o)
