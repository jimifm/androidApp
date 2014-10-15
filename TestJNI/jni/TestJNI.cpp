#include <unistd.h>
#include <netdb.h>
#include <endian.h>
#include <arpa/inet.h>

#include <jni.h>
#include <android/log.h>

#include "handler.h"

#define LOG_TAG "TestMsgQueue"

#define LOGI(...) \
	((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) \
	((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) \
	((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGD(...) \
	((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))


//typedef struct {
//	int what;
//	int arg0;
//	int arg1;
//	void *data;
//}T_MSG, *PT_MSG;
//
//#define NEW_MSG (PT_MSG)malloc(sizeof(T_MSG))


QUEUE mQueue;
pthread_t pid;
Handler *mHandler;


int mr_getHostByName_block(const char *ptr)
{
	char **pptr;
	struct hostent *hptr;
	char str[64];
	int err;
	int ret;

	LOGI("mr_getHostByName(%s)", ptr);

	//查询DNS
	if ((hptr = gethostbyname(ptr)) == NULL)
	{
		LOGE(" error host!");
		return -1;
	}

	//主机规范名
	LOGI(" official hostname:%s", hptr->h_name);

	//获取主机别名列表char *[]
	for (pptr = hptr->h_aliases; *pptr != NULL; pptr++)
		LOGI("  alias:%s", *pptr);

	//IP类型
	switch (hptr->h_addrtype)
	{
	case AF_INET:
	case AF_INET6:
		{
			LOGI(" first address: %s",
				inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));

			pptr = hptr->h_addr_list; //IP地址列表 char*[]
			for (; *pptr != NULL; pptr++)
			{
				LOGI("  address:%s",
					inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
			}

			memcpy(&ret, hptr->h_addr, 4);

			break;
		}

	default:
		LOGW(" unknown address type");
		break;
	}

	return ntohl(ret);
}

void handlerMsg(PT_MSG msg)
{
	LOGD("I'm handler (%d, %d,%d,%d,%s)", pthread_self(), msg->what, msg->arg0, msg->arg1, (char *)msg->data);
}

void handlerMsg2(PT_MSG msg)
{
	LOGI("I'm handler (%d, %d,%d,%d,%s)", pthread_self(), msg->what, msg->arg0, msg->arg1, (char *)msg->data);
}

void * main_loop(void *args)
{
	while(1)
	{
		PT_MSG msg = (PT_MSG)dequeue(mQueue);

		switch(msg->what)
		{
		case 1:
			LOGD("hello(%d,%d)", msg->arg0, msg->arg1);
			break;

		case 2:
			LOGD("world!");
			break;

		case 3:
			LOGD("test mr_getHostByName_block %d", mr_getHostByName_block("9iso.com"));
			break;

		case 4:
			mHandler->sendEmptyMsg(1001);
			break;

		case 5:
		{

			break;
		}

		case 100:
			LOGD("timer out!");
			break;

		case -100:
			pthread_exit((void *)"I'm died...");
			break;
		}

		free(msg);
	}
}

void sendMsg(int what, int arg0, int arg1)
{
	PT_MSG msg = NEW_MSG;

	msg->what = what;
	msg->arg0 = arg0;
	msg->arg1 = arg1;
	msg->data = NULL;

	enqueue(mQueue, (Element) msg);
}

static void timer_sigroutine(int signo)
{
	switch (signo)
	{
	case SIGALRM:
		sendMsg(100, 23, 23);
		break;
	}
}

int timerStart(int t)
{
	struct itimerval tick = { 0 };

	/*当setitimer()所执行的timer时间到了会呼叫SIGALRM signal，
	 用signal()将要执行的 function 指定给SIGALRM。*/
	signal(SIGALRM, timer_sigroutine);

	// 设定第一次执行发出signal所延迟的时间
	tick.it_value.tv_sec = t / 1000;
	tick.it_value.tv_usec = t * 1000 % 1000000;

	tick.it_interval.tv_sec = 1; //重复

	// ITIMER_REAL，表示以real-time方式减少timer，在timeout时会送出SIGALRM signal
	if (setitimer(ITIMER_REAL, &tick, NULL) == -1)
	{
		LOGE("setitimer err! t=%d", t);
		return -1;
	}

	return 0;
}

/*停止定时器。*/
int timerStop(void)
{
	struct itimerval tick = {0};

	setitimer(ITIMER_REAL, &tick, NULL);

	return 0;
}

extern "C"
{
JNIEXPORT void JNICALL Java_com_yichou_test_jni_MainActivity_init(JNIEnv * env, jobject self)
{
	mHandler = new Handler(handlerMsg);

	mQueue = new_queue();

	pthread_create(&pid, NULL, main_loop, NULL);

//	timerStart(1000);
}

JNIEXPORT void JNICALL Java_com_yichou_test_jni_MainActivity_exit(JNIEnv * env, jobject self)
{
	char *msg;

	timerStop();
	pthread_join(pid, (void **)&msg);
	LOGD("pthread_join finish, msg=%s", msg);
	delete_queue(mQueue);

	mHandler->exit();
}

JNIEXPORT void JNICALL Java_com_yichou_test_jni_MainActivity_testHandler(JNIEnv * env, jobject self)
{
	static int flag=0;


	flag++;

	Handler h(flag%2==0? handlerMsg : handlerMsg2);
	int i;

	for (i = 0; i < 10; ++i)
	{
		LOGW("h=%#p, flag=%#p, i=%#p", &h, &flag, &i);

		h.sendMsg(i, 23 * i, 32 * i, flag%2==0? (void *) "AAAA" : (void *)"BBBB");
//		usleep(1000);
		sleep(1);
	}
	h.exit();
}

JNIEXPORT void JNICALL Java_com_yichou_test_jni_MainActivity_sendMsg(JNIEnv * env, jobject self,
		jint what, jint arg0, jint arg1)
{
	sendMsg(what, arg0, arg1);
}
}
