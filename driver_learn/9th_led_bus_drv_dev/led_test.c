#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/*
 *led_test on
 *led_test off
 * */
int main(int argc, char *argv[])
{
	int fd;
	int val = 1;
	fd = open("/dev/myled", O_RDWR);
	if(fd<0)
	{	
		printf("open myled fail\n");
		return 0;
	}
	if(argc != 2)
	{
		printf("Usage:\n");
		printf("%s <on|off>\n", argv[0]);
		return -1;
	}

	if(strcmp(argv[1], "on") == 0)
	{
		val = 1;
	}
	else
	{
		val = 0;
	}
	write(fd, &val, 4);

	return 0;

}

