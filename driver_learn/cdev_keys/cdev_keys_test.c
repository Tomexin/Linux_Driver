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
	unsigned char key_vals[4] = {0};
	fd = open("/dev/cdev_keys", O_RDWR);
	if(fd<0)
	{	
		printf("open cdev_keys fail\n");
		return 0;
	}
	while(1)
	{
		read(fd, key_vals, sizeof(key_vals));
		if(!(key_vals[0] && key_vals[1] && key_vals[2] && key_vals[3]))
		{
			printf("Some of the keys have been pressed:%s %s %s %s\n", key_vals[0],\
				key_vals[1], key_vals[2], key_vals[3]);
		}
	}

	return 0;

}

