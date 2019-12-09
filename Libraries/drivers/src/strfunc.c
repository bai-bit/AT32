#include<strfunc.h>

char *itoa(uint32_t num)
{
	//定义一个数组，用来存放字符
	//把传递过来的数值，对十取取余，把得到的数字加‘0’，然后存放到数组中
	//不考虑负数的情况
	//最后将数组反转，末尾加’\0‘
	//return str
	char str[64] = "";
	static char str1[64] = "hello";
	uint32_t i = 0,j = 0;
	
	for(i = 0;i < 64 && num >= 1;i++)
	{
		str[i]=num%10 + '0';
		num /= 10;
		
	}
	while(i>0)
	{
		str1[j++] = str[i - 1];
		i--;
	}
	str1[j] = ' ';
	str1[j+1] = ' ';
	str1[j+2] = '\0';

	return str1;
}
