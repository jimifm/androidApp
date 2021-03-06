/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_comm_commi2c_I2cActivity */

#ifndef _Included_com_comm_commi2c_I2cActivity
#define _Included_com_comm_commi2c_I2cActivity
#ifdef __cplusplus
extern "C" {
#endif
#undef com_comm_commi2c_I2cActivity_MODE_PRIVATE
#define com_comm_commi2c_I2cActivity_MODE_PRIVATE 0L
#undef com_comm_commi2c_I2cActivity_MODE_WORLD_READABLE
#define com_comm_commi2c_I2cActivity_MODE_WORLD_READABLE 1L
#undef com_comm_commi2c_I2cActivity_MODE_WORLD_WRITEABLE
#define com_comm_commi2c_I2cActivity_MODE_WORLD_WRITEABLE 2L
#undef com_comm_commi2c_I2cActivity_MODE_APPEND
#define com_comm_commi2c_I2cActivity_MODE_APPEND 32768L
#undef com_comm_commi2c_I2cActivity_MODE_MULTI_PROCESS
#define com_comm_commi2c_I2cActivity_MODE_MULTI_PROCESS 4L
#undef com_comm_commi2c_I2cActivity_MODE_ENABLE_WRITE_AHEAD_LOGGING
#define com_comm_commi2c_I2cActivity_MODE_ENABLE_WRITE_AHEAD_LOGGING 8L
#undef com_comm_commi2c_I2cActivity_BIND_AUTO_CREATE
#define com_comm_commi2c_I2cActivity_BIND_AUTO_CREATE 1L
#undef com_comm_commi2c_I2cActivity_BIND_DEBUG_UNBIND
#define com_comm_commi2c_I2cActivity_BIND_DEBUG_UNBIND 2L
#undef com_comm_commi2c_I2cActivity_BIND_NOT_FOREGROUND
#define com_comm_commi2c_I2cActivity_BIND_NOT_FOREGROUND 4L
#undef com_comm_commi2c_I2cActivity_BIND_ABOVE_CLIENT
#define com_comm_commi2c_I2cActivity_BIND_ABOVE_CLIENT 8L
#undef com_comm_commi2c_I2cActivity_BIND_ALLOW_OOM_MANAGEMENT
#define com_comm_commi2c_I2cActivity_BIND_ALLOW_OOM_MANAGEMENT 16L
#undef com_comm_commi2c_I2cActivity_BIND_WAIVE_PRIORITY
#define com_comm_commi2c_I2cActivity_BIND_WAIVE_PRIORITY 32L
#undef com_comm_commi2c_I2cActivity_BIND_IMPORTANT
#define com_comm_commi2c_I2cActivity_BIND_IMPORTANT 64L
#undef com_comm_commi2c_I2cActivity_BIND_ADJUST_WITH_ACTIVITY
#define com_comm_commi2c_I2cActivity_BIND_ADJUST_WITH_ACTIVITY 128L
#undef com_comm_commi2c_I2cActivity_CONTEXT_INCLUDE_CODE
#define com_comm_commi2c_I2cActivity_CONTEXT_INCLUDE_CODE 1L
#undef com_comm_commi2c_I2cActivity_CONTEXT_IGNORE_SECURITY
#define com_comm_commi2c_I2cActivity_CONTEXT_IGNORE_SECURITY 2L
#undef com_comm_commi2c_I2cActivity_CONTEXT_RESTRICTED
#define com_comm_commi2c_I2cActivity_CONTEXT_RESTRICTED 4L
#undef com_comm_commi2c_I2cActivity_RESULT_CANCELED
#define com_comm_commi2c_I2cActivity_RESULT_CANCELED 0L
#undef com_comm_commi2c_I2cActivity_RESULT_OK
#define com_comm_commi2c_I2cActivity_RESULT_OK -1L
#undef com_comm_commi2c_I2cActivity_RESULT_FIRST_USER
#define com_comm_commi2c_I2cActivity_RESULT_FIRST_USER 1L
#undef com_comm_commi2c_I2cActivity_DEFAULT_KEYS_DISABLE
#define com_comm_commi2c_I2cActivity_DEFAULT_KEYS_DISABLE 0L
#undef com_comm_commi2c_I2cActivity_DEFAULT_KEYS_DIALER
#define com_comm_commi2c_I2cActivity_DEFAULT_KEYS_DIALER 1L
#undef com_comm_commi2c_I2cActivity_DEFAULT_KEYS_SHORTCUT
#define com_comm_commi2c_I2cActivity_DEFAULT_KEYS_SHORTCUT 2L
#undef com_comm_commi2c_I2cActivity_DEFAULT_KEYS_SEARCH_LOCAL
#define com_comm_commi2c_I2cActivity_DEFAULT_KEYS_SEARCH_LOCAL 3L
#undef com_comm_commi2c_I2cActivity_DEFAULT_KEYS_SEARCH_GLOBAL
#define com_comm_commi2c_I2cActivity_DEFAULT_KEYS_SEARCH_GLOBAL 4L
/*
 * Class:     com_comm_commi2c_I2cActivity
 * Method:    open
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_comm_commi2c_I2cActivity_open
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_comm_commi2c_I2cActivity
 * Method:    read
 * Signature: (II[BI)I
 */
JNIEXPORT jint JNICALL Java_com_comm_commi2c_I2cActivity_read
  (JNIEnv *, jobject, jint, jint, jbyteArray, jint);

/*
 * Class:     com_comm_commi2c_I2cActivity
 * Method:    write
 * Signature: (III[II)I
 */
JNIEXPORT jint JNICALL Java_com_comm_commi2c_I2cActivity_write
  (JNIEnv *, jobject, jint, jint, jint, jintArray, jint);

/*
 * Class:     com_comm_commi2c_I2cActivity
 * Method:    close
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_comm_commi2c_I2cActivity_close
  (JNIEnv *, jobject, jint);

#ifdef __cplusplus
}
#endif
#endif
