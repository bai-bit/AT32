#include "at32_timer.h"

//初始化定时器
//设置计数器初始值，比较值
//中断更新
//设置pwm输出
//工作过程：一条线程周期性产生pwm波形
//中断函数：更新计数器，比较值
//

void timer_init(uint32_t instance, uint32_t chl, uint32_t val)
{
    
}
void timer_set_int_mode(uint32_t instance, uint32_t chl, uint8_t val)
{
    
}
void timer_clear_counter(uint32_t instance)
{
    
}
void timer_start(uint32_t instance)
{
    
}
void timer_stop(uint32_t instance)
{
    
}
void timer_set_time(uint32_t instance, uint32_t chl, uint32_t us)
{
    
}
void timer_set_value(uint32_t instance, uint32_t chl, uint32_t val)
{
    
}
void timer_set_one_shot(uint32_t instance, uint32_t chl, uint8_t val)
{
    
}
uint32_t timer_get_value(uint32_t instance, uint32_t chl)
{
    
}
void timer_set_to_value(uint32_t instance, uint32_t chl, uint32_t val)
{
    
}