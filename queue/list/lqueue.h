#ifndef __LQUEUE_H__
#define __LQUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct list_node{
	struct list_node *next;
	struct list_node *prev;
} list_node_t;

typedef struct lqueue_data{
	void *pqdata;
	int len;
	list_node_t node;
} lqueue_data_t;

typedef struct lqueue{
	int count;
	int size;
	list_node_t *phead;
} lqueue_t;

int lqueue_init(lqueue_t *pqueue, int size);
int lqueue_enqueue_front_ext(lqueue_t *pqueue, void *pbuf, int length);
int lqueue_enqueue_back_ext(lqueue_t *pqueue, void *pbuf, int length);
int lqueue_enqueue_front(lqueue_t *pqueue, void *pbuf, int length);
int lqueue_enqueue_back(lqueue_t *pqueue, void *pbuf, int length);
int lqueue_dequeue_front(lqueue_t *pqueue, void **pbuf, int *plen);
int lqueue_dequeue_back(lqueue_t *pqueue, void **pbuf, int *plen);
void lqueue_destroy(lqueue_t *pqueue, void(*destroy)(void *data));
bool lqueue_is_full(lqueue_t *pqueue);
bool lqueue_is_empty(lqueue_t *pqueue);
void lqueue_info(lqueue_t *pqueue);

void lqueue_data_destroy_ext(void *pbuf);

#ifdef __cplusplus
}
#endif

#endif
