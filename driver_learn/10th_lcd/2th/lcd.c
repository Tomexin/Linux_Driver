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


struct lcd_regs{
	unsigned long lcdcon1;
	unsigned long lcdcon2;
	unsigned long lcdcon3;
	unsigned long lcdcon4;
	unsigned long lcdcon5;
	unsigned long lcdsaddr1;
	unsigned long lcdsaddr2;
	unsigned long lcdsaddr3;
	unsigned long redlut;
	unsigned long greenlut;
	unsigned long bluelut;
	unsigned long reserved[9];
	unsigned long dithmode;
	unsigned long tpal;
	unsigned long lcdintpnd;
	unsigned long lcdsrcpnd;
	unsigned long lcdintmsk;
	unsigned long lpcsel;
};


static struct fb_ops s3c_lcdfb_ops = {
	//.fb_setcolreg	= mc68x328fb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static struct fb_info *s3c_lcd;

static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;

static volatile struct lcd_regs *lcd_regs;

static int lcd_init(void)
{
	/*1���ں˷���һ��fb_info �ṹ��*/
	s3c_lcd = framebuffer_alloc(0, NULL); 

	/*2������*/
	/*2.1�����ù̶��Ĳ���*/
	strcpy(s3c_lcd -> fix.id, "mylcd");
 	s3c_lcd->fix.smem_len = 240*320*16/8;
 	s3c_lcd->fix.type	  = FB_TYPE_PACKED_PIXELS;
 	s3c_lcd->fix.visual   = FB_VISUAL_TRUECOLOR;
 	s3c_lcd->fix.line_length = 240*2;
	/*2.2�����ÿɱ�Ĳ���*/
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

	/*2.3�����ò�������*/
	s3c_lcd->fb_ops				= &s3c_lcdfb_ops;

	/*2.4����������*/
	//s3c_lcd->pseudo_palette = ...
	//s3c_lcd->screen_base 		=;	/* �Դ�������ַ*/
	s3c_lcd->screen_size		= 240*320*16/8;
	
	/*3��Ӳ���������*/
	/*3.1������GPIO����LCD*/
	gpbcon = ioremap(0x56000010, 8);
	gpbdat = gpbcon + 1;
	gpccon = ioremap(0x56000020, 4);
	gpdcon = ioremap(0x56000030, 4);
	gpgcon = ioremap(0x56000060, 4);

	*gpccon = 0xaaaaaaaa;   // GPIO�ܽ�����VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND 
	*gpdcon = 0xaaaaaaaa;   // GPIO�ܽ�����VD[23:8]
	*gpbcon	&=  ~(0x03);	// GPB0��Ϊ�������
	*gpbcon	|=  0x01;		//	
	*gpbdat &=  ~0x01;		//Ĭ�Ϲرձ���

	*gpgcon |= (3<<8);		//GPG4����LCD_PWREN

	/*3.2������LCD�ֲ�����LCD������������VCLCK��Ƶ��*/
	lcd_regs = ioremap(0X4D000000, sizeof(struct lcd_regs));

	/*bit [17:8]  VCLK = HCLK / [(CLKVAL+1) x 2]  LCD�ֲ� P14
	*			  (10MHz)100ns = 100MHz / [(CLKVAL+1) x 2]
	*				CLKVAL = 4
	*bit [6:5]	  0b11, TFT LCD
	*bit [4:1]	  0b110, 16 bpp for TFT
	*bit [0]	0  Disable the video output and the LCD control signal.	
 	*/
	lcd_regs->lcdcon1 = (4<<8) | (3<<5) | (0x0C<<1);

	/*��ֱ�����ʱ�����
	*bit[31:24] : VBPD, VSYNC ֮���ٹ��೤ʱ����ܷ�����һ������
	*			LCD�ֲ�   T0-T2-T1 = 4
	*					VBPD = 3
	*bit[23:14] : �����У� 320��, ����LINEVAL= 320-1 =319
	*bit[13:6]	: VFPD, �������һ������֮���ٹ��೤ʱ�䷢��VSYNC�źţ�
	*			LCD�ֲ�T2-T5=322-320=2,����VFPD=2-1=1
	*bit[5:0] 	:VSPW,VSYNC�źŵ�������ȣ�LCD�ֲ�T1=1,����VSPW=1-1=0
	*/
	lcd_regs->lcdcon2 = (3<<24) | (319<<14) | (1<<6) | (0<<0);
	/*ˮƽ�����ʱ�����
	*bit[25:19] : HBPD, HSYNC ֮���ٹ��೤ʱ����ܷ�����һ�����ص�����
	*			LCD�ֲ�   T6-T7-T8 = 17
	*					  HBPD = 16 
	*bit[18:8] : �����У� 240, ����HOZVAL= 240-1 =239
	*bit[7:0]	: HFPD, �������һ�������һ������֮���ٹ��೤ʱ�䷢��HSYNC�źţ�
	*			LCD�ֲ�T8-T11=251-240=11,����HFPD=11-1=10
	*/
	lcd_regs->lcdcon3 = (16<<19) | (239<<8) | (10<<0);

	/*ˮƽ�����ͬ���ź�
	*bit[7:0] 	:HSPW,HSYNC�źŵ�������ȣ�LCD�ֲ�T7=5,����HSPW=5-1=4
	*/
	lcd_regs->lcdcon4 = (0x04);

	/*�źŵļ���
	*bit[11]: 1 565 format
	*bit[10]: 0
	*bit[9]: 1 
	*bit[8]: 1
	*bit[6]: 0
	*bit[3]: 0  PWREN���0
	*bit[1]: 0  BSWP
	*bit[0]: 1	HWSWP  2440�ֲ� P413
	*/
	lcd_regs->lcdcon5 = (1<<11) | (0<<10) | (1<<9) | (1<<8) | (1<<0);
	
	/*3.3�������Դ棨framebuffer��,���ѵ�ַ����LCD������*/
	s3c_lcd->screen_base = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len, &s3c_lcd->fix.smem_start, GFP_KERNEL);
	lcd_regs->lcdsaddr1 = (s3c_lcd->fix.smem_start>>1) & ~(3<<30);
	lcd_regs->lcdsaddr2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len) >> 1) & 0x1fffff;
	lcd_regs->lcdsaddr3 = (240*16/16);	//һ�еĳ��ȣ���λ�ǣ�2�ֽ�
	//����LCD
	lcd_regs-> lcdcon1 |= (1<<0);	/*ʹ��lcd����*/
	*gpbdat |=  0x01;				//�򿪱���
	

	//s3c_lcd->fix.smem_start = xxx;	/*�Դ��������ַ*/

	/*4��ע��*/
	register_framebuffer(s3c_lcd);
	
	return 0;
}

statuc void lcd_exit(void)
{
}


module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");
