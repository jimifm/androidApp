LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := fs
LOCAL_SRC_FILES := fs.c jni.c onLoad.cpp
LOCAL_LDLIBS  += -llog

include $(BUILD_SHARED_LIBRARY)
