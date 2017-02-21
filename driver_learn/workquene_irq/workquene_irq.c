#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

/*定义默认中断号*/
static int irq = 1;

static char *devname = "default_iq1";
/*
*使用工作队列，一般习惯吧struct work_struct嵌入到自己定义的结构中
*/
typedef struct
{
	struct work_struct my_work;		//核心数据结构
	int x;							//自定义变量 
}my_work_t;

my_work_t work;						//定义my_work_t变量，里面包含struct work_struct 结构

static int max_count = 10;				//延时使用10次后，不在调度工作队列

module_param(irq,int,0644);
module_param(devname,charp,0644);
module_param(max_count,int,0644);

void mysleep(unsigned int msece);

struct myirq
{
	int devid;
};

static struct myirq mydev = {123};

/*队列处理函数*/
static void my_wq_function(struct work_struct *work)
{
	my_work_t *my_work = (my_work_t *)work;
	printk("my_work.x %d\n", my_work->x);
}

static irqreturn_t myirq_handler(int irq,void *dev)
{
	struct myirq mydev;
	static int count = 0;

	mydev = *(struct myirq *)dev;

	if(count<max_count)
	{
		printk("key:%d, devid:%d\n", count+1, mydev.devid);
		schedule_work(&work.my_work);		//调用底半部
		count++;
	}
	return IRQ_HANDLED;
}

static int myirq_init(void)
{
	work.x = 12345;
	//初始化工作队列
	INIT_WORK(&work.my_work, (void *)my_wq_function);

	printk("MODULE is working...\n");

	if(request_irq(irq, myirq_handler, IRQF_TRIGGER_FALLING | IRQF_SHARED, \
		devname, mydev.devid) != 0)
	{
		printk("%srequest IRQ:%d failed..\n", devname, irq);
		return -1;
	}

	printk("%srequest IRQ:%d success..\n", devname, irq);

	return 0;	
}

static void myirq_exit(void)
{
	printk("MODULE is leaving...\n");
	free_irq(irq, &(mydev.devid));
	printk("%srequest IRQ:%d success..\n", devname, irq);
}

module_init(myirq_init);
module_exit(myirq_exit);

MODULE_LICENSE("GPL");
