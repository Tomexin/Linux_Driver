#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *firstdrv_class;
static struct class_device	*firstdrv_class_dev;

volatile unsigned long *gpfcon = NULL;
volatile unsigned long *gpfdat = NULL;

static int first_drv_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open\n");
	/*配置GPF4,5,6为输出状态*/
	*gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2)) | (0x3<<(6*2)));	//寄存器相应位清零
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));	//设置相应位

	return 0;
}


static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	copy_from_user(&val, buf, count);			//内核空间从用户空间获取数据

	if( val == 1)
	{
		/*点灯*/
		*gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
	}
	else
	{
		/*关灯*/
		*gpfdat |= (1<<4) | (1<<5) | (1<<6);
	}

	//printk("first_drv_write\n");

	return 0;
}

static struct file_operations first_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   first_drv_open,        
	.write	=	first_drv_write,	   
};

int major = 0;

static int first_drv_init(void)
{
	major = register_chrdev(0, "first_drv", &first_drv_fops);//注册驱动程序

	firstdrv_class = class_create((struct module *)THIS_MODULE, (const char *)"first_drv");

	firstdrv_class_dev = class_device_create(firstdrv_class, NULL, MKDEV(major, 0), NULL, "first_drv");//创建设备节点/dev/firstdrv

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);		//内存映射
	gpfdat = gpfcon + 1;

	return 0;
}

static void first_drv_exit(void)
{
	unregister_chrdev(major, "first_drv");//卸载驱动

	class_device_unregister(firstdrv_class_dev);

	class_destroy(firstdrv_class);

	iounmap(gpfcon);			//解除内存映射
}

module_init(first_drv_init);//修饰注册驱动程序
module_exit(first_drv_exit);//修饰卸载驱动程序

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("xuxin_in_15#351");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("S3C2410/S3C2440 first Driver");
MODULE_LICENSE("GPL");
