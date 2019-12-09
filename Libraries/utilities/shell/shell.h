#ifndef __SHELL_H
#define __SHELL_H
#include<string.h>
#include<led.h>
#include<stdio.h>
#include<gpio_init.h>
#include<uart.h>
#include<stdlib.h>
#include<link.h>

#define SIZE       4096
#define COM_SIZE   6
#define BACKSPACE  0x7f
#define TAB_SIZE   6
#define ENTER      '\r'
#define TAB        '\t'


void shell_loop(void);
int led_red_ctrl(int argc,int data);
int get_timer(int argc,int data);
int reset(int argc,int data);
int clear_screen(int argc,int data);

#endif
