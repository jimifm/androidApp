#include <string.h>

#include <stdio.h>

//#include <jni.h>
#include "com_jpf_myjni_MyJNI.h"
 

#include <android/log.h>

#define LOG_TAG "MYJNI"

 

#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

 

static char s_string[] = "My god, I did it!";

 

jstring
Java_com_jpf_myjni_MyJNI_stringFromJNI( JNIEnv* env,

                                        jobject thiz )

{

       LOGI("MyJNI is called!");

       return (*env)->NewStringUTF(env, s_string);

}
