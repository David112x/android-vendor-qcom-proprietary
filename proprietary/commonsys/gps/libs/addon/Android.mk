#/******************************************************************************
#*@file Android.mk
#* brief Rules to create an package which exposes the location api
#*******************************************************************************/
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= com.qti.location.sdk.xml

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

$(LOCAL_MODULE): com.qualcomm.location
	vendor/qcom/proprietary/commonsys/gps/libs/addon/scripts/mksdkjar.sh out/host/common/izatsdk >/dev/null 2>&1

include $(BUILD_PREBUILT)

include $(call all-makefiles-under,$(LOCAL_PATH))
