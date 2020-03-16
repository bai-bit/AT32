# Flash

​	AT32F403ZGT6芯片内部有一个存储介质，闪存存储器模块组织，它分为三个部分：主存储器，信息块，接口寄存器。

​	主存储器:有两个块，一个512KB,有256个扇区，每个扇区2k字节，起始地址为0x08000000.一个3584KB,有1793个扇区，每个扇区2k字节，起始地址为0x08080000。

​	信息块：分启动代码区和用户选择字节区，启动代码区是用来存储AT自带的启动代码。用于串口下载代码。用户选择字节区一般用于配置写保护，读保护等功能。

​	接口寄存器：用于控制闪存的读写擦除等操作，是整个闪存模块的控制区域。

​	下面我就说明一下，对闪存的操作：

读过程

​	1、读取flash_CTRL的lock位，为__1__时，向Flash_FCKEY寄存器写入两个键值

​                                            为__0__时，直接下一步操作。

​	2、直接对地址进行解引用，赋值，解引用结果时强转为半字

​	3、返回结果

```c
uint16_t FLASH_ReadHalfWord(uint32_t faddr)
{
	return *(uint16_t *)faddr; 
}
//直接对地址进行寻址
//当然了，在此之前先进行使能相应的时钟
void flash_init(void)
{
	//开启时钟
	RCC->AHBEN |= RCC_AHBEN_FLASHEN;
	flash_lock();
}
//判断lock位，写入键值
void flash_lock(void)
{
	//判断flash_CTRL中lock位
	//写入两个键值
	if(FLASH->CTRL & FLASH_CTRL_LCK)
	{
		FLASH->FCKEY = FLASH_KEY1;
		FLASH->FCKEY = FLASH_KEY2;
	}
	if(FLASH->CTRL & FLASH_CTRL_LCK)
	{
		//return status
	}
}
```



擦除过程

​	1、读取flash_CTRL的lock位，为__1__时，向flash_FCKEY寄存器写入两个键值

​														为__0__时，直接下一步操作	

​	2、置Flash_CTRL的第1位为1，对flash进行页擦除操作

​	3、在flash_ADDR中添加要擦除页的地址

​	4、置flash_CTRL中第6位（RSTR）为1，触发一次擦除操作。

​	5、等待flash_STS的BSY位为0，可以加入一个超时控制。

​	6、置Flash_CTRL的第1位为0，取消页擦除操作

​	7、返回结果。

```c
	  FLASH->CTRL |= 2;
      FLASH->ADDR = Page_Address;
      FLASH->CTRL |= 0x40;

      status = FLASH_WaitForBank1Process(TIMEOUT);

      FLASH->CTRL &= ~2;
```





写入过程

​	1、读取flash_CTRL的lock位，为__1__时，向flash_FCKEY寄存器写入两个键值

​														为__0__时，直接进行下一步操作

​	2、读出在该扇区中所有的数据。

​	3、判断写入地址之后的空间是否已擦除。

​	4、如果已擦除，直接进行写操作，如果没有擦除，先进行擦除操作

​	5、置flash_CTRL的第0位为1，对flash进行操作

​	6、在指定的地址处赋值半字数据。

​	7、等待flash_STS的BSY位为0，可以添加一个超时控制。

​	8、返回结果

```c
 	  FLASH->CTRL |= CTRL_PRGM_Set;

      *(uint16_t *)address = Data;
     
      status = FLASH_WaitForBank1Process(PRGM_TIMEOUT);

      FLASH->CTRL &= CTRL_PRGM_Rst;
```

​	具体的代码，不再进行粘贴，不过有一个需要特别注意：就是再擦除操作时，留给硬件的超时时间一定要充分，不然会导致擦除失败。这是一个大坑，我当时因为超时时间不够长，而导致擦除失败，写入不成功。浪费了好多时间。只要把它的逻辑搞明白了，函数的功能其实很好实现。

