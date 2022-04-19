BD7710_FLASH_LIBS_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(BD7710_FLASH_LIBS_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../includes/

LOCAL_SRC_FILES:= bd7710.c
LOCAL_MODULE           := libflash_bd7710_Sample
LOCAL_SHARED_LIBRARIES := libcutils_Sample

include $(BUILD_SHARED_LIBRARY)
