LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := comm_i2c
LOCAL_SRC_FILES := com_comm_commi2c_I2cActivity.h
include $(BUILD_SHARED_LIBRARY)
