#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>
#include <pthread.h>
#include <linux/input.h>
#define LOG_TAG "i2cIput"

#include <android/log.h>
#include "jniUtils.h"

static JavaVM *g_jvm;
//static jobject g_obj;
static jclass g_obj;
static count ;
//#include "JNIHelp.h"
//#include "android_runtime/AndroidRuntime.h"
//#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
//#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
//#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__)
#define LOGI(...) \
	((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) \
	((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) \
	((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGD(...) \
	((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

//#define __USE_PTHREAD__

static const char* const kClassPathName = "com/imput/i2cinputsystem/I2CInput";
int i2c_input_readerloop(JNIEnv *env, jmethodID mid)
{
	struct pollfd plfd;
	int fd = open("/dev/input/event0", O_RDONLY);
	if(0 > fd)
	{
		LOGE("FindClass() Error.....");
	 	goto error;
	}
	/*while(1){
		//memset(&plfd, 0, sizeof(plfd));
		plfd.fd = fd;
		plfd.events = POLLIN;
		plfd.revents = 0;

		int ret = poll(&plfd, fd + 1, -1);

		if(ret > 0 && (plfd.revents & POLLIN))
		{
			// todo callback java layout code
			 jstring param = (*env)->NewStringUTF(env,"huazi!");
			 (*env)->CallStaticVoidMethod(env, g_obj, mid , param, 6);

	        break;
		}
	}*/
	close(fd);
	 jstring param = (*env)->NewStringUTF(env,"huazi!");
	 (*env)->CallStaticVoidMethod(env, g_obj, mid , param, 6);
error:
	 return -1;
}

void *funProc(void* arg)
{
    JNIEnv *env;
    jclass cls;
    jmethodID mid;

    sleep(3);
    //Attach主线程
    if((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK)
    {
        LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
        return NULL;
    }
    //找到对应的类
    cls = g_obj;
    //cls = (*env)->GetObjectClass(env,g_obj);
    //cls = (*env)->FindClass(env, "com/imput/i2cinputsystem/I2CInput");
    if(cls == NULL)
    {
       LOGE("FindClass() Error.....");
        goto error;
    }
    //再获得类中的方法
	mid = (*env)->GetStaticMethodID(env, cls, "JNICBProc", /*"([BI)V"*/"(Ljava/lang/String;I)V");
	//mid = (*env)->GetStaticMethodID(env, cls, "JNICBProc", "()V");
	if (mid == NULL)
	 {
		LOGE("GetMethodID() Error");

	    goto error;
	 }
    //最后调用java中的静态方法
	jstring param = (*env)->NewStringUTF(env,"android ndk test");
	(*env)->CallStaticVoidMethod(env, cls, mid , param, 6);
	 //i2c_input_readerloop(env, mid);
	struct pollfd plfd;
	int fd = open("/dev/input/event0", O_RDONLY);
	if(0 > fd)
	{
		LOGE("open() Error.....");
	 	goto error;
	}
	int ret;
	char buf[2048]="";
	//jstring zparam = (*env)->NewStringUTF(env,"huanhuan!");

	while(1){
		//memset(&plfd, 0, sizeof(plfd));
		plfd.fd = fd;
		plfd.events = POLLIN;
		plfd.revents = 0;

		ret = poll(&plfd, fd + 1, -1);

		if(ret > 0 && (plfd.revents & POLLIN))
		{
			// todo callback java layout code
			//mid = (*env)->GetStaticMethodID(env, cls, "JNICBProc", /*"([BI)V"*/"(Ljava/lang/String;I)V");

			//(*env)->CallStaticVoidMethod(env, g_obj, mid , zparam, 6);
			read(fd, buf, 2048);
			LOGD("@poll read ok.....");
	        //break;
		}
		else
		{
			//LOGE("poll() Error.....");
		}
	}
	close(fd);

error:
    //Detach主线程
    if((*g_jvm)->DetachCurrentThread(g_jvm) != JNI_OK)
    {
       LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
    }


    pthread_exit(0);
}
void *CBProc(void* arg)
{
    JNIEnv *env;
    jclass cls;
    jmethodID mid;

   // sleep(3);
    //Attach主线程
    if((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK)
    {
        LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
        return NULL;
    }
    //找到对应的类
    cls = g_obj;
    //cls = (*env)->GetObjectClass(env,g_obj);
    //cls = (*env)->FindClass(env, "com/imput/i2cinputsystem/I2CInput");
    if(cls == NULL)
    {
       LOGE("FindClass() Error.....");
        goto error;
    }
    //再获得类中的方法
	mid = (*env)->GetStaticMethodID(env, cls, "JNICBProc", /*"([BI)V"*/"(Ljava/lang/String;I)V");
	//mid = (*env)->GetStaticMethodID(env, cls, "JNICBProc", "()V");
	if (mid == NULL)
	 {
		LOGE("GetMethodID() Error");

	    goto error;
	 }
	jstring zparam = (*env)->NewStringUTF(env,"huanhuan!");
	(*env)->CallStaticVoidMethod(env, g_obj, mid , zparam, count);

error:
    //Detach主线程
    if((*g_jvm)->DetachCurrentThread(g_jvm) != JNI_OK)
    {
       LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
    }


    pthread_exit(0);
}

void Java_com_imput_i2cinputsystem_I2CInput_native_1init(JNIEnv *env, /*jobject thiz*/jclass thiz)
{
	//保存全局JVM以便在子线程中使用
    (*env)->GetJavaVM(env,&g_jvm);
    //不能直接赋值(g_obj = obj)
    g_obj = (*env)->NewGlobalRef(env,thiz);

}

void startI2Cprocess(JNIEnv *env,/*jobject thiz*/ jclass thiz)
{
#ifndef __USE_PTHREAD__
	jclass cls = thiz;
	jmethodID mid;
//	char cbbuf[128];
	//找到对应的类
	//strcpy(cbbuf,"call back java function");
	//cls = (*env)->GetObjectClass(env,NULL);
	//cls = (*env)->GetObjectClass(env,thiz);
	//cls = (*env)->FindClass(env, "com/imput/i2cinputsystem/I2CInput");
	if(cls == NULL){
		LOGE("FindClass() Error.....");
		goto error;
	}
	//再获得类中的方法

	mid = (*env)->GetStaticMethodID(env, cls, "JNICBProc", /*"([BI)V"*/"(Ljava/lang/String;I)V");
	//mid = (*env)->GetStaticMethodID(env, cls, "JNICBProc", "()V");
	if (mid == NULL)
	 {
	    LOGE("GetMethodID() Error.....");
	    goto error;
	 }
#endif
#ifdef __USE_PTHREAD__
	pthread_t pid;
	pthread_create(&pid ,NULL, &funProc,NULL);
#endif

#ifndef __USE_PTHREAD__

	struct pollfd plfd;
	int fd = open("/dev/input/event0", O_RDONLY);
	if(0 > fd)
	{
		LOGE("open() Error.....");
	 	goto error;
	}
	int ret;
	struct input_event input_ev[3];
	memset(input_ev,0,sizeof(input_ev)*3);

	//char buf[2048]="";
	jstring zparam = (*env)->NewStringUTF(env,"huanhuan!");
	while(1){
		//memset(&plfd, 0, sizeof(plfd));
		plfd.fd = fd;
		plfd.events = POLLIN;
		plfd.revents = 0;

		ret = poll(&plfd, fd + 1, -1);

		if(ret > 0 && (plfd.revents & POLLIN))
		{
			// todo callback java layout code
			read(fd, (void *)input_ev, sizeof(input_ev)*3);
			LOGD("@poll read ok->>>\n"
					"[type0]%d, [code0]%d, [value0] %d,\n"
					"[type1]%d, [code1]%d, [value1] %d,\n"
					"[type2]%d, [code2]%d, [value2] %d,\n"
					"<<<-",
					input_ev[0].type, input_ev[0].code ,input_ev[0].value,
					input_ev[1].type, input_ev[1].code ,input_ev[1].value,
					input_ev[2].type, input_ev[2].code ,input_ev[2].value);
			//pthread_t pid;
			//pthread_create(&pid ,NULL, &CBProc,NULL);
			//count++;
			(*env)->CallStaticVoidMethod(env, cls, mid , zparam, count++);
			//break;
		}
		else
		{
				//LOGE("poll() Error.....");
		}
	}
	close(fd);

	 //最后调用java中的静态方法
	// jstring param = (*env)->NewStringUTF(env,"huazi");
	// (*env)->CallStaticVoidMethod(env, cls, mid , param, 6);
	//i2c_input_readerloop(env, cls, mid);
#endif
	// 	 	 while(1);
error:
	return ;
}

static JNINativeMethod gMethods[] = {
    {"native_init",       "()V",           (void *)Java_com_imput_i2cinputsystem_I2CInput_native_1init},
    {"start_i2c_prc",       "()V",           (void *)startI2Cprocess},
};

int register_com_imput_i2cinputsystem_I2CInput(JNIEnv *env) {
	return jniRegisterNativeMethods(env, kClassPathName, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));

}
