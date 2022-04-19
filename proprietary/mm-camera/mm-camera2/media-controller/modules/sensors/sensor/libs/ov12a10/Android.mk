LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)
ifeq ($(call is-board-platform-in-list, msm8952),true)
  LOCAL_CFLAGS += -DMSM8952_SENSORS
endif
LOCAL_CFLAGS += -Werror

LOCAL_C_INCLUDES := $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_SRC_FILES:= ov12a10_lib.c
LOCAL_MODULE           := libmmcamera_ov12a10
LOCAL_SHARED_LIBRARIES := libcutils

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_SHARED_LIBRARY)
