#======================================================================
#makefile for libmmcamera2_mct.so from mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS  := -D_ANDROID_
LOCAL_CFLAGS  += -DMEMLEAK_FLAG
LOCAL_CFLAGS  += -DFDLEAK_FLAG
LOCAL_CFLAGS += -DMCT_STUCK_FLAG

ifeq ($(TARGET_KERNEL_VERSION), 4.9)
LOCAL_CFLAGS += -DUSE_4_9_DEFS
endif

ifeq ($(TARGET_KERNEL_VERSION), 4.14)
LOCAL_CFLAGS += -DUSE_4_9_DEFS
endif

LOCAL_MMCAMERA_PATH  := $(QC_PROP_ROOT)/mm-camera/mm-camera2

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/

#************* MCT headers ************#
LOCAL_C_INCLUDES += $(LOCAL_PATH)/bus/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/controller/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/event/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/object/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/port/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/stream/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/tools/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/debug/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/debug/fdleak
LOCAL_C_INCLUDES += $(LOCAL_PATH)/debug/memleak
LOCAL_C_INCLUDES += $(LOCAL_PATH)/supermct

#************* HAL headers ************#
LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/../../../../../../../hardware/qcom/camera/QCamera2/stack/common

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
endif

ifeq ($(TARGET_HAS_LOW_RAM),true)
    LOCAL_CFLAGS += -DCAMERA_FEATURE_LOW_RAM
endif

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_CFLAGS  += -Werror

LOCAL_SRC_DIR := $(LOCAL_PATH)
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_MODULE           := libmmcamera2_mct
include $(SDCLANG_COMMON_DEFS)

LOCAL_SHARED_LIBRARIES := libdl liblog libcutils libmmcamera_dbg
LOCAL_MODULE_TAGS      := optional

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)

endif # is-vendor-board-platform,QCOM
