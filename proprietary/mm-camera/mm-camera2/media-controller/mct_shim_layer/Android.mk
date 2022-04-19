ifeq ($(call is-vendor-board-platform,QCOM),true)
#
# mct_shim_layer
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  -DMSM_CAMERA_BIONIC

LOCAL_CFLAGS  += -Werror
LOCAL_CFLAGS  += -D_ANDROID_
LOCAL_CFLAGS  += -DMEMLEAK_FLAG
LOCAL_CFLAGS  += -DFDLEAK_FLAG

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../
LOCAL_MMCAMERA2_PATH  := $(LOCAL_PATH)/../../../mm-camera2
LOCAL_CAMERAHAL_PATH := $(LOCAL_MMCAMERA_PATH)/../../../../hardware/qcom/camera

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/includes/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA_PATH)/services/thread/include
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/includes/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/stream/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/debug/
LOCAL_C_INCLUDES+= $(LOCAL_CAMERAHAL_PATH)/QCamera2/stack/common/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/debug/memleak/
LOCAL_C_INCLUDES+= $(LOCAL_MMCAMERA2_PATH)/media-controller/mct/debug/fdleak/

LOCAL_HEADER_LIBRARIES := libutils_headers

include $(LOCAL_PATH)/../../../local_additional_dependency.mk

LOCAL_SRC_FILES:= mct_shim_layer.c

LOCAL_SHARED_LIBRARIES:= libcutils libdl libmmcamera2_mct \
                                   libmmcamera2_stats_modules \
                                   libmmcamera2_iface_modules \
                                   libmmcamera2_isp_modules \
                                   libmmcamera2_sensor_modules \
                                   libmmcamera2_pproc_modules \
                                   libmmcamera2_imglib_modules \
                                   libmmcamera_thread_services \
                                   libmmcamera_dbg

LOCAL_MODULE:= libmmcamera2_mct_shimlayer
include $(SDCLANG_COMMON_DEFS)
LOCAL_MODULE_TAGS := optional

LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
endif
