LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := TestJNI

LOCAL_SRC_FILES := TestJNI.cpp \
	msgqueue.c \
	handler.cpp

LOCAL_LDLIBS := -lm -llog -lc -lz

include $(BUILD_SHARED_LIBRARY)
