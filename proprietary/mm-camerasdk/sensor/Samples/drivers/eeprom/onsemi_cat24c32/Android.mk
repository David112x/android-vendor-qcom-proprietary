ONSEMI_CAT24C32_LIBS_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(ONSEMI_CAT24C32_LIBS_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../includes

LOCAL_SRC_FILES:= onsemi_cat24c32.c
LOCAL_MODULE           := libmmcamera_onsemi_cat24c32_eeprom_s
LOCAL_SHARED_LIBRARIES := libcutils_Sample

include $(BUILD_SHARED_LIBRARY)
