LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

TMP_LOCAL_PATH := $(LOCAL_PATH)

include $(LOCAL_PATH)/BATestApp/Android.mk
include $(TMP_LOCAL_PATH)/certification_tools/Android.mk
include $(TMP_LOCAL_PATH)/system_bt_ext/btconfigstore/Android.mk

