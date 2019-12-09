# shell

#### 工具：电脑，AT32F403ZGT6开发板，逻辑分析仪

#### 软件工具：Keil5，putty串口工具，xcom串口调试助手

​	shell是一种人机交互的接口，由于单片机上的资源十分紧张，无法承担图形界面带来的巨大资源消耗。这时候，命令行就排上用场。我们可以用它来调试单片机中的程序。获取到程序某一时刻的运行状态。

​	我自己编写了一个超级简单的shell，实现的功能也很简单，控制led的亮灭，闪烁，向终端打印信息，重启开发板，清屏命令。

​	思路：

​	1、封装一个结构体，这个结构体中包含一个函数指针，一个标识符，一个传递给函数指针的参数。

​	2、将所有用到的函数，都封装成这样的一个结构体，组成一个结构体数组。

​	3、读取终端上的输入，按enter键结束输入。

​	4、将输入和结构体数组的标识符匹配，匹配成功，执行函数。执行结束后，向终端上打印__ok__。

​	5、在输入的过程中，我们会遇到忘记命令，只记得开头的开头的几个字符。这时，我想让电脑提示一下，我设置TAB键，提示当前程序中记录的以用户输入的几个字符为开头的指令，统统打印到终端上。如果只有一个，则自动补全命令，用户只需按下enter键，结束输入，让程序执行输入的命令。

​	6、有了提示，那么肯定会有输入错误的情况。为了实现输入提示，我们会在打开putty的时候设置终端输入立即发送。就是说，当我在putty的窗口上输入一个字符，尽管我还没有按下enter键，但是这个字符已经通过串口发送到开发板上。这时，我们不小心输入错误，错误的字符已经发送到开发板上了，怎么办。当我们按下backspace键的时候，他会发送一个字符。它的__ascii__码是__0x7f__，只要我们在开发板上设置一下，当接收到0x7f的字符，我们减掉一个输入的字符就好了。

​	7、有时候我们会输入一个错误的命令，我们应该提示一下用户，please input again。然后等待用户重新输入。

​	8、正在实现的功能：查看历史命令。

​	当用户按下光标上移或光标下移的两个方向键时，应该能查看历史命令。

​	9、构思过程中：shell只是一个调试用的工具，并不是我们需要的主要程序，所以，我们不希望处理器一直都在shell中等待。

​	一种方式是在shell的死循环中运行：在我们需要的时候，通过串口发送一个shell_start这样的一个信号，来调起shell调试程序，不用的时候，发送shell_exit退出shell程序。其他时间，处理器一直工作在任务程序中，一个我们真正需要工作的程序。

​	没有操作系统的支持，只是在裸机环境中。利用RTC的计数器，中断，来轮询串口输入的信息是否为shell_start数据。考虑到中断的优先级，我们设置的时候，串口的优先级要高于RTC的中断。

​	另外一种方式是：时间片轮转，利用RTC的中断服务函数，不断的轮询串口接收到的信息，对信息进行处理，这种方式有可能会造成相应不够及时。

接下来记录我过程中出现的问题：

​	刚开始我编写了一个框架，就是putty的窗口输入一个字符，然后发送到开发板，接着开发板再发送到putty的窗口上。流程非常简单。在我编译，下载到开发板上，打开putty，配置好串口，弹出一个黑窗口，这时候，在这个黑窗口上，我无法输入数据。不管按什么键都没反应。

​	排查：打开我的xcom串口助手，提示串口被占用。这个好办，关掉putty，这个提示说明我的putty是连接上开发板了。接下来，我用xcom发送一个字符，没反应，说明是我的程序出问题了，我打开程序看了看，哦，没有初始化串口和时钟系统。

​	结果：修改完代码后，编译下载，成功回显。

​	好，到这里，说明我的程序没有问题了。

​	问题出在哪里呢？出在putty的配置上，在刚打开putty的时候，出现的是一个putty configuration的配置窗口，它还有其他的配置，我看了看，经过下面的配置，可以正常输入了：

设置putty的选项：

​	Teminal->Line discipline options->local echo:选择Force on

​	Teminal->Line discipline options->local line editing:选择Force off

​	Window->Translation->Remote character set:选择Use font encoding

​	SSH->Serial:选择串口的配置，最后点击Open，完成连接。

​	这个时候，最后的结果是成功回显，接着往下走，完善程序。

​	接着下来，我先接收一个命令，就是控制led的亮灭。

​	写完之后，编译，下载。

​	在终端上输入设置好的命令，led灯没反应。

​	排除：也只能代码的问题了。查代码，仔细一看，好吧，没有初始化led灯..........我也真是够菜的。。。。。

​	解决：添加led灯的初始化函数，添加延时函数的初始化。

​	编译，下载。输入命令，led实现预期的动作。

​	接下开始正式的编写shell的框架，按照上面写的思路，进行编写。

​	重定义fgetc函数

```c
static u8 rev1;
static u8 flag = 0;
void USART1_IRQHandler(void)
{
	//接受数据
	//判断接受标志位
	//判断接受的数据，0x0a 0x0d	
	u8 rev = 0;
	
	if(uart_getinter(HW_USART1, uart_int_rdne) == SET)
	{
		rev = uartrecvive_data(HW_USART1);
		rev1 = rev;
		flag = 1;
		
		if((uart_rx_status & UART_STATUS_NLINE) == RESET)
		{
			if(uart_rx_status & UART_STATUS_ENTER)
			{
				if(rev == 0x0a)
					uart_rx_status |= UART_STATUS_NLINE;
				else
					uart_rx_status = RESET;
			}
			else
			{
				if(rev == 0x0d)
					uart_rx_status |= UART_STATUS_ENTER;
				else
				{
					uart_rx_buf[uart_rx_status & UART_STATUS_MASK] = rev;
					uart_rx_status ++;
					if(uart_rx_status > UART_SIZE)
						uart_rx_status = RESET;
				}
			}
		}
	}
}

int fgetc(FILE *stream) //在这里重新定义fgetc的动作
{
	while(flag == 0)
		continue;
	flag = 0;
	
	return rev1;	
}
```



利用结构体数组来封装函数，

```c
struct shell_functional {
	int   (*fp)(int argc,int data);
	char  *name;
	int   data;
};
typedef struct shell_functional shell_command;
//上面这两个是全局变量。
shell_command shell_command[] = {
		{led_red_ctrl,"led_red_ON",1},
		{led_red_ctrl,"led_red_OFF",0},
		{get_timer,"timer",1},
		{reset,"RESET",1},
		{led_red_ctrl,"led_red_blink",2},
		{clear_screen,"clear",0}
	};
	char *command[COM_SIZE] = {"led_red_ON","led_red_OFF","timer","RESET","led_red_blink","clear"};
//这里的两个是放在shell_loop()函数中
```

下面贴出shell_loop的函数：

```c
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
	char *command[COM_SIZE] = {"led_red_ON","led_red_OFF","timer","RESET","led_red_blink","clear"};
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
		if(rev != '\r' && rev != '\t' && rev != BACKSPACE)
		{
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
		else if(rev == '\t')
		{
			//进入到这里，根据终端输入的字符匹配当前命令表中现有的命令，如果存在一个，打印到终端上，如果存在多个，全部显示到终端上
			str[i] = '\0';
			for(j = 0,k = 0;j < COM_SIZE;j++)
				if(strncmp(command[j],str,i) == 0)
				{
					temp_command[k++] = command[j];
				}
			if(k == 0)
				printf("This command does not exist\r\n");
			else if(k == 1)
			{
				for(i += TAB_SIZE;i >= 0;i--)
					printf("%c",BACKSPACE);
				printf("bai@AT32:%s",temp_command[k-1]);
				
				for(i = 0;*temp_command[k-1] != '\0';i++)
					str[i] = (*temp_command[k-1]++);
				
			}
			else
			{
				printf("\r\n");
				for(;k >= 1;k--)
					printf("%s\t",temp_command[k - 1]);
				printf("\r\n%s",str);
			}
		}
		else if(rev == BACKSPACE)
		{
			if(i == 0)
                continue;
			i--;
			str[i]='\0';
		}
		else if(rev == '\r')
		{
			str[i] = '\0';
			for(j = 0;j < COM_SIZE;j++)
				if(strcmp(shell_command[j].name,str) == RESET)
				{
					flag = SET;
					shell_command[j].fp(0,shell_command[j].data);
					printf("\r\n");
					install_head(head,shell_command[j].name);
				}
			if(flag == RESET)
			{
				printf("please enter again!\r\n");
			}
			if(i == 0)
				continue;
			i = 0;
			flag = 0;
			memset(str,0,sizeof(str));
		}
	}
}
//这个函数的功能并不完善，上下方向键的查看历史命令的功能，做了一半。
//剩下的部分是：需要把方向键和链表的遍历关联起来。
//还有的就是输出打印的部分，还有很大的美化空间，比如在终端打印““用户名”@“主机名”：”，这种格式，来等待用户输入。
//方向键的ascii码，不是一个。
//在技术大牛的提示下，用逻辑分析仪，查看出四个方向键对应的ascii码。
//每一个方向键都是由三个ascii码组成：
//上：27 '[' 'A'
//下：27 '[' 'B'
//右：27 '[' 'C'
//左：27 '[' 'D'
//还有个清屏命令：printf("\e[1;1H\e[2J");
//只要向终端上打印这个指令，就可以完成清屏操作。
//还有就是在调起和退出shell_loop循环的处理。这一块基本都是纯软件的逻辑关系了。有时间再搞。
//还有个功能是RTC的中断轮询，终端发送的信息。来决定shell_loop（）函数的状态。
//在编写这个函数的过程中，遇到很多编程上的问题，比如，不用的指针变量，不要留着。容易变成野指针。
//指针变量要初始化，一定要初始化。比如char *p = NULL；这样的。
//还有指针的应用，这个只能是多加练习了。
```

为了实现查询历史命令，我使用链表来保存历史命令。下面我贴出链表的代码：

```c
typedef char data_t;

typedef struct node {
	data_t *history_command;     //存放数据的成员，可以根据需要随意更改数据类型。
	struct node *before_node;
	struct node *after_node;
}DLL;                            //双向循环链表

DLL *create_head()               //创建链表的头指针
{
	DLL *head = (DLL *)malloc(sizeof(DLL));
	head->before_node = head->after_node = head;
	return head;
}

DLL * create_node(data_t *data)  //创建链表的结点
{
	DLL *node = (DLL *)malloc(sizeof(DLL));
	node->history_command = data;
	node->after_node = node->before_node = NULL;
	return node;
}

void install_head(DLL *head,data_t *data)   //在链表中插入数据节点
{
	DLL *node = create_node(data);
		
	node->after_node = head->after_node;
	head->after_node->before_node = node;
	head->after_node = node;
	node->before_node = head;
	tatol++;
	if(tatol == HOIS_SIZE)
		deleter_end(head);
}

void deleter_end(DLL *head)   //删除链表中的数据节点
{
	if(head->after_node == head->before_node)
		return ;
	
	DLL *temp = head->before_node;
	temp->before_node->after_node = head;
	head->before_node = temp->before_node;
	free(temp);
	temp = NULL;
}
//在编译的时候，出现这么一个错误：
.\Flash\Template.axf: Error: L6915E: Library reports error: __use_no_semihosting was requested, but _ttywrch was referenced
Not enough information to list load addresses in the image map.
Finished: 1 information, 0 warning and 1 error messages.
//库报告错误，声明不使用半主机模式，但是引用了_ttywrch.
//解决的办法有两个，一种是勾选options for target ->USE MicroLib。
//另一种是需要重新定义_ttywrch函数
_ttywrch(int ch)
{
	ch = ch;
}
//在代码中添加这个，就好了。不过多了一条警告。
//说明：重定义_ttyerch的函数，在编译的时候虽然没有提示出错和警告。
//但是程序却在使用这些函数的时侯跑飞了。
//但是采用第一种方法，勾选USE MicrolLib，成功运行。
//所以第二种方法可以使用，但是需要了解_ttywrch函数的重定义。
//不了解的就直接勾选，方便快捷。
```

这是一个非常简单的shell，后期我会不断的丰富它的功能，美化命令行的显示。添加命令说明，在终端上输入一个shell_ls,可以把当前程序中包含的命令全部打印出来，并附带命令描述。

还有就是结构体数组的大小，它会随着命令的不断丰富，而不断增加，在后面的程序中，我需要知道当前这个结构体数组的大小。应该使用一个函数，来获取结构体数组的大小，而不是每次增加成员之后，在去修改相应的宏定义，会比较麻烦。

或者是将封装函数的结构体放入到链表中，这样做的话，有一个好处，就是我可以通过终端的输入，来制作一个命令。缺点就是内存的利用率不高。不如数组的利用率高，但是胜在灵活性高。



​																					                                       			2019年10月30日 星期三  __20:00__

