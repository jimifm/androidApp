/*
 * msgqueue.h
 *
 *  Created on: 2013-6-13
 *      Author: Yichou
 */

#ifndef MSGQUEUE_H_
#define MSGQUEUE_H_

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Node * NODE;
typedef struct HeadNode * QUEUE;

typedef int Element;

#define QUEUE_SIZE sizeof(struct HeadNode)
#define NODE_SIZE sizeof(struct Node)
#define NEW_NODE (NODE)malloc(NODE_SIZE);


struct HeadNode {
	Element element;
	NODE next;
	NODE end;

	pthread_cond_t  not_empty; 	//条件 空
	pthread_mutex_t mutex; 		//互斥锁
};

struct Node {
	Element element;
	NODE next;
};


QUEUE new_queue();
void delete_queue(QUEUE queue);

void enqueue(QUEUE queue, Element e);
Element dequeue(QUEUE queue);

#ifdef __cplusplus
}
#endif

#endif /* MSGQUEUE_H_ */
