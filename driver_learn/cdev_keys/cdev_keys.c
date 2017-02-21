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

#define KEY_SIZE 4						//按键数量
#define DEV_NAME "cdev_keys"			//设备名称

static struct class *cdev_keys_class;				//自动创建设备类
static struct class_device	*cdev_keys_class_dev;	//自动创建设备节点

static volatile unsigned long *gpfcon = NULL;
static volatile unsigned long *gpfdat = NULL;

static volatile unsigned long *gpgcon = NULL;
static volatile unsigned long *gpgdat = NULL;

static int cdev_keys_open(struct inode *inode, struct file *file)
{
	/*配置GPF0,2为输入引脚*/
	*gpfcon  &= ~((0x3<<(0*2)) | (0x3<<(2*2)));
	/*配置GPG3,11为输入引脚*/
	*gpgcon  &= ~((0x3<<(3*2)) | (0x3<<(11*2)));

	return 0;
}

static ssize_t cdev_keys_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{

	return 0;
}

ssize_t cdev_keys_read(struct file *file, char __user *buf, size_t size, loff_t *f_pos)
{
	/*返回四个引脚的电平状态*/
	unsigned char key_vals[4] = {0};
	int regval;

	if(size != sizeof(key_vals))
		return -EINVAL;

	/*读GPF0,2*/
	regval = *gpfdat;
	key_vals[0] = (regval & (0x1<<0)) ? 1 : 0;
	key_vals[1] = (regval & (0x1<<2)) ? 1 : 0;

	/*读GPg3,11*/
	regval = *gpgdat;
	key_vals[2] = (regval & (0x1<<3)) ? 1 : 0;
	key_vals[3] = (regval & (0x1<<11)) ? 1 : 0;

	if(*f_pos >= KEY_SIZE)
		goto out;

	/*修正可以读取的按键值的数量*/
	if(*f_pos + size > KEY_SIZE)
		size = KEY_SIZE-*f_pos;

	if(copy_to_user(buf, &key_vals[0 + (*f_pos)], sizeof(key_vals)))
	{
		goto out;
	}

	*f_pos += size;		//更新文件读写指针

	return size;

	out:
		return 0;
}

static loff_t cedv_keys_llseek(struct file * filp, loff_t off, int whence)
{
	loff_t newpos = 0;
	int offset = off;
	printk("llseek() is called\n");
	switch(whence)
	{
	case SEEK_SET:		/*以文件头为偏移起始值*/
		newpos = offset;
		break;

	case SEEK_CUR:		/*以当前位置为偏移起始值*/
		newpos = filp->f_pos + offset;
		break;

	case SEEK_END:		/*以文件尾为偏移起始值*/
		newpos = KEY_SIZE + offset;

	default:
		return -EINVAL;

	}
	if(newpos < 0)
		return -EINVAL;

	filp->f_pos = newpos;	/*更新文件当前读写位置*/

	return newpos;
}


static struct file_operations cdev_keys_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   cdev_keys_open, 
    .read 	=	cdev_keys_read,
	.write	=	cdev_keys_write,
	.llseek = 	cedv_keys_llseek,   
};

int major = 0;		//主设备号
static int cdev_keys_init(void)
{
	major = register_chrdev(0, DEV_NAME, &cdev_keys_fops);//注册驱动程序

	cdev_keys_class = class_create(THIS_MODULE, DEV_NAME);

	cdev_keys_class_dev = class_device_create(cdev_keys_class, NULL, MKDEV(major, 0), NULL, DEV_NAME);//创建设备节点/dev/firstdrv

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);		//内存地址映射
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);		//内存地址映射
	gpgdat = gpgcon + 1;
	return 0;
}

static void cdev_keys_exit(void)
{
	unregister_chrdev(major, DEV_NAME);

	class_device_unregister(cdev_keys_class_dev);

	class_destroy(cdev_keys_class);

	iounmap(gpfcon);			//解除内存映射
	iounmap(gpgcon);			//解除内存映射
	return;
}

module_init(cdev_keys_init);//修饰注册驱动程序
module_exit(cdev_keys_exit);//修饰卸载驱动程序

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("xuxin_in_15#351");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("S3C2410/S3C2440 second Driver");
MODULE_LICENSE("GPL");		//遵循GPL协议
