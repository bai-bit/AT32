#ifndef __LINK_H
#define __LINK_H
#include<stdlib.h>
#include<at32f4xx.h>
#define HOIS_SIZE 100

typedef char data_t;

typedef struct node {
	data_t *history_command;
	struct node *before_node;
	struct node *after_node;
}DLL;

DLL *create_head(void);
DLL * create_node(data_t *data);
void install_head(DLL *head,data_t *data);
void deleter_end(DLL *head);

#endif
