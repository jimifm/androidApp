LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := i2c_input
LOCAL_SRC_FILES := i2cInput.c onLoad.cpp
LOCAL_LDLIBS  += -llog

include $(BUILD_SHARED_LIBRARY)
