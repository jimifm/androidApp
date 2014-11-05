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
#include <linux/videodev2.h>
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

#define kClassPathName "org/monitor/display/CaptureImage"
void InitCAT6023() ;
BOOL CheckHDMIRX() ;
pthread_t pid = 0;
int pstatus = 0;

enum v4l2_field			field = V4L2_FIELD_NONE;
int videoWidth = 1920, videoHeight = 1080;

static JavaVM *g_jvm;
//static jobject g_obj;
static jclass g_obj;

BYTE FPGA_ReadI2C_Byte(int busfd, BYTE RegAddr)
{
	int ret = 0;
	struct i2c_rdwr_ioctl_data work_queue;
	//unsigned char data[2] = "";
	//data[0] = (RegAddr & 0xff00) >> 8;
	//data[1] = (RegAddr & 0x00ff);
	unsigned char data = 0;

	work_queue.nmsgs = 2;
	work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct
		i2c_msg));
	if (!work_queue.msgs) {
		printf("Memory alloc error\n");
		//close(i2cbus_fd);
		return -1;
	}

//	ioctl(i2cbus_fd, I2C_TIMEOUT, 2);
//	ioctl(i2cbus_fd, I2C_RETRIES, 1);

	(work_queue.msgs[0]).len = 1;
	(work_queue.msgs[0]).addr = 0x68;
	(work_queue.msgs[0]).flags = 0;
	(work_queue.msgs[0]).buf = &RegAddr;

	(work_queue.msgs[1]).len = 1;
	(work_queue.msgs[1]).flags = I2C_M_RD;
	(work_queue.msgs[1]).addr = 0x68;
	(work_queue.msgs[1]).buf = &data;

	ret = ioctl(i2cbus_fd, I2C_RDWR, (unsigned long) &work_queue);
	if (ret < 0)
	{
		printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
	//goto err;
		perror("[read] error:");
	}
	else{
	//	if(data == 0)
			printf("[read ] reg:%02x val:%02x\n", RegAddr, data);
	//	else
	//		printf("===[KEY]=====[val======  %02x\n",data);
	}
	free(work_queue.msgs);
 	return data;

}

SYS_STATUS FPGA_WriteI2C_Byte(int busfd, BYTE RegAddr,BYTE d)
{
	int ret = 0;
	struct i2c_rdwr_ioctl_data work_queue;
	BYTE msg[2] = "";
	work_queue.nmsgs = 1;
	work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct
		i2c_msg));
	if (!work_queue.msgs) {
		printf("Memory alloc error\n");
		//close(i2cbus_fd);
		return -1;
	}

	msg[0] = RegAddr;
	msg[1] = d;
//	ioctl(i2cbus_fd, I2C_TIMEOUT, 2);
//	ioctl(i2cbus_fd, I2C_RETRIES, 1);

	(work_queue.msgs[0]).len = 2;
	(work_queue.msgs[0]).flags = 0;
	(work_queue.msgs[0]).addr = 0x68;
	(work_queue.msgs[0]).buf = msg;

	ret = ioctl(i2cbus_fd, I2C_RDWR, (unsigned long) &work_queue);
	if (ret < 0)
	{
		printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
		//goto err;
	}
	else
		printf("[write] reg:%02x val:%02x\n", RegAddr, d);

	free(work_queue.msgs);
 	return ret;

}

void CheckFPGA()
{
	int width = 1920, height = 1080;
	enum v4l2_field	cfield;
	BYTE data = FPGA_ReadI2C_Byte(i2cbus_fd, 0xdd);
	if(data == 0x00)
		return;
	BYTE tmp = (data & 0x18)>>3;
	switch(tmp)
	{
	case 0x01:
		cfield = V4L2_FIELD_INTERLACED;
		break;
	case 0x02:
		cfield = V4L2_FIELD_NONE;
		break;
		//case 0x03:
	default:
		cfield = V4L2_FIELD_NONE;
	}

	tmp = data & 0x07;
	switch(tmp)
	{
	case 0x01:
		width = 640;
		height = 480;
		break;
	case 0x02:
		width = 1024;
		height = 576;
		break;
	case 0x03:
		width = 1280;
		height = 720;
		break;
	case 0x04:
		width = 1920;
		height = 1080;
		break;
	case 0x05:
		width = 2560;
		height = 1440;
		break;
	default:
		width = 1920;
		height = 1080;
	}

	if(width != videoWidth || cfield != field)
	{
		LOGI("CheckFPGA videoWidth = %d",width);
		LOGI("CheckFPGA videoHeight = %d",height);
		if(cfield != field)
		{
			LOGI("CheckFPGA filed changed %0x",tmp);
		}
		videoWidth = width;
		videoHeight = height;
		field = cfield;

		JNIEnv *env;
	    jclass cls;
	    jmethodID mid;

	    //Attach主线程
	    if((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK)
	    {
	        LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
	        return ;
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
		mid = (*env)->GetStaticMethodID(env, cls, "switVideoResetCb", "(II)V");

		if (mid == NULL)
		 {
			LOGE("GetMethodID() Error");

		    goto error;
		 }
	    //最后调用java中的静态方法
		(*env)->CallStaticVoidMethod(env, cls, mid , videoWidth, videoHeight);
		//hui diao
	}
	return;
error:
	  //Detach主线程
	if((*g_jvm)->DetachCurrentThread(g_jvm) != JNI_OK)
	{
	    LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
}

void * switvideo_device_running_fun(void* arg)
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
        CheckFPGA();
        delay1ms(200);
        if(pstatus == 0)
        {
            LOGI("pthread_exit\n");
            pthread_exit(NULL);
        }
	//sleep(1);

    }
}


int run_switvideo()
//int main( void )
{
	return pthread_create(&pid, NULL, &switvideo_device_running_fun, NULL);
}

void exit_switvideo()
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
void switvideo_1native_1init
  (JNIEnv *env, jclass thiz){

    (*env)->GetJavaVM(env,&g_jvm);

    g_obj = (*env)->NewGlobalRef(env,thiz);

    printf("%s\n" , VERSION_STRING) ;
	i2cbus_fd= open("/dev/i2c-0", O_RDWR);

	if (!i2cbus_fd) {
		printf("Error on opening the device file\n");
		return ;
	}
	InitCAT6023() ;
    //init_hdmirx();
}
/*
 * Class:     com_it6604_hdmirx_HdmirxActivity
 * Method:    hdmirx_native_grun
 * Signature: ()V
 */
void Java_switvideo_1native_1grun
  (JNIEnv *env, jclass thiz){
	run_switvideo();
}
/*
 * Class:     com_it6604_hdmirx_HdmirxActivity
 * Method:    hdmirx_native_exit
 * Signature: ()V
 */
void Java_switvideo_1native_1exit
  (JNIEnv *env, jclass thiz){
	exit_switvideo();
}
/*
 * Class:     org_monitor_display_CaptureImage
 * Method:    video_native_fieldset
 * Signature: ()V
 */
void Java_switvideo_1native_1fieldset
  (JNIEnv *env, jclass thiz){
	int vfd = open("/dev/video1",O_RDWR);
	if(vfd < 0)
		return;
    //frame format
    struct v4l2_format tv_fmt;

    memset(&tv_fmt, 0, sizeof(tv_fmt));
    tv_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(vfd, VIDIOC_G_FMT, &tv_fmt)< 0) {
        printf("VIDIOC_G_FMT\n");
        //exit(-1);
        goto err;
    }
    //set the form of camera capture data
    tv_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tv_fmt.fmt.pix.width = videoWidth;
    tv_fmt.fmt.pix.height = videoHeight;
    //tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    tv_fmt.fmt.pix.field = field;
    if (ioctl(vfd, VIDIOC_S_FMT, &tv_fmt)< 0) {
        printf("VIDIOC_S_FMT\n");
        //exit(-1);
        goto err;

    }
err:
    close(vfd);

}
#define __DEBUG__ 1
/*
 * Class:     org_monitor_display_CaptureImage
 * Method:    video_native_getsize
 * Signature: (Lorg/monitor/display/videoSize;)V
 */
jobject  Java_switvideo_1native_1getsize
  (JNIEnv *env, jclass thiz)
{

//#ifdef __DEBUG__
	//int busfd = open("/dev/i2c-0", O_RDWR);
	//if(busfd < 0)
	//	goto RETOBJ;
	BYTE data = FPGA_ReadI2C_Byte(i2cbus_fd, 0xdd);
	BYTE tmp = (data & 0x18)>>3;
	switch(tmp)
	{
	case 0x01:
		field = V4L2_FIELD_INTERLACED;
		break;
	case 0x02:
		field = V4L2_FIELD_NONE;
		break;
	//case 0x03:
	default:
		field = V4L2_FIELD_NONE;
	}
	tmp = data & 0x07;
	switch(tmp)
	{
	case 0x01:
		videoWidth = 640;
		videoHeight = 480;
		break;
	case 0x02:
		videoWidth = 1024;
		videoHeight = 576;
		break;
	case 0x03:
		videoWidth = 1280;
		videoHeight = 720;
		break;
	case 0x04:
		videoWidth = 1920;
		videoHeight = 1080;
		break;
	case 0x05:
		videoWidth = 2560;
		videoHeight = 1440;
		break;
	default:
		videoWidth = 1920;
		videoHeight = 1080;
	}
	LOGI("videoWidth = %d",videoWidth);
	LOGI("videoHeight = %d",videoHeight);
//#endif
//#ifndef __DEBUG__

	jclass videoSizeClass = (*env)->FindClass(env, "org/monitor/display/videoSize");
    //if(videoSizeClass){
	jmethodID v_mid   = (*env)->GetMethodID(env,videoSizeClass,"<init>","(II)V");
    jobject videoSize = (*env)->NewObject(env, videoSizeClass,v_mid,videoWidth,videoHeight);
       // jfieldID widthId = (*env)->GetFieldID(env, videoSizeClass, "videoWidth", "I");
        //(*env)->SetIntField(env, videoSize, widthId, videoWidth);
        //LOGI("videoWidth = %d",videoWidth);

        //jfieldID heightId = (*env)->GetFieldID(env, videoSizeClass, "videoWeight", "I");
        //(*env)->SetIntField(env, videoSize, heightId, videoHeight);
       // LOGI("videoHeight = %d",videoHeight);
    //}
//#endif
#ifdef __DEBUG__
	//close(busfd);
	return videoSize;
#endif
}
/*
 * Class:     org_monitor_display_CaptureImage
 * Method:    switvideo_native_setinterface
 * Signature: (I)I
 */
jint Java_switvideo_1native_1setinterface
  (JNIEnv *env, jclass thiz, jint videoInterface)
{
	return 0;
}
static JNINativeMethod gMethods[] = {
    {"switvideo_native_init",       "()V",           (void *)switvideo_1native_1init},
    {"switvideo_native_grun",       "()V",           (void *)Java_switvideo_1native_1grun},
    {"switvideo_native_exit",       "()V",           (void *)Java_switvideo_1native_1exit},
    {"switvideo_native_fieldset",       "()V",           (void *)Java_switvideo_1native_1fieldset},
    {"switvideo_native_getsize",       "()Lorg/monitor/display/videoSize;",           (void *)Java_switvideo_1native_1getsize},
    {"switvideo_native_setinterface",       "(I)I",           (void *)Java_switvideo_1native_1setinterface},
};

int register_com_it6604_hdmirx_HdmirxActivity(JNIEnv *env) {
	return jniRegisterNativeMethods(env, kClassPathName, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));

}
