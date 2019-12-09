#include<systick.h>

static __IO float fac_us;
static __IO float fac_ms;


void DelayInit(void)
{
	//传入内部时钟源的频率
	//初始化systick定时器
	//选择时钟源
	//用传入的hclk时钟源,计算出ms和us的周期数
	//初始化fac_ms,fac_us.
	SysTick->CTRL &= ~(SysTick_CTRL_CLKSOURCE_Msk);
	
	fac_us = (float)getclock_frequency(pll) / 8000000;
	fac_ms = fac_us * 1000;
}

void delayms(uint32_t num)
{
	//由于重装载寄存器只有24位来保存数据,所以传入的数不能太大,
	//如果传入的数据过大,需要分多次写入重装载寄存器.
	//计算最大值
	uint32_t temp = 0;
	SysTick->LOAD = fac_us * 1000;
	SysTick->VAL = SysTick->LOAD;
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
	while((num)--)
	{
		do{
				temp = SysTick->CTRL;
			}while((temp & SysTick_CTRL_ENABLE_Msk ) && !(temp & SysTick_CTRL_COUNTFLAG_Msk));
	}
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void delayus(uint32_t num)
{
	//把传入的延时时间乘以fac_us写入重装载寄存器
	//开启定时器
	//用do...while()出口条件循环来判断控制寄存器的第十六位
	//当第十六位为1时,关闭定时器.
	
	uint32_t temp = 0;
	
	SysTick->LOAD = (num) * fac_us;
	SysTick->VAL = 0;
	SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
	do{
		 temp = SysTick->CTRL;
    }while((temp & SysTick_CTRL_ENABLE_Msk ) && !(temp & SysTick_CTRL_COUNTFLAG_Msk));
	
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}


/*
** @brief   enable or disable systick
** @note    none
** @param   val:
**          @arg  true    enable systick exception request
**          @arg  false   disable systick exception request
** @retval  none
*/
void systick_setexception(bool val)
{
	val == true ? (SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk) : (SysTick->CTRL &= ~(SysTick_CTRL_TICKINT_Msk)); 
}



