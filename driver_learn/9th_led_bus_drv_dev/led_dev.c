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


/*分配/设置/注册一个platform_device结构体*/
static struct resource led_resource[] = {
	[0] = {
		.start = 0x56000050,
		.end   = 0x56000050+8-1,
		.flag  = IORESOURCE_MEM,
	},
	[1] = {
		.start = 4,
		.end   = 4,
		.flag  = IORESOURCE_IRQ,
	}
}

static struct platform_device led_dev = {
	.name			= "myled",
	.id				= -1,
	.num_resources	= ARRAY_SIZE(led_resource),
	.resource 		= led_resource,
	
	
};

static int led_dev_init(void)
{
	platform_device_register(&led_dev);
}

static void led_dev_exit(void)
{
	platform_device_unregister(&led_dev);
}

module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomxin in 15#351");


