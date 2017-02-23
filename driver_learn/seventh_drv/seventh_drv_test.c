#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

/*
 *seventh_drv_test
 * */

int fd;
unsigned char key_val;
#if 0
void my_signal_fun(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
	printf("-----------------------\n");
	printf("key_val : 0x%x\n", key_val);
}
#endif

int main(int argc, char *argv[])
{
	unsigned char key_val = 0;
	int pid = 0;
	int oflags = 0;
	int ret;

	//signal(SIGIO, my_signal_fun);		//应用程序注册信号处理函数

	fd = open("/dev/seventh_drv", O_RDWR);
	if(fd<0)
	{	
		printf("open seventh_drv fail\n");
		return -1;
	}

	// fcntl(fd, F_SETOWN, (pid=getpid()));	//通知内核要将信号发给那个进程
	// oflags = fcntl(fd, F_GETFL);			//
	// fcntl(fd, F_GETFL, oflags | FASYNC);	//
	//printf("The pid of process is %d\n", pid);

	while(1)
	{
		ret = read(fd, &key_val, 1);
		printf("key_val : 0x%x; ret : %d\n", key_val, ret);
		//sleep(5);
	}

	return 0;
}

