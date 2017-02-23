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

/*分配/设置/注册一个platform_driver结构体*/

static int __devexit led_remove(struct platform_device *pdev)
{
	/*根据platform_device的资源进行iounremap*/

	/*卸载字符设备驱动*/

	printk("led_remove() is called\n");
	return 0;
	
}


static int  led_probe(struct platform_device *pdev)
{
	/*根据platform_device的资源进行ioremap*/


	/*注册字符设备驱动*/

	printk("led_probe() is called\n");
	return 0;
}


struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove 	= led_remove,
	.driver 	={
		.name 		= "myled",
	}
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


