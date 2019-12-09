#include<shell.h>

//当前实现的功能，查看命令，执行命令
//缺少的功能：查看历史命令，使用链表保存历史命令。
//进出shell功能

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
	//提示：当向shell_command结构体添加新的成员时，要去修改COM_SIZE的数字，这个数值就是这个结构体中的成员个数。
	char *command[COM_SIZE] = {"led_red_ON","led_red_OFF","timer","RESET","led_red_blink","clear"};
	//提示：当向shell_command结构体添加新的成员时，要在command数组中添加新成员的标识符，也就是他的name成员。
	char *temp_command[COM_SIZE] = {""};
	
	char str[SIZE] = "";
	int32_t i = 0,j = 0,k = 0;
	u8 rev;
	u8 flag = 0;
	DLL *head = create_head();
	while(1)
	{
		//等待接收
		//接收判断
		//根据接收到的信息返回相应的信息
		//fgetc，strncmp
		//建立命令集合
		if(strlen(uart_rx_buf) != RESET)
			if(strncmp(uart_rx_buf,"shell_exit",strlen("shell_exit")) == 0)
			{
				printf("exit\r\n");
				return ;
			}
		rev = fgetc(stdin);
		if(rev != ENTER && rev != TAB && rev != BACKSPACE)
		{
			//进入到这里，说明是需要存储的字符，这些字符组成要执行的命令。
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
			//进入到这里，根据终端输入的字符匹配当前命令表中现有的命令，如果存在一个，打印到终端上，如果存在多个，全部显示到终端上
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
			//进入到这里，说明输入结束，要执行输入的命令。在执行之前，需要先进行判断是否存在这样的一条命令。
			//如果存在，就执行相应的函数，并将执行结束的命令保存到链表中
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
	printf("\r\nbai@AT32:2019年10月29日\r\n");
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

