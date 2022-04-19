
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        radio_helium_hal.c \
        radio_helium_hal_cmds.c \
        ../fm_hci/fm_hci.cpp

LOCAL_SHARED_LIBRARIES := \
         libdl \
         liblog \
         libcutils


FM_HCI_DIR:= vendor/qcom/proprietary/fm/fmvendorlib

LOCAL_C_INCLUDES += $(FM_HCI_DIR)/fm_hci

LOCAL_MODULE := fm_helium
LOCAL_MODULE_SUFFIX:= .so

LOCAL_MODULE_PATH_32 := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_PATH_64 := $(TARGET_OUT_VENDOR)/lib64

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

