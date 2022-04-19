M24C64S_EEPROM_LIBS_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH := $(M24C64S_EEPROM_LIBS_PATH)
LOCAL_MODULE_TAGS := optional
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../mm-camerasdk/sensor/includes/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_SRC_FILES        := m24c64s_eeprom.c
LOCAL_MODULE           := libmmcamera_m24c64s_eeprom
include $(SDCLANG_COMMON_DEFS)
LOCAL_SHARED_LIBRARIES := liblog libcutils libmmcamera_eeprom_util

LOCAL_MODULE_OWNER       := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
