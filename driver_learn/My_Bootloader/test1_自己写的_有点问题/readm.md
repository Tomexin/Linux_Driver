最简单的BootLoader功能的编写步骤:
1、初始化硬件：关看门狗、设置时钟、设置SDRAM、初始化nand flash
2、如果BootLoader比较大，要把他重定位到SDRAM
3、把内核从NAND FLASh读到SDRAM中
4、设置要传给内核的参数
5、跳转执行内核
