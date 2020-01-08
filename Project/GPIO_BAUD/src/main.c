#include "systemclk.h"
#include "uart.h"
#include "auto_uart_baud.h"
extern uint32_t auto_baud;


int main(int argc,const char *argv[])
{
    //��ʼ��ʱ��ϵͳ
    //�����ж����ȼ�����
    //ʹ�ܴ��ڣ����ǲ�ʹ�ܶ�Ӧ��GPIO��
    //����Ӧ��GPIO�ڳ�ʼ��Ϊ���룬˫���ش���
    //ʹ��systick
    //��GPIO���жϺ����м��������غ��½���֮���ʱ�䣬�����ʱ�䱣�浽һ�������С�
    //���Ҷ�ʮ�飬Ȼ�����ƽ��ֵ��
    //Ȼ����1000000�������ƽ��ֵ���õ��ľ��ǽ�����baud��ֵ��
    //���ֵ���Ǻ�׼ȷ����Ҫ��һ��ѡ��
    SysClk_PLLEN(PLLCLK_MUL_192MHz);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//  GPIO_Init(HW_GPIOA,GPIO_PIN_9,GPIO_Mode_AF_PP);
//  GPIO_Init(HW_GPIOA,GPIO_PIN_10,GPIO_Mode_IN_FLOATING);
//  UART_Init(HW_USART1,BAUD_115200);
    DelayInit();
    
    auto_uart1_baud();
    
    for(;;)
    {
        printf("welcome to xcom\r\n");
        delayms(500);
    }
}
 


