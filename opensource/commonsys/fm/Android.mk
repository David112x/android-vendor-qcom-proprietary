LOCAL_PATH:= $(call my-dir)
LOCAL_DIR_PATH:= $(call my-dir)

include $(LOCAL_PATH)/jni/Android.mk
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/fmapp2/Android.mk

LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/fm_hci/Android.mk

LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/helium/Android.mk
