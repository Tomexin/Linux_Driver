#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/kdev_t.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *third_drv_class;
static struct class_device	*third_drv_class_dev;

static volatile unsigned long *gpfcon = NULL;
static volatile unsigned long *gpfdat = NULL;

static volatile unsigned long *gpgcon = NULL;
static volatile unsigned long *gpgdat = NULL;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);			/*定义一个等待队列头*/

/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0;

struct pin_desc{
	unsigned int pin;
	unsigned int key_val;
};

/*键值：按下时， 0x01,0x02,0x03,0x04*/
/*键值：松开时， 0x81,0x82,0x83,0x84*/
static unsigned char key_val;

struct pin_desc	pins_desc[4]={
	{S3C2410_GPF0, 0x01},
	{S3C2410_GPF2, 0x02},
	{S3C2410_GPG3, 0x03},
	{S3C2410_GPG11, 0x04},
};

/*
 *中断处理函数：确定按键值
 *
*/
static irqreturn_t buttons_irp(int irq, void *dev_id)
{
	struct pin_desc *pindec = (struct pin_desc *)dev_id;
	unsigned int pinval;

	/*读取pin值*/
	pinval = s3c2410_gpio_getpin(pindec->pin);

	if(pinval){
		/*松开*/
		key_val = 0x80 | pindec->key_val;
	}else{
		/*按下*/
		key_val = pindec->key_val;
	}

	ev_press = 1;							//表示中断发生了
	wake_up_interruptible(&button_waitq);	//唤醒休眠的进程

	return IRQ_RETVAL(IRQ_HANDLED);
}

static int third_drv_open(struct inode *inode, struct file *file)
{
	/*配置GPF0,2为输入引脚*/
	/*配置GPG3,11为输入引脚*/
	request_irq(IRQ_EINT0,  buttons_irp, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	request_irq(IRQ_EINT2,  buttons_irp, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	request_irq(IRQ_EINT11, buttons_irp, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	request_irq(IRQ_EINT19, buttons_irp, IRQT_BOTHEDGE, "S5", &pins_desc[3]);

	return 0;
}

// static ssize_t third_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
// {

// 	return 0;
// }

ssize_t third_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if(size != 1)
		return -EINVAL;
	/*如果没有按键动作发生，休眠*/
	wait_event_interruptible(button_waitq, ev_press);
	/*如果有按键动作发生，返回键值*/
	copy_to_user(buf, &key_val, 1);

	/*可以运行到这里，说明程序已经被唤醒，为了下次还能进入休眠，把条件请0*/
	ev_press = 0;

	return 1;
}

static third_drv_release(struct inode *inode, struct file *file)
{
	//释放中断
	free_irq(IRQ_EINT0,  &pins_desc[0]);
	free_irq(IRQ_EINT2,  &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);

	return 0;
}


static struct file_operations third_drv_fops = {
    .owner   =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =   third_drv_open, 
    .read 	 =	 third_drv_read,
	// .write	 =	 third_drv_write,
	.release = 	 third_drv_release, 
};

int major = 0;		//主设备号
static int third_drv_init(void)
{
	//注册驱动程序
	major = register_chrdev(major, "third_drv", &third_drv_fops);

	//自动创建设备节点
	third_drv_class = class_create(THIS_MODULE, "third_drv");
	third_drv_class_dev = class_device_create(third_drv_class, NULL, MKDEV(major, 0), NULL, "third_drv");

	//内存地址映射
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);		
	gpfdat = gpfcon + 1;
	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void third_drv_exit(void)
{
	unregister_chrdev(major, "third_drv");

	class_device_unregister(third_drv_class_dev);

	class_destroy(third_drv_class);

	iounmap(gpfcon);			//解除内存映射
	iounmap(gpgcon);			//解除内存映射
	return;
}

module_init(third_drv_init);//修饰注册驱动程序
module_exit(third_drv_exit);//修饰卸载驱动程序

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("xuxin_in_15#351");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("S3C2410/S3C2440 third Driver");
MODULE_LICENSE("GPL");		//遵循GPL协议 