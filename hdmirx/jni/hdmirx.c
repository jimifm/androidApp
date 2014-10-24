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
#define LOG_TAG "hdmirxStub"

#include <hardware/hardware.h>
#include <hardware/hdmirx.h>
#include <android/log.h>
#include <pthread.h>
#define LOGI(...) \
        ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) \
        ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) \
        ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOGD(...) \
        ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))

#define MODULE_NAME "hdmirx"
#define MODULE_AUTHOR   "zhanghong"
#define DEVICE_NAME "/dev/i2c-0"
pthread_t pid = 0;
int pstatus = 0;
/*设备打开和关闭接口*/
static int hdmirx_device_open(const struct hw_module_t* module, const char* name, struct hw_device_t** device);
static int hdmirx_device_close(struct hw_device_t* device);

/*设备访问接口*/
static int hdmirx_device_init(struct hdmirx_device_t* dev);
static int hdmirx_device_run(struct hdmirx_device_t* dev);

static int hdmirx_device_stop(struct hdmirx_device_t* dev);
/*模块方法表*/
static struct hw_module_methods_t hdmirx_module_methods = {
        open: hdmirx_device_open
};

/*模块实例变量*/
struct hdmirx_module_t HAL_MODULE_INFO_SYM = {
        common: {
                tag: HARDWARE_MODULE_TAG,
                version_major: 1,
                version_minor: 0,
                id: HDMIRX_HARDWARE_MODULE_ID,
                name: MODULE_NAME,
                author: MODULE_AUTHOR,
                methods: &hdmirx_module_methods,
        }
};
static int hdmirx_device_open(const struct hw_module_t* module, const char* name, struct hw_device_t** device) {
    struct hdmirx_device_t* dev = (struct hdmirx_device_t*)malloc(sizeof(struct hdmirx_device_t));
            
    if(!dev) {
        LOGE("hdmirx Stub: failed to alloc space");
        return -EFAULT;
    }
    memset(dev, 0, sizeof(struct hdmirx_device_t));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (hw_module_t*)module;
    dev->common.close = hdmirx_device_close;
    dev->init_hdmirx = hdmirx_device_init;
    dev->run_hdmirx = hdmirx_device_run;
    dev->stop_hdmirx = hdmirx_device_stop;

    if((dev->fd = open(DEVICE_NAME, O_RDWR)) == -1) {
        LOGE("hdmirx Stub: failed to open /dev/i2c-0 -- %s.", strerror(errno));free(dev);
        return -EFAULT;
    }
    i2cbus_fd = dev->fd;
    *device = &(dev->common);
    LOGI("hdmirx Stub: open /dev/i2c-0 successfully.");

    return 0;
}

static int hdmirx_device_close(struct hw_device_t* device){

    struct hdmirx_device_t *hdmirx_device = (struct hdmirx_device_t*)device;
    if(hdmirx_device){
        close(hdmirx_device->fd);
        free(hdmirx_device);
    }
    i2cbus_fd = 0;
    return 0;
}

void InitCAT6023() ;
BOOL CheckHDMIRX() ;

static int hdmirx_device_init(struct hdmirx_device_t* dev){
    
    InitCAT6023();
    return 0;
}

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

static int hdmirx_device_run(struct hdmirx_device_t* dev){

    return pthread_create(&pid, NULL, &hdmirx_device_running_fun, NULL);
    //return 0;
}
static int hdmirx_device_stop(struct hdmirx_device_t* dev){

    pstatus = 0;
    pthread_join(pid, NULL);
   return 0; 
}
#if 0
int open_hdmirx()
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
}

void close_hdmirx()
{
    close(i2cbus_fd);
}
#endif
