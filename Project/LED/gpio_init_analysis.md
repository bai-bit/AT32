# GPIO_Init()函数

先实现最基本的初始化功能,不考虑其他情况

```c
void G_init(GPIO_Type* GPIOx,GPIO_InitType* GPIO_InitStructure)
{
	int count;
	int pos,countpos,temp;
	//判断GPIO的引脚编号
	if(((uint32_t)GPIO_InitStructure->GPIO_Pins & (uint32_t)0x00ff) != 0 )
	{
		//配置低八位寄存器
		//循环左移count次
		
		for(count = 0;count <8;count++)
		{
			pos = 1 << count;
			countpos = GPIO_InitStructure->GPIO_Pins & pos;
			if(countpos == pos)
			{
				pos = count << 2;
				if((GPIO_InitStructure->GPIO_Mode & 0x10) == 0x10)	
					GPIOx->CTRLL = (GPIOx->CTRLL & ~(0xF << pos)) | ((GPIO_InitStructure->GPIO_Mode & 0xf) << pos) | (GPIO_InitStructure->GPIO_MaxSpeed << pos);
			}
		}		
	}		
	else
	{
		//配置高八位寄存器
		for(count = 0;count <8;count++)
		{
			pos = 1 << (count + 8);
			countpos = GPIO_InitStructure->GPIO_Pins & pos;
			if(countpos == pos)
			{
				pos = count << 2;
				if((GPIO_InitStructure->GPIO_Mode & 0x10) == 0x10)
				{					
					GPIOx->CTRLH = (GPIOx->CTRLH & ~(0xF << pos)) |((GPIO_InitStructure->GPIO_Mode & 0xf) << pos) | (GPIO_InitStructure->GPIO_MaxSpeed << pos);
					
				}
			}
		}	
	}
}

//上述代码只针对输出模式
//过程中碰到好多问题,最大的问题就是手册和宏定义没有仔细看,导致
//GPIOx->CTRLH = (GPIOx->CTRLH & ~(0xF << pos)) |((GPIO_InitStructure->GPIO_Mode & 0xf) << pos) | (GPIO_InitStructure->GPIO_MaxSpeed << pos)一直出错.
//原因就是GPIO_MODE的宏定义除了第四位,高四位也数值,不能直接参与运算,需要与0xF相与,保留低四位.它的第五位用来表示区分输入输出模式
//所以这里的GPIO_InitStructure->GPIO_Mode & 0xF
//还有一些小问题,就是在配置高低寄存器的时候没有注意寄存器的名字一样,都是低八位寄存器,导致第一编译的时候寄存器配置错误.
//另外,对GPIO口的八种输入输出模式,他们的宏定义有了一个新的理解.
typedef enum
{ GPIO_Mode_IN_ANALOG = 0x0,
  GPIO_Mode_IN_FLOATING = 0x04,
  GPIO_Mode_IN_PD = 0x28,
  GPIO_Mode_IN_PU = 0x48,
  GPIO_Mode_OUT_OD = 0x14,
  GPIO_Mode_OUT_PP = 0x10,
  GPIO_Mode_AF_OD = 0x1C,
  GPIO_Mode_AF_PP = 0x18
}GPIOMode_Type;
//他们的低四位用来确定模式,第五位用来确定输入还是输出,配合低四位.
//因为输出模式还有三种功率输出,所以需要通过speed来配置输出功率.
//上面这一串代码有很大的缺陷,就是没有使用中间值,
//接下来采用中间值的方式重新敲一次,
     
```

````c
void gpio_init(GPIO_Type *GPIOx,GPIO_InitType *GPIO_InitStructure)
{
    int pin =0,pinmode = 0,pos = 0,poscount = 0;
    int temp = 0,pinmask = 0;
    
    pinmode = (GPIO_InitStructure->GPIO_Mode & 0xf) | GPIO_InitStructure->GPIO_MaxSpeed;
 
    if((GPIO_InitStructure->GPIO_Pins & 0xff) != 0)  //低八位
    {
     	temp = GPIOx->CTRLL;
        for(pos = 0;pos <8;pos++)
        {
            poscount = 1 << pos;   
            if(GPIO_InitStructure->GPIO_Pins == poscount)
            {
                //如果进入这个条件,说明当前的pos值是就是GPIO的编号
                //知道了GPIO的编号,我们把它左移2位,也就是放大四倍,
                //为什么是放大四倍,因为配置一个GPIO引脚,需要四个位,在寄存器里边,刚好排着顺序,从低到高,
                //每四个位控制一个GPIO引脚,这个GPIO的引脚的编号是多少我们就把它乘以四,便于后续的使用.
                poscount = pos << 2;
                pinmask = 0xf << poscount;  //这句话的意思是把四个1左移poscount位
                temp &= ~pinmask;           //将对应的位屏蔽,因为我们要重新设置对应的位,所以需要提前清零.防止出错
                temp |= pinmode << poscount;//把我们要设置的模式左移到对应的位,然后赋值给temp.
            }
        }
        GPIOx->CTRLL = temp;                //修改GPIO配置寄存器,完成GPIO的配置.
    }
    else                                           //高八位
    {
        temp = GPIOx->CTRLH;
        for(pos = 0;pos <8;pos++)
        {
            poscount = 1 << (pos + 8);
            if(GPIO_InitStructure->GPIO_Pins == poscount)
            {
                poscount = pos << 2;
                pinmask = 0xf << poscount;
                temp &= ~pinmask;
                temp |= pinmode << poscount;
            }
        }
        GPIOx->CTRLH = temp;
    }
}

//上述代码,加入了中间值,一直到最后才修改寄存器的值,避免中间出现未知的错误.
//但是同样只是针对输出模式,
//输入模式需要特殊处理,因为在上拉或下拉的输入,GPIO_Mode的低四位是一样的,都是 0x1000,所以它需要通过高四位的第六位和第七位区分一下.
//那么他们是怎么来区分呢,就是通过GPIO_OPTDT寄存器对应的值来区分,为1时,是上拉输入,为0时,是下拉输入.
//把这个代码加入到上面的if...else...中,就可以连输入的模式都包含进去.
//然后在开头加入一个if条件判断,判断GPIO_InitStructure->GPIO_Mode的第五位,是否为1.
//选择性的添加GPIO_Speed的值.
//缺陷是没有添加判断传入的参数是否符合要求.
````

下面来敲包含输入模式的gpio_init();

```c
void gpio_init(GPIO_Type *GPIOx,GPIO_InitType *GPIO_InitStructure)
{
    int pin =0,pinmode = 0,pos = 0,poscount = 0;
    int temp = 0,pinmask = 0;
    
    pinmode = GPIO_InitStructure->GPIO_Mode & 0xf;
    if((GPIO_InitStructure->GPIO_Mode & 0x10) == 0x10)
    	pinmode |= GPIO_InitStructure->GPIO_MaxSpeed;
    
    if((GPIO_InitStructure->GPIO_Pins & 0xff) != 0)  //低八位
    {
     	temp = GPIOx->CTRLL;
        for(pos = 0;pos <8;pos++)
        {
            poscount = 1 << pos;
            if(GPIO_InitStructure->GPIO_Pins == poscount)
            {
                poscount = pos << 2;
                pinmask = 0xf << poscount;
                temp &= ~pinmask;
                temp |= pinmode << poscount;
                if(GPIO_InitStructure->GPIO_Mode == GPIO_Mode_IN_PU)
                    GPIOx->BSRE |= 1 << poscount;
                else if(GPIO_InitStructure->GPIO_Mode == GPIO_Mode_IN_PD)
                    GPIOx->BRE |= 1 << poscount;
            }
        }
        GPIOx->CTRLL = temp;
    }
    else                                           //高八位
    {
        temp = GPIOx->CTRLH;
        for(pos = 0;pos <8;pos++)
        {
            poscount = 1 << (pos + 8);
            if(GPIO_InitStructure->GPIO_Pins == poscount)
            {
                poscount = pos << 2;
                pinmask = 0xf << poscount;
                temp &= ~pinmask;
                temp |= pinmode << poscount;
                if(GPIO_InitStructure->GPIO_Mode == GPIO_Mode_IN_PU)
                    GPIOx->BSRE |= 1 << poscount;
                else if(GPIO_InitStructure->GPIO_Mode == GPIO_Mode_IN_PD)
                    GPIOx->BRE |= 1 << poscount;
            }
        }
        GPIOx->CTRLH = temp;
    }
}

//上述代码加入了判断输入的模式选择.
//通过开头的一个if条件来判断传入的参数GPIO_Mode的第五位是否为1,来选择性的添加GPIO_Speed的值.
```



