LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../includes/

LOCAL_SRC_FILES:= pmic_flash.c
LOCAL_MODULE           := libflash_pmic_s
LOCAL_SHARED_LIBRARIES := libcutils_Sample

include $(BUILD_SHARED_LIBRARY)

