LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)
LOCAL_CFLAGS += -Werror

LOCAL_C_INCLUDES := $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/
ifeq ($(OEM_CHROMATIX), true)
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camera-ext/mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
else
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_SRC_FILES:= imx258_lib.c
LOCAL_MODULE           := libmmcamera_imx258
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_SHARED_LIBRARY)
