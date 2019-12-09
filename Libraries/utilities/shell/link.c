#include<link.h>

uint32_t tatol = 0;
DLL *create_head()
{
	DLL *head = (DLL *)malloc(sizeof(DLL));
	head->before_node = head->after_node = head;
	return head;
}

DLL * create_node(data_t *data)
{
	DLL *node = (DLL *)malloc(sizeof(DLL));
	node->history_command = data;
	node->after_node = node->before_node = NULL;
	return node;
}

void install_head(DLL *head,data_t *data)
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

void deleter_end(DLL *head)
{
	if(head->after_node == head->before_node)
		return ;
	
	DLL *temp = head->before_node;
	temp->before_node->after_node = head;
	head->before_node = temp->before_node;
	free(temp);
	temp = NULL;
}
