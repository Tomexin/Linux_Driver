#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>

static struct fb_ops s3c_lcdfb_ops = {
	//.fb_setcolreg	= mc68x328fb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static struct fb_info *s3c_lcd;
static int lcd_init(void)
{
	/*1、内核分配一个fb_info 结构体*/
	s3c_lcd = framebuffer_alloc(0, NULL); 

	/*2、设置*/
	/*2.1、设置固定的参数*/
	strcpy(s3c_lcd -> fix.id, "mylcd");
 	s3c_lcd->fix.smem_len = 240*320*16/8;
 	s3c_lcd->fix.type	  = FB_TYPE_PACKED_PIXELS;
 	s3c_lcd->fix.visual   = FB_VISUAL_TRUECOLOR;
 	s3c_lcd->fix.line_length = 240*2;
	/*2.2、设置可变的参数*/
 	s3c_lcd->var.xres			= 240;
 	s3c_lcd->var.yres			= 320;
 	s3c_lcd->var.xres_virtual	= 240;
 	s3c_lcd->var.yres_virtual	= 320;
 	s3c_lcd->var.bits_per_pixel	= 16;

 	/*RGB: 5 6 5*/
 	s3c_lcd->var.red.offset		= 11;
 	s3c_lcd->var.red.length 	= 5;

 	s3c_lcd->var.green.offset	= 5;
 	s3c_lcd->var.green.length 	= 6;

 	s3c_lcd->var.blue.offset	= 0;
 	s3c_lcd->var.blue.length 	= 5;

 	s3c_lcd->var.activate		= FB_ACTIVATE_NOW;

	/*2.3、设置操作函数*/
	s3c_lcd->fb_ops				= &s3c_lcdfb_ops;

	/*2.4、其他设置*/
	//s3c_lcd->pseudo_palette = ...
	//s3c_lcd->screen_base 		=;	/* 显存的虚拟地址*/
	s3c_lcd->screen_size		= 240*320*16/8;
	
	/*3、硬件相关设置*/
	/*3.1、配置GPIO用于LCD*/


	/*3.2、根据LCD手册设置LCD控制器，比如VCLCK的频率*/

	/*3.3、分配显存（framebuffer）,并把地址告诉LCD控制器*/
	//s3c_lcd->fix.smem_start = xxx;	/*显存的物理地址*/

	/*4、注册*/
	register_framebuffer(s3c_lcd);
	
	return 0;
}

statuc void lcd_exit(void)
{
}


module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");
