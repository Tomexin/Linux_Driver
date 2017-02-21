#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

/*
 *fifth_drv_test
 * */

int fd;

void my_signal_fun(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
	printf("-----------------------\n");
	printf("key_val : 0x%x\n", key_val);
}

int main(int argc, char *argv[])
{
	unsigned char key_val = 0;
	int pid = 0;
	int oflags = 0;
	int ret;

	signal(SIGIO, my_signal_fun);		//应用程序注册信号处理函数

	fd = open("/dev/fifth_drv", O_RDWR);
	if(fd<0)
	{	
		printf("open fifth_drv fail\n");
		return 0;
	}

	fcntl(fd, F_SETOWN, (pid=getpid()));			//通知内核要将信号发给那个进程
	oflags = fcntl(fd, F_GETFL);			//
	fcntl(fd, F_GETFL, oflags | FASYNC);	//

	printf("The pid of process is %d\n", pid);

	while(1)
	{
		sleep(1000);
	}

	return 0;
}

