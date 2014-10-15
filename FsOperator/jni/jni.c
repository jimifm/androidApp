//jni.c
#define TAG "fs_jni"

#include <android/log.h>
#include "jniUtils.h"



static const char* const kClassPathName = "com/FileSystem/fsoperator/FsActivity";


jint
Java_com_FileSystem_fsoperator_FsActivity_NativeFileOpen( JNIEnv* env, jobject thiz,jstring filename,jint flags ){


	 const char *filename_char = (*env)->GetStringUTFChars(env,filename, NULL);

	return file_open(filename_char, flags);
}
jint
Java_com_FileSystem_fsoperator_FsActivity_NativeFileRead(JNIEnv* env, jobject thiz,int fd,jbyteArray buf,jint size){

	unsigned char *buf_char = (char*)((*env)->GetByteArrayElements(env,buf, NULL));

	return file_read(fd, buf_char,  size);
}

jint
Java_com_FileSystem_fsoperator_FsActivity_NativeFileWrite(JNIEnv* env, jobject thiz,int fd,jbyteArray buf,jint size){

	unsigned char *buf_char = (char*)((*env)->GetByteArrayElements(env,buf, NULL));

	return file_write(fd, buf_char,  size);
}

jlong
Java_com_FileSystem_fsoperator_FsActivity_NativeFileSeek(JNIEnv* env, jobject thiz,int fd,jlong Offset,jint whence){

	return file_seek(fd, Offset,  whence);
}

jint
Java_com_FileSystem_fsoperator_FsActivity_NativeFileClose(JNIEnv* env, jobject thiz,int fd){

	return file_close(fd);
}

/******************************JNI registration.************************************/
static JNINativeMethod gMethods[] = {
    {"NativeFileOpen",       "(Ljava/lang/String;I)I",           (void *)Java_com_FileSystem_fsoperator_FsActivity_NativeFileOpen},
    {"NativeFileRead",   	 "(I[BI)I",       					 (void *)Java_com_FileSystem_fsoperator_FsActivity_NativeFileRead},
    {"NativeFileWrite",      "(I[BI)I",                          (void *)Java_com_FileSystem_fsoperator_FsActivity_NativeFileWrite},
    {"NativeFileSeek",       "(IJI)J",                           (void *)Java_com_FileSystem_fsoperator_FsActivity_NativeFileSeek},
    {"NativeFileClose",      "(I)I",                             (void *)Java_com_FileSystem_fsoperator_FsActivity_NativeFileClose},
};

int register_com_FileSystem_fsoperator_FsActivity(JNIEnv *env) {
	return jniRegisterNativeMethods(env, kClassPathName, gMethods, sizeof(gMethods) / sizeof(gMethods[0]));

}


