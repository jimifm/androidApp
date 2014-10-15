#include "msgqueue.h"

//#ifdef __cplusplus
//extern "C"
//{
//#endif

/**
 * 创建队列
 *
 * 头结点并不存储数据
 * 头结点->next 为第一个节点
 */
QUEUE new_queue()
{
	QUEUE q = (QUEUE)malloc(QUEUE_SIZE);

	q->next = NULL;
	q->end = NULL;

	pthread_mutex_init(&(q->mutex), NULL);
	pthread_cond_init(&(q->not_empty), NULL);

	return q;
}

int is_empty(QUEUE queue)
{
	return (queue->next == NULL);
}

void delete_queue(QUEUE queue)
{
	pthread_mutex_lock(&(queue->mutex));

	pthread_cond_destroy(&(queue->not_empty));

	pthread_mutex_unlock(&(queue->mutex));
	pthread_mutex_destroy(&(queue->mutex));

	while(!is_empty(queue)){
		dequeue(queue);
	}
	free(queue);
}

void enqueue(QUEUE queue, Element e)
{
	int wakeup = 0;

	pthread_mutex_lock(&(queue->mutex));
	{
		NODE n0 = NEW_NODE;
		NODE oldEnd = queue->end;

		if(is_empty(queue))
			wakeup = 1;

		n0->element = e;
		n0->next = NULL;

		if(oldEnd)
			oldEnd->next = n0;
		else
			queue->next = n0;

		queue->end = n0;
	}
	pthread_mutex_unlock(&(queue->mutex));

	if (wakeup)
		pthread_cond_signal(&(queue->not_empty)); //唤醒第一个调用 pthread_cond_wait() 而进入睡眠的线程
}

Element dequeue(QUEUE queue)
{
	Element e;

	pthread_mutex_lock(&(queue->mutex));
	{
		NODE n0;

		if(is_empty(queue)){ //阻塞等待消息可取
			pthread_cond_wait(&(queue->not_empty), &(queue->mutex));
		}

		n0 = queue->next;
		e = n0->element;
		queue->next = n0->next;
		if(n0->next == NULL)
			queue->end = NULL;

		free(n0);
	}
	pthread_mutex_unlock(&(queue->mutex));

	return e;
}

//#ifdef __cplusplus
//}
//#endif

