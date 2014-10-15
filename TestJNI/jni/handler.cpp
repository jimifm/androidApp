#include "handler.h"
#include <android/log.h>


#define LOG_TAG "handler"

#define LOGI(...) \
	((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) \
	((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) \
	((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGD(...) \
	((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))



void * mainLoop(void * arg)
{
	Handler *h = (Handler *)arg;

	h->print();

	while(1)
	{
		PT_MSG msg = (PT_MSG) dequeue(h->mQueue);

		switch(msg->what)
		{
		case MSG_EXIT:
			pthread_exit((void *)"I'm died...");
			break;

		default:
			if(h->mCb != NULL)
				h->mCb(msg);
			break;
		}

		free(msg);
	}
}

Handler::Handler()
{
	Handler(NULL);
}

Handler::Handler(Callback cb)
	:mCb(cb)
{
	mQueue = new_queue();
	pthread_create(&mTid, NULL, mainLoop, this);
}

Handler::~Handler()
{
	LOGD("~Handler()");
}

void Handler::print()
{
	LOGD("hello I'm handler:{tid=%d, cb=%#p}", mTid, mCb);
}

void Handler::setCallback(Callback cb)
{
	mCb = cb;
}

void Handler::exit()
{
	char *msg;

	sendEmptyMsg(MSG_EXIT);
	pthread_join(mTid, (void **)&msg);
	LOGD("handler pthread_join finish, msg=%s", msg);

	delete_queue(mQueue);
}

void Handler::sendMsg(PT_MSG msg)
{
	enqueue(mQueue, (Element) msg);
}

void Handler::sendMsg(int what, int arg0, int arg1, void *data)
{
	PT_MSG msg = NEW_MSG;

	msg->what = what;
	msg->arg0 = arg0;
	msg->arg1 = arg1;
	msg->data = data;

	sendMsg(msg);
}

void Handler::sendEmptyMsg(int what)
{
	sendMsg(what, 0, 0, NULL);
}
