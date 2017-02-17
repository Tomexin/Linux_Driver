#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/*
 *second_drv_test
 * */
int main(int argc, char *argv[])
{
	int fd;
	unsigned char key_val = 0;
	fd = open("/dev/third_drv", O_RDWR);
	if(fd<0)
	{	
		printf("open third_drv fail\n");
		return 0;
	}
	while(1)
	{
		read(fd, &key_val, 1);
		printf("key_val = 0x%x\n", key_val);
	}

	return 0;

}

