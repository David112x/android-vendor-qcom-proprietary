#======================================================================
#makefile for libmmcamera2_iface_modules.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS :=  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/debug/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/debug/fdleak

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/iface2/hvx/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/hvx/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/hvx/lib/zzHDR/

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../system/core/libion/kernel-headers
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../ system/core/libion/include
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/../../../../../../../../hardware/qcom/camera/QCamera2/stack/common

LOCAL_CFLAGS  += -Werror

LOCAL_CFLAGS += -DCDS_FEATURE_ENABLE=1
LOCAL_SRC_DIR := $(LOCAL_PATH)
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_COPY_HEADERS_TO := mm-camera-interface
LOCAL_COPY_HEADERS := hvx/hvx_stub.h

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_MODULE           := libmmcamera2_iface_modules
include $(SDCLANG_COMMON_DEFS)

LOCAL_SHARED_LIBRARIES := liblog libcutils libmmcamera2_mct libmmcamera2_isp_modules \
                          libdl libmmcamera_dbg

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES += libion
endif
LOCAL_MODULE_TAGS      := optional

LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc

LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)

endif # is-vendor-board-platform,QCOM
