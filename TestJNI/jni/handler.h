/*
 * handler.h
 *
 *  Created on: 2013-6-13
 *      Author: Yichou
 */

#ifndef HANDLER_H_
#define HANDLER_H_

#include <pthread.h>

#include "msgqueue.h"

enum {
	MSG_EXIT = -1000
};

typedef struct {
	int what;
	int arg0;
	int arg1;
	void *data;
}T_MSG, *PT_MSG;

typedef void (*Callback)(PT_MSG msg);

#define NEW_MSG (PT_MSG)malloc(sizeof(T_MSG))


class Handler
{
private:
	Callback mCb;
	QUEUE mQueue;
	pthread_t mTid;

	friend void * mainLoop(void * arg);

public:
	Handler();
	Handler(Callback cb);
	~Handler();

	void setCallback(Callback cb);
	void sendMsg(PT_MSG msg);
	void sendMsg(int what, int arg0, int arg1, void *data);
	void sendEmptyMsg(int what);
	void exit();
	void print();
};

#endif /* HANDLER_H_ */
