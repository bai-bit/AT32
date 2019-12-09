# 串口波特率的计算问题

```c
  RCC_GetClocksFreq(&RCC_ClocksStatus);

  if (usartxbase == USART1_BASE)
  {
    apbclock = RCC_ClocksStatus.APB2CLK_Freq;
  }
  else
  {
    apbclock = RCC_ClocksStatus.APB1CLK_Freq;
  } 

    integerdivider = ((25 * apbclock) / (4 * baud));
  

  tmpreg = (integerdivider / 100) << 4;


  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

 
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
//串口的波特率计算过程
//就是这后面的四行shang代码
//思路就是：integerdivider = ((25 * apbclock) / (4 * baud));
//将apbclock放大一百倍，相应的商也就扩大了100倍。
//但是这里存在一个危险，如果apbclock的值超过172M，25 * apbclock的值就会溢出
//所以这个算法适用于APB2总线时钟不超过172M的硬件系统。
//但是它为什么要放大一百倍呢，不放大不行吗？
//在计算机系统中，两个整数相除，得到的结果也是整数，不会保留小数部分。
//但是我们的BRR寄存器中需要小数部分
//但是我们的计算结果无法保留小数部分，我们只能是把除数放大，那么放大多少呢，看我们的寄存器存放小数部分的寄存器，用四个bit来保存，他是二进制，换成十进制就是16，占了两位，为了保证准确，我们直接将除数放大一百倍。将商的末尾两位小数向左移动两位。
//得出得结果自然也是一个整数，是包括两位小数的整数。
//接下来的一句：tmpreg = (integerdivider / 100) << 4;
//很明显：商除以100，得到的是整数部分
//然后向左移动四位，这是因为在BRR寄存器中，整数部分是存放在[15:4]这一块区域中，最后我们会用tmpreg的值直接赋值给BRR寄存器。
//接下来就是小数部分了：fractionaldivider = integerdivider - (100 * (tmpreg >> 4));
//这一句就是算出小数部分，我们只取两位小数的精度，完全能满足精度要求，
//最后一句：tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
//这一句是什么意思呢，小数部分左移四位，然后加50，接着除以100，
//再接着屏蔽位，只保留低四位
//它为什么要乘16？也就是向左移四位。
//因为这里的小数是一个被放大100倍的整数。而我们只需要的它的高四位。
//那怎么操作，先把我们需要的高四位移出去，然后缩小100倍，得到得整数就是我们提前移出来得高四位。
//但是在除以100之前，我把左移了得小数部分加了50，然后才除以100，这是什么意思？
//其实这是一个四舍五入的操作。最后的与运算，就很简单了。
//屏蔽不需要的位，只留下低四位，与temreg相或。置位。
//最后这个tmpreg的值，就是我们要放到BRR寄存器中的值。

```

