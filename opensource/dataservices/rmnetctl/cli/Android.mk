TARGET_DISABLE_RMNT := false
ifeq ($(TARGET_USES_QMAA),true)
ifneq ($(TARGET_USES_QMAA_OVERRIDE_DATA),true)
TARGET_DISABLE_RMNT := true
endif
endif
ifneq ($(TARGET_DISABLE_RMNT),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := rmnetcli.c
LOCAL_CFLAGS := -Wall -Werror

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../src
LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_MODULE := rmnetcli
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := librmnetctl

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin

include $(BUILD_EXECUTABLE)
endif
