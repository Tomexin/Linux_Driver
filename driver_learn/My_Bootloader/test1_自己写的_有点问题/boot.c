#include "setup.h"

extern void uart0_init(void);
extern void putstr(char *str);
extern void nand_read(unsigned int src, unsigned char *dest, unsigned int len);

static struct tag *params;

void setup_start_tag(void)
{
	params = (struct tag *)0x30000100;			//uboot和内核键约定好将参数放在这个位置，
												//内核启动后到这个位置读取uboot传入的参数

	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);

	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;

	params = tag_next (params);

}
void setup_memory_tags(void)
{
	params->hdr.tag = ATAG_MEM;
	params->hdr.size = tag_size (tag_mem32);

	params->u.mem.start = 0x30000000;
	params->u.mem.size  = 64*1024*1024;

	params = tag_next (params);
}

int strlen(char *str)
{
	int i = 0;
	while(str[i])
	{
		i++;
	}
	return i;
}

void strcopy(char *dest, char *src)
{
	while ((*dest++ = *src++) != '\0');
}

void setup_commandline_tag(char *cmdline)
{
	int len = strlen(cmdline)+1;
	params->hdr.tag = ATAG_CMDLINE;
	params->hdr.size = (sizeof (struct tag_header) + len + 3) >> 2;

	strcopy(params->u.cmdline.cmdline, cmdline);

	params = tag_next (params);

}
void setup_end_tag(void)
{
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;
}

int main(void)
{
	void (*theKernel)(int zero, int arch, unsigned int params);

	/*0、帮内核设置串口：内核启动的开始时，会从串口打印一些信息，但是内核一开始并没有初始化串口*/
	uart0_init();
	/*1.从NAND FLASH中吧内核读入内存*/
	putstr("Copy Kernel from nand\r\n");
	nand_read((0x60000+64), (unsigned char *)0x30008000, 0x200000);
	/*2、设置参数*/
	putstr("set boot params\r\n");
	setup_start_tag();
	setup_memory_tags();
	setup_commandline_tag("noinitrd root=/dev/mtdblock3 init=/linuxrc console=ttySAC0");
	setup_end_tag();

	/*3、跳转执行*/
	putstr("Boot kernel\r\n");
	theKernel = (void (*)(int, int, unsigned int))0x30008000;
	theKernel(0, 362, 0x30000100);	
	/*
	*	mov r0, #0
	*	ldr r1, =362
	*	ldr r2, =0x30000100
	*	mov pc, #0x30008000
	*/	

	/*如果一切正常，不会执行到这里*/
	putstr("Error\n\r");	

	return -1;
}

