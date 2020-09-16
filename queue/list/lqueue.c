#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lqueue.h"

#define _QDEBUG

#ifdef _QDEBUG
#define _qprintf(fmt, args...) printf(fmt, ##args)
#else
#define _qprintf(fmt, args...)
#endif

#define lqueue_container_of(ptr, type, member) \
	((type *)((char *)ptr - (unsigned long)(&(((type *)0)->member))))

int lqueue_init(lqueue_t *pqueue, int size)
{
	if (pqueue == NULL){
		return -11;
	}

	pqueue->count = 0;
	pqueue->size = size;
	pqueue->phead = NULL;
	return 0;
}

void lqueue_data_destroy_ext(void *pbuf)
{
	if(pbuf != NULL){
		_qprintf("queue destroy one data!\n");
		free(pbuf);
	}
	pbuf = NULL;
}

int lqueue_enqueue_front_ext(lqueue_t *pqueue, void *pbuf, int length)
{
	if ((pqueue == NULL) || (pbuf == NULL)){
		return -11;
	}

	if((pqueue->size > 0) && (pqueue->count >= pqueue->size)){ // overflow
		_qprintf("Warn: queue is full!\n");
		return -1;
	}
	
	lqueue_data_t *pdata = (lqueue_data_t *)malloc(sizeof(lqueue_data_t));	
	if (pdata == NULL){
		_qprintf("Warn: malloc qdata error!\n");
		return -10;
	}
	pdata->pqdata = (unsigned char *)malloc(length);
	if(pdata->pqdata == NULL){
		_qprintf("Warn: malloc data error!\n");
		free(pdata);
		return -10;
	}
	memcpy(pdata->pqdata, pbuf, length);
	pdata->len = length;

	// insert into list back 
	if(pqueue->phead == NULL){
		(pdata->node).next = &(pdata->node); 
		(pdata->node).prev = &(pdata->node);
		pqueue->phead = &(pdata->node);
	} else {
		(pdata->node).next = pqueue->phead; 
		pqueue->phead->prev->next = &(pdata->node);
		(pdata->node).prev = pqueue->phead->prev;
		pqueue->phead->prev = &(pdata->node);
		pqueue->phead = &(pdata->node);
	}
	pqueue->count++;
	return 0;
}

int lqueue_enqueue_back_ext(lqueue_t *pqueue, void *pbuf, int length)
{
	if ((pqueue == NULL) || (pbuf == NULL)){
		return -11;
	}
	if((pqueue->size > 0) && (pqueue->count >= pqueue->size)){ // overflow
		_qprintf("Warn: queue is full!\n");
		return -1;
	}
	
	lqueue_data_t *pdata = (lqueue_data_t *)malloc(sizeof(lqueue_data_t));	
	if (pdata == NULL){
		_qprintf("Warn: malloc qdata error!\n");
		return -10;
	}
	pdata->pqdata = (unsigned char *)malloc(length);
	if(pdata->pqdata == NULL){
		_qprintf("Warn: malloc data error!\n");
		free(pdata);
		return -10;
	}
	memcpy(pdata->pqdata, pbuf, length);
	pdata->len = length;

	// insert into list back 
	if(pqueue->phead == NULL){
		(pdata->node).next = &(pdata->node); 
		(pdata->node).prev = &(pdata->node);
		pqueue->phead = &(pdata->node);
	} else {
		(pdata->node).next = pqueue->phead; 
		pqueue->phead->prev->next = &(pdata->node);
		(pdata->node).prev = pqueue->phead->prev;
		pqueue->phead->prev = &(pdata->node);
//		pqueue->phead = &(pdata->node);
	}
	pqueue->count++;
	return 0;
}

int lqueue_enqueue_front(lqueue_t *pqueue, void *pbuf, int length)
{
	if ((pqueue == NULL) || (pbuf == NULL)){
		return -11;
	}

	if((pqueue->size > 0) && (pqueue->count >= pqueue->size)){ // overflow
		_qprintf("Warn: queue is full!\n");
		return -1;
	}
	
	lqueue_data_t *pdata = (lqueue_data_t *)malloc(sizeof(lqueue_data_t));	
	if (pdata == NULL){
		_qprintf("Warn: malloc qdata error!\n");
		return -10;
	}
	pdata->pqdata = pbuf;
	pdata->len = length;

	// insert into list back 
	if(pqueue->phead == NULL){
		(pdata->node).next = &(pdata->node); 
		(pdata->node).prev = &(pdata->node);
		pqueue->phead = &(pdata->node);
	} else {
		(pdata->node).next = pqueue->phead; 
		pqueue->phead->prev->next = &(pdata->node);
		(pdata->node).prev = pqueue->phead->prev;
		pqueue->phead->prev = &(pdata->node);
		pqueue->phead = &(pdata->node);
	}
	pqueue->count++;
	return 0;
}

int lqueue_enqueue_back(lqueue_t *pqueue, void *pbuf, int length)
{
	if ((pqueue == NULL) || (pbuf == NULL)){
		return -11;
	}

	if((pqueue->size > 0) && (pqueue->count >= pqueue->size)){ // overflow
		_qprintf("Warn: queue is full!\n");
		return -1;
	}
	
	lqueue_data_t *pdata = (lqueue_data_t *)malloc(sizeof(lqueue_data_t));	
	if (pdata == NULL){
		_qprintf("Warn: malloc qdata error!\n");
		return -10;
	}
	pdata->pqdata = pbuf;
	pdata->len = length;

	// insert into list back 
	if(pqueue->phead == NULL){
		(pdata->node).next = &(pdata->node); 
		(pdata->node).prev = &(pdata->node);
		pqueue->phead = &(pdata->node);
	} else {
		(pdata->node).next = pqueue->phead; 
		pqueue->phead->prev->next = &(pdata->node);
		(pdata->node).prev = pqueue->phead->prev;
		pqueue->phead->prev = &(pdata->node);
//		pqueue->phead = &(pdata->node);
	}
	pqueue->count++;
	return 0;
}

int lqueue_dequeue_front(lqueue_t *pqueue, void **pbuf, int *plen)
{
	if ((pqueue == NULL) || (pbuf == NULL)|| plen == NULL){
		return -11;
	}

	if(pqueue->phead == NULL){ // empty 
		_qprintf("Warn: queue is empty!\n");
		return -1;
	}

	// dequeue front
	lqueue_data_t *ppdata = lqueue_container_of(pqueue->phead, lqueue_data_t, node);
	if (pqueue->phead->next == pqueue->phead){ // only one element 
		pqueue->phead = NULL;
	} else {
		pqueue->phead->prev->next = pqueue->phead->next;
		pqueue->phead->next->prev = pqueue->phead->prev;
	}
	pbuf = ppdata->pqdata;
	*plen = ppdata->len;
	free(ppdata);

	pqueue->count--;
	return 0;
}

int lqueue_dequeue_back(lqueue_t *pqueue, void **pbuf, int *plen)
{
	if ((pqueue == NULL) || (pbuf == NULL)|| plen == NULL){
		return -11;
	}
	if(pqueue->phead == NULL){ // empty 
		_qprintf("Warn: queue is empty!\n");
		return -1;
	}

	// dequeue back 
	lqueue_data_t *ppdata ;
	if (pqueue->phead->next == pqueue->phead){ // only one element 
		ppdata = lqueue_container_of(pqueue->phead, lqueue_data_t, node);
		pqueue->phead = NULL;
	} else {
		ppdata = lqueue_container_of(pqueue->phead->prev, lqueue_data_t, node);
		pqueue->phead->prev->prev->next = pqueue->phead;
		pqueue->phead->prev = pqueue->phead->prev->prev;
	}
	*pbuf = ppdata->pqdata;
	*plen = ppdata->len;
	free(ppdata);

	pqueue->count--;
	return 0;
}
void lqueue_destroy(lqueue_t *pqueue, void(*destroy)(void *data))
{
	if (pqueue == NULL){
		return;
	}

	lqueue_data_t *pdata = NULL;
	while(pqueue->phead !=NULL){
		pdata = lqueue_container_of(pqueue->phead, lqueue_data_t, node);
		if(pqueue->phead == pqueue->phead->next){
			pqueue->phead = NULL;
		} else {
			pqueue->phead = pqueue->phead->next;
		}
		if(destroy != NULL){
			destroy(pdata->pqdata);
		}
		free(pdata);
		pdata = NULL;
	}

	pqueue->count = 0;
	pqueue->size = 0;
}

bool lqueue_is_full(lqueue_t *pqueue)
{
	bool ret = false;
	if((pqueue->size > 0) && (pqueue->count >= pqueue->size)){ // overflow
		ret = true;
	}
	return ret;
}

bool lqueue_is_empty(lqueue_t *pqueue)
{
	bool ret = false;
	if(pqueue->phead == NULL){ // empty 
		ret = true;
	}
	return ret;
}

void lqueue_info(lqueue_t *pqueue)
{
	if (pqueue == NULL){
		_qprintf("queue is NULL!\n");
		return ;
	}
	printf("queue count: %d, size: %d\n", pqueue->count, pqueue->size);
	if(pqueue->phead == NULL){
		return;
	}

	int i = 0;
	lqueue_data_t *pdata = NULL;
	list_node_t *pnode = pqueue->phead;

	do {
		pdata = lqueue_container_of(pnode, lqueue_data_t, node);
		printf("The %dth item info is [%d]:[%p]\n", i++, pdata->len, pdata->pqdata);
		pnode = pnode->next;
	} while(pnode != pqueue->phead);
}
