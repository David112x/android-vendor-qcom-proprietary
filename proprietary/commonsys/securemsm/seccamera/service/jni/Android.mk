ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE        := libseccamservice
LOCAL_MODULE_TAGS    := optional

LOCAL_CFLAGS        := $(COMMON_CFLAGS) \
                        -fno-short-enums \
                        -g -fdiagnostics-show-option -Wno-format \
                        -Wno-missing-braces -Wno-missing-field-initializers \
                        -fpermissive -Wno-unused-parameter

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := false
LOCAL_PRODUCT_MODULE := true

SECUREMSM_SHIP_PATH :=  $(QC_PROP_ROOT)/securemsm

LOCAL_C_INCLUDES += \
  $(LOCAL_PATH) \
  $(LOCAL_PATH)/../../lib \
  $(SECUREMSM_SHIP_PATH)/QSEEComAPI \
  $(TARGET_OUT_HEADERS)/common/inc


LOCAL_SHARED_LIBRARIES := \
    libc \
    liblog \
    libandroid \
    libnativewindow \
    libseccam

LOCAL_SRC_FILES := \
    jni_if.cpp \
    jni_vendor_if.cpp

include $(BUILD_SHARED_LIBRARY)

endif # not BUILD_TINY_ANDROID
