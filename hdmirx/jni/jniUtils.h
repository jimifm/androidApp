/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_it6604_hdmirx_HdmirxActivity */

#ifndef _Included_com_it6604_hdmirx_HdmirxActivity
#define _Included_com_it6604_hdmirx_HdmirxActivity
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_it6604_hdmirx_HdmirxActivity
 * Method:    hdmirx_native_init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_it6604_hdmirx_HdmirxActivity_hdmirx_1native_1init
  (JNIEnv *, jclass);

/*
 * Class:     com_it6604_hdmirx_HdmirxActivity
 * Method:    hdmirx_native_grun
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_it6604_hdmirx_HdmirxActivity_hdmirx_1native_1grun
  (JNIEnv *, jclass);

/*
 * Class:     com_it6604_hdmirx_HdmirxActivity
 * Method:    hdmirx_native_exit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_it6604_hdmirx_HdmirxActivity_hdmirx_1native_1exit
  (JNIEnv *, jclass);

int jniThrowException(JNIEnv* env, const char* className, const char* msg);

JNIEnv* getJNIEnv();

int jniRegisterNativeMethods(JNIEnv* env,
                             const char* className,
                             const JNINativeMethod* gMethods,
                             int numMethods);
#ifdef __cplusplus
}
#endif
#endif