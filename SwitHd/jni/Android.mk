LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := switvideo
LOCAL_SRC_FILES := cat6023.c io.c mainmcu.c onLoad.cpp
LOCAL_LDLIBS += -llog
include $(BUILD_SHARED_LIBRARY)
