 我在学习第12课关于异步通信机制的时候遇到一个问题我在驱动程序中定义了button_async结构体，然后在file_operations结构体重注册了.fasync  =         fifth_drv_fasync，并定义了函数 fifth_drv_fasync()

static int fifth_drv_fasync(int fd, struct file *filp, int on){
        printk("driver:fifth_drv_fasync\n");
        return fasync_helper (fd, filp, on, &button_async);
}

     然后在测试程序中注册信号处理函数signal(SIGIO, my_signal_fun);并进行处理：fcntl(fd, F_SETOWN, (pid=getpid()));
oflags = fcntl(fd, F_GETFL);fcntl(fd, F_GETFL, oflags | FASYNC);

测试的时候发现：
      程序执行完以上代码进入睡眠后，并没有打印printk("driver:fifth_drv_fasync\n");也就是说并没有成功调用fifth_drv_fasync（）函数