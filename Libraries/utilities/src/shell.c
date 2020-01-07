#include<shell.h>

//��ǰʵ�ֵĹ��ܣ��鿴���ִ������
//ȱ�ٵĹ��ܣ��鿴��ʷ���ʹ����������ʷ���
//����shell����

struct shell_functional {
	int   (*fp)(int argc,int data);
	char  *name;
	int   data;
};
typedef struct shell_functional shell_command;

void shell_loop(void)
{
	printf("welcome to shell");
	shell_command shell_command[] = {
		{led_red_ctrl,"led_red_ON",1},
		{led_red_ctrl,"led_red_OFF",0},
		{get_timer,"timer",1},
		{reset,"RESET",1},
		{led_red_ctrl,"led_red_blink",2},
		{clear_screen,"clear",0}
	};                     
	//��ʾ������shell_command�ṹ������µĳ�Աʱ��Ҫȥ�޸�COM_SIZE�����֣������ֵ��������ṹ���еĳ�Ա������
	char *command[COM_SIZE] = {"led_red_ON","led_red_OFF","timer","RESET","led_red_blink","clear"};
	//��ʾ������shell_command�ṹ������µĳ�Աʱ��Ҫ��command����������³�Ա�ı�ʶ����Ҳ��������name��Ա��
	char *temp_command[COM_SIZE] = {""};
	
	char str[SIZE] = "";
	int32_t i = 0,j = 0,k = 0;
	u8 rev;
	u8 flag = 0;
	DLL *head = create_head();
	while(1)
	{
		//�ȴ�����
		//�����ж�
		//���ݽ��յ�����Ϣ������Ӧ����Ϣ
		//fgetc��strncmp
		//���������
		if(strlen(uart_rx_buf) != RESET)
			if(strncmp(uart_rx_buf,"shell_exit",strlen("shell_exit")) == 0)
			{
				printf("exit\r\n");
				return ;
			}
		rev = fgetc(stdin);
		if(rev != ENTER && rev != TAB && rev != BACKSPACE)
		{
			//���뵽���˵������Ҫ�洢���ַ�����Щ�ַ����Ҫִ�е����
			str[i++] = rev;
			if(i > 2)
				for(int m= 0;m<i;m++)
				{
					if(str[m] == 27 && str[m + 1] == '[')
					{
						switch(str[m+2])
						{
							case 'A':
								printf("%c[B",27);
								break;
							case 'B':
								printf("%c[A",27);
								break;
							case 'C':
								printf("left");
								break;
							case 'D':
								printf("right");
								break;
						}
						i=0;
						memset(str,'\0',sizeof(str));
					}
				}
		}
		else if(rev == TAB)
		{
			//���뵽��������ն�������ַ�ƥ�䵱ǰ����������е�����������һ������ӡ���ն��ϣ�������ڶ����ȫ����ʾ���ն���
			str[i] = '\0';
			
			for(j = 0,k = 0;j < COM_SIZE;j++)
				if(strncmp(command[j],str,i) == 0)
					temp_command[k++] = command[j];
								
			if(k == 0)
				printf("This command does not exist\r\n");
			else if(k == 1)
			{
				for(i += TAB_SIZE;i >= 0;i--)
					printf("%c",BACKSPACE);
				
				printf("%s",temp_command[k-1]);
				
				for(i = 0;*temp_command[k-1] != '\0';i++)
					str[i] = (*temp_command[k-1]++);
			}
			else
			{
				printf("\r\n");
				for(;k >= 1;k--)
					printf("%s\t",temp_command[k - 1]);
				printf("\r\nbai@AT32:%s",str);
			}
		}
		else if(rev == BACKSPACE)
		{
			if(i == 0)
				continue;
			i--;
			str[i]='\0';
		}
		else if(rev == ENTER)
		{
			//���뵽���˵�����������Ҫִ������������ִ��֮ǰ����Ҫ�Ƚ����ж��Ƿ����������һ�����
			//������ڣ���ִ����Ӧ�ĺ���������ִ�н���������浽������
			str[i] = '\0';
			
			for(j = 0;j < COM_SIZE;j++)
				if(strcmp(shell_command[j].name,str) == RESET)
				{
					flag = SET;
					shell_command[j].fp(0,shell_command[j].data);
					printf("bai@AT32:");
					install_head(head,shell_command[j].name);
				}
				
			if(flag == RESET)
			{
				printf("bai@AT32:please enter again!\r\nbai@AT32:");
				install_head(head,shell_command[j].name);
			}
			
			if(i == 0)
				continue;
			
			i = 0;
			flag = 0;
			memset(str,0,sizeof(str));
		}
	}
}


int led_red_ctrl(int argc,int data)
{
	int i = 0;
	switch(data)
	{
		case 0:
			GPIO_PinWrite(HW_GPIOD,gpio_pin_13,1);
			break;
		case 1:
			GPIO_PinWrite(HW_GPIOD,gpio_pin_13,0);
			break;
		case 2:
			for(i = 0;i < 50;i++)
			{
				GPIO_PinToggle(HW_GPIOD,gpio_pin_13);
				delayms(300);
			}
	}
	return 0;
}
	

int get_timer(int argc,int data)
{
	printf("\r\nbai@AT32:2019��10��29��\r\n");
	return 0;
}

int reset(int argc,int data)
{
	printf("please waiting....\n");
	NVIC_SystemReset();
	return 0;
}

int clear_screen(int argc,int data)
{
	printf("\e[1;1H\e[2J");
	printf("%c[A",27);
	return 0;
}

