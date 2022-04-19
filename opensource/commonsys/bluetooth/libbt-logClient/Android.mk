LOCAL_PATH := $(call my-dir)

ifeq ($(BOARD_HAVE_BLUETOOTH_QCOM),true)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
        src/bt_vendor_logc.c

LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/include \
        vendor/qcom/opensource/commonsys/system/bt/include

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        liblog

LOCAL_MODULE := libbt-logClient
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := qcom

include $(BUILD_SHARED_LIBRARY)

endif # BOARD_HAVE_BLUETOOTH_QCOM
