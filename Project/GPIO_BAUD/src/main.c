#include<systemclk.h>
#include<uart.h>
#include<auto_uart_baud.h>



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
	sysclk_PLLEN(PLLCLK_MUL_192MHz);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//uart1_init(115200);
	DelayInit();
	
	auto_uart1_baud();

	for(;;)
		printf("welcome to xcom");
	
}
 


