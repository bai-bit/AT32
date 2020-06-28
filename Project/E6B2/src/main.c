#include "systemclk.h"
#include "common.h"
#include "gpio_init.h"
#include "imu_data_decode.h"
#include "packet.h"
#include "uart.h"
#include "rtthread.h"
#include "rthw.h"

#if (APP_PN == 229)
#define SYSTEM_HEAP_BASE        (0x20002000)
#else
#define SYSTEM_HEAP_BASE        (0x20008000)
#endif

#define SYSTEM_HEAP_SIZE        (24*1024)

void init_thread_entry(void* parameter);

void rt_application_init(void* parameter)
{
     rt_thread_startup(rt_thread_create("init", init_thread_entry, RT_NULL, 1024, 10, 20));
}

void dump_data_packet(receive_imusol_packet_t *data);
void serial_init(void);
void simulation_encoder_init(void);
void tmr_init(uint32_t div);

static uint32_t frame_rate;  
static int refer_value;
static uint32_t count;
static float temp_refer_value;

int main(int argc,const char *argv[])
{
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    DelayInit();
    imu_data_decode_init();
    serial_init();
    simulation_encoder_init();
    tmr_init(191);

    while(1)
    {
        refer_value = 0;
        if(receive_imusol.gyr[2] > 2 && receive_imusol.gyr[2] < 280)
        {
            refer_value = receive_imusol.gyr[2];
            TMR2->AR = 1;
            TMR2->CTRL1 = TMR_CTRL1_CNTEN;

//            printf("timer_ms = [%d]\r\n", timer_ms);
        }
        else if(receive_imusol.gyr[2] < -2 && receive_imusol.gyr[2] > -280)
        {
            refer_value = (receive_imusol.gyr[2]);
//            TMR2->AR = 45000 / refer_value * -1;
            TMR2->AR = 1;
            TMR2->CTRL1 = TMR_CTRL1_CNTEN;
//            printf("timer_ms = [%d]\r\n", timer_ms);
        }
        else
        {
            TMR2->CTRL1 = ~TMR_CTRL1_CNTEN;
        }
        
        delayms(10);
    }
}

void tmr_init(uint32_t div)
{
    //初始化定时器2 
    //开启定时器时钟
    RCC->APB1EN |= RCC_APB1EN_TMR2EN;
    //增强模式使能
    TMR2->CTRL1 |= TMR_CTRL1_PMEN;
    TMR2->CTRL1 |= TMR_CTRL1_DIR; 
    TMR2->CTRL1 |= TMR_CTRL1_ARPEN;
    
    TMR2->DIV = div;
    //开启中断
    TMR2->DIE |= TMR_DIE_UEVIE;
    NVIC_Init(TMR2_GLOBAL_IRQn, 2, 3, ENABLE);
}

void simulation_encoder_init(void)
{
    GPIO_Init(HW_GPIOB, GPIO_PIN_8, GPIO_Mode_Out_PP);
    GPIO_Init(HW_GPIOB, GPIO_PIN_9, GPIO_Mode_Out_PP);
    PBout(8) = 0;
    PBout(9) = 0;
    
    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_Out_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_Out_PP);
    PAout(9) = 1;
    PAout(10) = 1;
}

void serial_init(void)
{
     /* serial  1 */
    RCC->APB2EN |= AFIO_ENABLEBIT;
    AFIO->MAP |= AFIO_MAP_USART1_REMAP;
    GPIO_Init(HW_GPIOB, GPIO_PIN_6, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOB, GPIO_PIN_7, GPIO_Mode_IPU);
//    GPIO_Init(HW_GPIOA, GPIO_PIN_9, GPIO_Mode_AF_PP);
//    GPIO_Init(HW_GPIOA, GPIO_PIN_10, GPIO_Mode_IPU);
    UART_Init(HW_USART1, BAUD_115200);
    USART_Cmd(HW_USART1, ENABLE);
    
    /* serial  2 */
    GPIO_Init(HW_GPIOA, GPIO_PIN_2, GPIO_Mode_AF_PP);
    GPIO_Init(HW_GPIOA, GPIO_PIN_3, GPIO_Mode_IN_FLOATING);

    UART_Init(HW_USART2, BAUD_115200);                  
    USART_ITConfig(HW_USART2, UART_INT_RDNE, ENABLE);
    USART_Cmd(HW_USART2, ENABLE);
    NVIC_Init(USART2_IRQn, 2, 2, ENABLE);
}

void systick_start(uint32_t time_us)
{
    DelayInit();
    SysTick->LOAD = (float)GetClock_Frequency(pll) / 8000000 * time_us;
    systick_setexception(true);
    SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
}

void rt_thread_system(void)
{
    rt_hw_interrupt_disable();

    SysTick_Config(GetClock_Frequency(pll) / RT_TICK_PER_SECOND - 1);
    
    rt_system_timer_init();
    rt_system_scheduler_init();

    rt_device_init_all();
    rt_system_timer_thread_init();
    rt_thread_idle_init();
    rt_system_heap_init((void*)SYSTEM_HEAP_BASE, (void*)(SYSTEM_HEAP_BASE + SYSTEM_HEAP_SIZE));  
    rt_application_init(RT_NULL);
    
    rt_system_scheduler_start();
}

void SysTick_Handler(void)
{
    rt_interrupt_enter();
    rt_tick_increase();
    rt_interrupt_leave();
}

/* serial usart2 interrupt functional */
uint8_t systick_true_flag = 1;
void USART2_IRQHandler(void)                	            //串口2中断服务程序
{
    uint8_t ch;
	if(UART_GetInter(HW_USART2, UART_INT_RDNE) != RESET)   //接收中断 
		ch = UART_RecviveData(HW_USART2);	                    //读取接收到的数据
    frame_rate++;
    packet_decode(ch);                                      //解析数据
} 

void dump_data_packet(receive_imusol_packet_t *data)
{
	if(bitmap & BIT_VALID_ID)
		printf("    Device ID:  %-8d\r\n",  data->id);
	printf("   Frame Rate: %4dHz\r\n", frame_rate);
	if(bitmap & BIT_VALID_ACC)
		printf("       Acc(G):	%8.3f %8.3f %8.3f\r\n",  data->acc[0],  data->acc[1],  data->acc[2]);
	if(bitmap & BIT_VALID_GYR)
		printf("   gyr(deg/s):	%8.2f %8.2f %8.2f\r\n",  data->gyr[0],  data->gyr[1],  data->gyr[2]);
 	if(bitmap & BIT_VALID_MAG)
		printf("      mag(uT):	%8.2f %8.2f %8.2f\r\n",  data->mag[0],  data->mag[1],  data->mag[2]);
	if(bitmap & BIT_VALID_EUL)
		printf("   eul(R P Y):  %8.2f %8.2f %8.2f\r\n",  data->eul[0],  data->eul[1],  data->eul[2]);
	if(bitmap & BIT_VALID_QUAT)
		printf("quat(W X Y Z):  %8.3f %8.3f %8.3f %8.3f\r\n",  data->quat[0],  data->quat[1],  data->quat[2],  data->quat[3]);
}

void TMR2_GLOBAL_IRQHandler(void)
{
    temp_refer_value = refer_value;

    TMR2->STS = 0;
    if(temp_refer_value > 0)
        TMR2->AR = 45000 / refer_value;
    else
        TMR2->AR = 45000 / refer_value * -1;
    
    if(temp_refer_value)
    {
        count++;
        if(count % 16 <=3)
        {
            if(temp_refer_value > 0)
            {
                PBout(8) = 0;
                PBout(9) = 1;
            }
            else if(temp_refer_value < 0)
            {
                PBout(8) = 1;
                PBout(9) = 0;
            }
        }
        else if(count % 16 <= 7)
        {
            PBout(8) = 1;
            PBout(9) = 1;
        }
        else if(count % 16 <= 11)
        {
             if(temp_refer_value > 0)
            {
                PBout(8) = 1;
                PBout(9) = 0;
            }
            else if(temp_refer_value < 0)
            {
                PBout(8) = 0;
                PBout(9) = 1;
            }
        }
        else if(count % 16 <= 15)
        {
            PBout(8) = 0;
            PBout(9) = 0;
        }
    }
}  
