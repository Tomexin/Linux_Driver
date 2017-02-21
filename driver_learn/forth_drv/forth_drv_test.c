#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>

/*
 *forth_drv_test
 * */
int main(int argc, char *argv[])
{
	int fd;
	unsigned char key_val = 0;
	int ret;
	struct pollfd fds[1];		//可以监控多个驱动程序，这里只监控一个
	nfds_t nfds;
	int timeout = 5000;

	fd = open("/dev/forth_drv", O_RDWR);
	if(fd<0)
	{	
		printf("open forth_drv fail\n");
		return 0;
	}

	fds[0].fd 	  = fd;
	fds[0].events = POLLIN;
	while(1)
	{
		ret = poll(fds, 1, timeout);
		if(ret == 0)
		{
			printf("timeout: %d\n", timeout);
		}
		else
		{
			read(fd, &key_val, 1);
			printf("key_val = 0x%x\n", key_val);
		}
	}

	return 0;

}

