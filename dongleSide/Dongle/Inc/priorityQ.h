#include <stdio.h>
#include <stdlib.h>
 
struct node
{
	uint8_t priority;
	uint8_t info;
	struct node *link;
}*front=NULL;
 
void pushQ(uint8_t item, uint8_t item_priority);
uint8_t popQ();
void displayQ();
uint8_t isEmpty();
 

