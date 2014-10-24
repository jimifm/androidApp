///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <mainmcu.c>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2010/08/10
//   @fileversion: CAT6023_SRC_1.15
//******************************************/

#include "cat6023.h"
#include "io.h"
#include "version.h"
#include <android/log.h>
#include <pthread.h>
#include "jniUtils.h"

#define LOG_TAG "hdmirxStub"
#define LOGI(...) \
	((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) \
	((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) \
	((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGD(...) \
	((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

#define kClassPathName "com/it6604/hdmirx/HdmirxActivity"
void InitCAT6023() ;
BOOL CheckHDMIRX() ;
pthread_t pid = 0;
int pstatus = 0;
static JavaVM *g_jvm;
//static jobject g_obj;
static jclass g_obj;

void * hdmirx_device_running_fun(void* arg)
{

    static BOOL bSignal ;
    BOOL bOldSignal, bChangeMode ;
   pstatus = 1;
    while(1) {

		bOldSignal = bSignal ;
		bSignal = CheckHDMIRX() ;
		bChangeMode = ( bSignal != bOldSignal ) ;

        if( bChangeMode )
        {

	        if( bSignal )
	        {

		        printf("getCAT6023HorzTotal() = %d\n",getCAT6023HorzTotal()) ;
		        printf("getCAT6023HorzActive() = %d\n",getCAT6023HorzActive()) ;
		        printf("getCAT6023HorzFrontPorch() = %d\n",getCAT6023HorzFrontPorch()) ;
		        printf("getCAT6023HorzSyncWidth() = %d\n",getCAT6023HorzSyncWidth()) ;
		        printf("getCAT6023HorzBackPorch() = %d\n",getCAT6023HorzBackPorch()) ;
		        printf("getCAT6023VertTotal() = %d\n",getCAT6023VertTotal()) ;
		        printf("getCAT6023VertActive() = %d\n",getCAT6023VertActive()) ;
		        printf("getCAT6023VertFrontPorch() = %d\n",getCAT6023VertFrontPorch()) ;
		        printf("getCAT6023VertSyncToDE() = %d\n",getCAT6023VertSyncToDE()) ;
		        printf("getCAT6023VertSyncBackPorch() = %d\n",getCAT6023VertSyncBackPorch()) ;
		        printf("getCAT6023VertSyncWidth() = %d\n",getCAT6023VertSyncWidth()) ;

	        }
	        else
	        {
	        }

			bChangeMode = FALSE ; // clear bChange Mode action
        }
        else
        {
	        if( bSignal )
	        {
	        }
	        else
	        {
	        }
        }
        delay1ms(100);
        if(pstatus == 0)
        {
            LOGI("pthread_exit\n");
            pthread_exit(NULL);
        }
	//sleep(1);

    }
}


int init_hdmirx()
//int main( void )
{
    static BOOL bSignal ;
    BOOL bOldSignal, bChangeMode ;

	printf("%s\n" , VERSION_STRING) ;


	i2cbus_fd= open("/dev/i2c-0", O_RDWR);

	if (!i2cbus_fd) {
		printf("Error on opening the device file\n");
		return -1;
	}
	InitCAT6023() ;
	return pthread_create(&pid, NULL, &hdmirx_device_running_fun, NULL);
#if 0
	InitCAT6023() ;

    while(1) {

	    {

		    bOldSignal = bSignal ;
		    bSignal = CheckHDMIRX() ;
		    bChangeMode = ( bSignal != bOldSignal ) ;
	    }


        if( bChangeMode )
        {

	        if( bSignal )
	        {

		        printf("getCAT6023HorzTotal() = %d\n",getCAT6023HorzTotal()) ;
		        printf("getCAT6023HorzActive() = %d\n",getCAT6023HorzActive()) ;
		        printf("getCAT6023HorzFrontPorch() = %d\n",getCAT6023HorzFrontPorch()) ;
		        printf("getCAT6023HorzSyncWidth() = %d\n",getCAT6023HorzSyncWidth()) ;
		        printf("getCAT6023HorzBackPorch() = %d\n",getCAT6023HorzBackPorch()) ;
		        printf("getCAT6023VertTotal() = %d\n",getCAT6023VertTotal()) ;
		        printf("getCAT6023VertActive() = %d\n",getCAT6023VertActive()) ;
		        printf("getCAT6023VertFrontPorch() = %d\n",getCAT6023VertFrontPorch()) ;
		        printf("getCAT6023VertSyncToDE() = %d\n",getCAT6023VertSyncToDE()) ;
		        printf("getCAT6023VertSyncBackPorch() = %d\n",getCAT6023VertSyncBackPorch()) ;
		        printf("getCAT6023VertSyncWidth() = %d\n",getCAT6023VertSyncWidth()) ;


	        }
	        else
	        {
	        }

			bChangeMode = FALSE ; // clear bChange Mode action
        }
        else
        {
	        if( bSignal )
	        {
	        }
	        else
	        {
	        }
        }

	sleep(1);
    }

	close(i2cbus_fd);
	return 0;
#endif
}

void exit_hdmirx()
{
    pstatus = 0;
    pthread_join(pid, NULL);
    close(i2cbus_fd);
}

/*
 * Class:     com_it6604_hdmirx_HdmirxActivity
 * Method:    hdmirx_native_init
 * Signature: ()V
 */
void Java_com_it6604_hdmirx_HdmirxActivity_hdmirx_1native_1init
  (JNIEnv *env, jclass thiz){
	//保存全局JVM以便在子线程中使用
    (*env)->GetJavaVM(env,&g_jvm);
    //不能直接赋值(g_obj = obj)
    g_obj = (*env)->NewGlobalRef(env,thiz);
    //init_hdmirx();
}
/*
 * Class:     com_it6604_hdmirx_HdmirxActivity
 * Method:    hdmirx_native_grun
 * Signature: ()V
 */
void Java_com_it6604_hdmirx_HdmirxActivity_hdmirx_1native_1grun
  (JNIEnv *env, jclass thiz){
	init_hdmirx();
}
/*
 * Class:     com_it6604_hdmirx_HdmirxActivity
 * Method:    hdmirx_native_exit
 * Signature: ()V
 */
void Java_com_it6604_hdmirx_HdmirxActivity_hdmirx_1native_1exit
  (JNIEnv *env, jclass thiz){
	exit_hdmirx();
}

static JNINativeMethod gMethods[] = {
    {"hdmirx_native_init",       "()V",           (void *)Java_com_it6604_hdmirx_HdmirxActivity_hdmirx_1native_1init},
    {"hdmirx_native_grun",       "()V",           (void *)Java_com_it6604_hdmirx_HdmirxActivity_hdmirx_1native_1grun},
    {"hdmirx_native_exit",       "()V",           (void *)Java_com_it6604_hdmirx_HdmirxActivity_hdmirx_1native_1exit},
};

int register_com_it6604_hdmirx_HdmirxActivity(JNIEnv *env) {
	return jniRegisterNativeMethods(env, kClassPathName, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));

}
