#include "priorityQ.h"

 
void pushQ(uint8_t item,uint8_t item_priority)
{
	struct node *tmp,*p;
	uint8_t memerr = 1;
		tmp=(struct node *)malloc(sizeof(struct node));
	if(tmp==NULL)
	{
		memerr = 1;//printf("\nMemory not available\n");
		return;
	}
	tmp->info=item;
	tmp->priority=item_priority;
	/*Queue is empty or item to be added has priority more than first element*/
	if( isEmpty() || item_priority < front->priority )
	{
		tmp->link=front;
		front=tmp;
	}
	else
	{
		p = front;
		while( p->link!=NULL && p->link->priority<=item_priority )
			p=p->link;
		tmp->link=p->link;
		p->link=tmp;
	}
}/*End of insert()*/
 
uint8_t popQ()
{
	struct node *tmp;
	uint8_t item;
	uint8_t Qerr;
	if( isEmpty() )
	{
		Qerr = 1;//printf("\nQueue Underflow\n");
		exit(1);
	}
	else
	{
		tmp=front;
		item=tmp->info;
		front=front->link;
		free(tmp);
	}
	return item;
}/*End of del()*/
 
uint8_t isEmpty()
{
	if( front == NULL )
		return 1;
	else
		return 0;
 
}/*End of isEmpty()*/
 
 
void displayQ()
{
	/*struct node *ptr;
	ptr=front;
	if( isEmpty() )
		printf("\nQueue is empty\n");
	else
	{	printf("\nQueue is :\n");
		printf("\nPriority       Item\n");
		while(ptr!=NULL)
		{
			printf("%5d        %5d\n",ptr->priority,ptr->info);
			ptr=ptr->link;
		}
	}*/
}/*End of display() */
