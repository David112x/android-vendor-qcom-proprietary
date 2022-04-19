LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        radio_helium_hal.c \
        radio_helium_hal_cmds.c

LOCAL_SHARED_LIBRARIES := \
         libfm-hci \
         libdl \
         liblog \
         libnativehelper \
         libcutils

FM_HCI_DIR:= vendor/qcom/opensource/commonsys/fm

LOCAL_C_INCLUDES += $(FM_HCI_DIR)/fm_hci

LOCAL_MODULE := fm_helium
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
