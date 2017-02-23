/*
*添加原子操作，保证同一时刻智能化有一个应用调用驱动程序
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/kdev_t.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/atomic.h>


static struct class *sixth_drv_class;
static struct class_device	*sixth_drv_class_dev;

static volatile unsigned long *gpfcon = NULL;
static volatile unsigned long *gpfdat = NULL;

static volatile unsigned long *gpgcon = NULL;
static volatile unsigned long *gpgdat = NULL;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);			/*定义一个等待队列头*/

/* 中断事件标志, 中断服务程序将它置1，sixth_drv_read将它清0 */
static volatile int ev_press = 0;

//static struct fasync_struct *button_async;

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

//static int canopen = 1;
//static atomic_t canopen = ATOMIC_INIT(1);			//定义变量cabopen为原子变量，并初始化为1
static DECLARE_MUTEX(button_lock);					//定义信号量互斥锁:定义了信号量，并初始化为1
 
/*
 *中断处理函数：确定按键值
 *
*/
static irqreturn_t buttons_irp(int irq, void *dev_id)
{
	unsigned int pinval;
	struct pin_desc *pindec = (struct pin_desc *)dev_id;

	printk("buttons_irp() is called\n");

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

	//kill_fasync(&button_async,SIGIO,POLL_IN); //检测到中断，给应用程序发送SIGIO信号

	return IRQ_RETVAL(IRQ_HANDLED);
}

static int sixth_drv_open(struct inode *inode, struct file *file)
{
	int request_irq_return_value[4];
	printk("sixth_drv_open() is called\n");

	if(file->f_flags & O_NONBLOCK)
	{
		//非阻塞
		if(down_trylock(&button_lock))
			return EBUSY;
	}
	else
	{
		/*获取信号量*/
		down(&button_lock);
	}

#if 0	
	if(!atomic_dec_and_test(&canopen))		//原子操作：自减1后检查是否为0，为0返回TRUE，不为0返回false	
	{
		atomic_inc(&canopen);			//原子变量canopen自加1
		return -EBUSY;
	}
#endif

	/*配置GPF0,2为输入引脚*/
	/*配置GPG3,11为输入引脚*/
	request_irq_return_value[0] = \
		request_irq(IRQ_EINT0,  buttons_irp, IRQT_BOTHEDGE, "S2", &pins_desc[0]);
	request_irq_return_value[1] = \
		request_irq(IRQ_EINT2,  buttons_irp, IRQT_BOTHEDGE, "S3", &pins_desc[1]);
	request_irq_return_value[2] = \
		request_irq(IRQ_EINT11, buttons_irp, IRQT_BOTHEDGE, "S4", &pins_desc[2]);
	request_irq_return_value[3] = \
		request_irq(IRQ_EINT19, buttons_irp, IRQT_BOTHEDGE, "S5", &pins_desc[3]);

	if(request_irq_return_value[0]!=0 || request_irq_return_value[1]!=0 ||\
		request_irq_return_value[2]!=0 || request_irq_return_value[3]!=0)
	{
		printk("request_irq() failed\n");
		return -1;
	}

	return 0;
}

static ssize_t sixth_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	printk("sixth_drv_write() is called\n");
	return 0;
}

ssize_t sixth_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if(size != 1)
		return -EINVAL;

	if(file->f_flags & O_NONBLOCK)
	{
		//非阻塞,直接返回
		if(!ev_press)
			return -EAGAIN;
	}
	else
	{
		//阻塞，进入休眠状态
		/*如果没有按键动作发生，休眠*/
		wait_event_interruptible(button_waitq, ev_press);
	}
	/*如果有按键动作发生，返回键值*/
	copy_to_user(buf, &key_val, 1);

	/*可以运行到这里，说明程序已经被唤醒，为了下次还能进入休眠，把条件请0*/
	ev_press = 0;

	return 1;
}

static int sixth_drv_release(struct inode *inode, struct file *file)
{

	//atomic_inc(&canopen);			//原子变量canopen自加1, 释放改原子变量
	up(&button_lock);				//释放信号量

	//释放中断
	free_irq(IRQ_EINT0,  &pins_desc[0]);
	free_irq(IRQ_EINT2,  &pins_desc[1]);
	free_irq(IRQ_EINT11, &pins_desc[2]);
	free_irq(IRQ_EINT19, &pins_desc[3]);

	return 0;

}

static unsigned int sixth_drv_poll(struct file *file, struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait);		//将进程挂到button_waitq等待队列中，不会立即休眠

	if(ev_press)
	{
		mask |= POLLIN | POLLRDNORM;
	}

	return mask;
}

// static int sixth_drv_fasync(int fd, struct file *filp, int on)
// {
// 	printk("driver:sixth_drv_fasync\n");
// 	return fasync_helper (fd, filp, on, &button_async);
// }


static struct file_operations sixth_drv_fops = {
    .owner   =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open    =   sixth_drv_open, 
    .read 	 =	 sixth_drv_read,
	.write	 =	 sixth_drv_write,
	.release = 	 sixth_drv_release, 
	.poll 	 =	 sixth_drv_poll,
	//.fasync  =	 sixth_drv_fasync,
};

int major = 0;		//主设备号
static int sixth_drv_init(void)
{
	//注册驱动程序
	major = register_chrdev(major, "sixth_drv", &sixth_drv_fops);

	//自动创建设备节点
	sixth_drv_class = class_create(THIS_MODULE, "sixth_drv");
	sixth_drv_class_dev = class_device_create(sixth_drv_class, NULL, MKDEV(major, 0), NULL, "sixth_drv");

	//内存地址映射
	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);		
	gpfdat = gpfcon + 1;
	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;

	return 0;
}

static void sixth_drv_exit(void)
{
	unregister_chrdev(major, "sixth_drv");

	class_device_unregister(sixth_drv_class_dev);

	class_destroy(sixth_drv_class);

	iounmap(gpfcon);			//解除内存映射
	iounmap(gpgcon);			//解除内存映射
	return;
}

module_init(sixth_drv_init);//修饰注册驱动程序
module_exit(sixth_drv_exit);//修饰卸载驱动程序

/* 描述驱动程序的一些信息，不是必须的 */
MODULE_AUTHOR("xuxin_in_15#351");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("S3C2410/S3C2440 sixth Driver");
MODULE_LICENSE("GPL");		//遵循GPL协议
