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

static struct class *second_drv_class;
static struct class_device	*second_drv_class_dev;

static volatile unsigned long *gpfcon = NULL;
static volatile unsigned long *gpfdat = NULL;

static volatile unsigned long *gpgcon = NULL;
static volatile unsigned long *gpgdat = NULL;

static int second_drv_open(struct inode *inode, struct file *file)
{
	/*配置GPF0,2为输入引脚*/
	*gpfcon  &= ~((0x3<<(0*2)) | (0x3<<(2*2)));
	/*配置GPG3,11为输入引脚*/
	*gpgcon  &= ~((0x3<<(3*2)) | (0x3<<(11*2)));

	return 0;
}

static ssize_t second_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{

	return 0;
}

ssize_t second_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	/*返回四个引脚的电平状态*/
	unsigned char key_vals[4];
	int regval;

	if(size != sizeof(key_vals))
		return -EINVAL;

	/*读GPF0,2*/
	regval = *gpfdat;
	key_vals[0] = (regval & (0x1<<0)) ? '1' : '0';
	key_vals[1] = (regval & (0x1<<2)) ? '1' : '0';

	/*读GPg3,11*/
	regval = *gpgdat;
	key_vals[2] = (regval & (0x1<<3)) ? '1' : '0';
	key_vals[3] = (regval & (0x1<<11)) ? '1' : '0';

	copy_to_user(buf, key_vals, sizeof(key_vals));

	return sizeof(key_vals);
}


static struct file_operations second_drv_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   second_drv_open, 
    .read 	=	second_drv_read,
	.write	=	second_drv_write,	   
};

int major = 0;		//主设备号
static int second_drv_init(void)
{
	major = register_chrdev(0, "second_drv", &second_drv_fops);//注册驱动程序

	second_drv_class = class_create(THIS_MODULE, "second_drv");

	second_drv_class_dev = class_device_create(second_drv_class, NULL, MKDEV(major, 0), NULL, "second_drv");//创建设备节点/dev/firstdrv

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);		//内存地址映射
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);		//内存地址映射
	gpgdat = gpgcon + 1;
	return 0;
}

static void second_drv_exit(void)
{
	unregister_chrdev(major, "second_drv");

	class_device_unregister(second_drv_class_dev);

	class_destroy(second_drv_class);

	iounmap(gpfcon);			//解除内存映射
	iounmap(gpgcon);			//解除内存映射
	return;
}

module_init(second_drv_init);//修饰注册驱动程序
module_exit(second_drv_exit);//修饰卸载驱动程序

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("xuxin_in_15#351");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("S3C2410/S3C2440 second Driver");
MODULE_LICENSE("GPL");		//遵循GPL协议
