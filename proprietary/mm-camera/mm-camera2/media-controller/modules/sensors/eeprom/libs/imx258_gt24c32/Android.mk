IMX258_GT24C32_EEPROM_LIBS_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#        Make the shared library (libmmcamera_imx258_gt24c32_eeprom.so)
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(IMX258_GT24C32_EEPROM_LIBS_PATH)
LOCAL_MODULE_TAGS := optional
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../mm-camerasdk/sensor/includes/
ifeq ($(OEM_CHROMATIX), true)
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camera-ext/mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
else
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
endif

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_SRC_FILES:= imx258_gt24c32_eeprom.c
LOCAL_MODULE           := libmmcamera_imx258_gt24c32_eeprom
LOCAL_SHARED_LIBRARIES := liblog libcutils libmmcamera_eeprom_util

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
