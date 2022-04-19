BRCE064GWZ_LIBS_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#        Make the shared library (libmmcamera_onsemi_cat24c32_eeprom.so)
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(BRCE064GWZ_LIBS_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../mm-camerasdk/sensor/includes/
ifeq ($(OEM_CHROMATIX), true)
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camera-ext/mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
else
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
endif

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_SRC_FILES:= rohm_brce064gwz.c
LOCAL_MODULE           := libmmcamera_rohm_brce064gwz_eeprom
LOCAL_SHARED_LIBRARIES := libcutils libmmcamera_eeprom_util liblog

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
