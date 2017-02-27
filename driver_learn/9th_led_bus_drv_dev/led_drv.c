/*分配/设置/注册一个platform_driver结构体*/

#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/gpio_keys.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/device.h>

#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/atomic.h>

static int major = 0;
static struct class *led_class;
static struct class_device	*led_class_dev;

static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;

static int led_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open\n");
	/*配置为输出状态*/
	*gpio_con &= ~(0x3<<(pin*2));	//寄存器相应位清零
	*gpio_con |= (0x1<<(pin*2));	//设置相应位

	return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	copy_from_user(&val, buf, count);			//内核空间从用户空间获取数据

	if( val == 1)
	{
		/*点灯*/
		*gpio_dat &= ~(1<<pin);
	}
	else
	{
		/*关灯*/
		*gpio_dat |= (1<<pin);
	}

	//printk("first_drv_write\n");

	return 0;
}

static struct file_operations led_fops = {
	 /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
	.owner  =   THIS_MODULE,   
    .open   =   led_open,        
	.write	=	led_write,
};

static int  led_probe(struct platform_device *pdev)
{
	struct resource 			*res;
	/*根据platform_device的资源进行ioremap*/
	res = platform_get_resource(pdev,IORESOURCE_MEM, 0);
	gpio_con = ioremap(res->start, res->end - res->start + 1);
	gpio_dat = gpio_con + 1;

	res = platform_get_resource(pdev,IORESOURCE_IRQ, 0);
	pin = res->start;

	/*注册字符设备驱动*/

	printk("led_remove() is called\n");

	major = register_chrdev(0, "myled", &led_fops);
	//自动创建设备节点
	led_class = class_create((struct module *)THIS_MODULE, \
		(const char *)"myled");
	led_class_dev = class_device_create(led_class, NULL,\
		MKDEV(major, 0), NULL, "myled");/*/dev/myled*/

	printk("led_probe() is called\n");
	return 0;
}


static int __devexit led_remove(struct platform_device *pdev)
{
	/*根据platform_device的资源进行iounremap*/
	iounmap(gpio_con);

	/* 卸载字符设备驱动*/
	class_device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "myled");
	
	return 0;
}

struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove 	= led_remove,
	.driver 	={
		.name 		= "myled",		//平台驱动层.driver下的name要和平台设备层保持一致
	},
};


static int led_drv_init(void)
{
	platform_driver_register(&led_drv);
	return 0;
}

static void led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomxin in 15#351");

