ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \

LOCAL_CFLAGS  += -D_ANDROID -Werror
LOCAL_CPPFLAGS  += -D_ANDROID_ -std=c++11 -Werror
ifeq ($(BOARD_QTI_CAMERA_V2),true)
LOCAL_CPPFLAGS  += -Wno-error=literal-suffix
endif

LOCAL_SRC_FILES:= eztune_lookup.c        \
                  eztune_items_diag.c    \
                  eztune_items_$(CHROMATIX_VERSION).c

LOCAL_HAL_PATH := $(TOP)/hardware/qcom/camera
MM_CAMERA_PATH := $(QC_PROP_ROOT)/mm-camera/mm-camera2

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_C_INCLUDES:= $(LOCAL_PATH)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../includes/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../tuning/

LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/modules/stats/chromatix-wrapper/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/includes/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/includes/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/stream/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/debug/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/mct/debug/fdleak
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/pproc-new/eztune
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/sensors/module/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/sensors/actuators/module/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/stats/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/stats/asd/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/stats/q3a/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/stats/q3a/aec/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/stats/q3a/awb/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/stats/q3a/af/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/stats/q3a/include/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/stats/q3a/platform/linux/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/../../mm-camerasdk/sensor/includes/
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES+= $(LOCAL_HAL_PATH)/QCamera2/stack/common/

LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libcutils
USE_FASTCV_OPT ?= true
ifeq ($(USE_FASTCV_OPT),true)
LOCAL_SHARED_LIBRARIES += libfastcvopt
endif

LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

LOCAL_MODULE_TAGS := optional

# Build tuning library
LOCAL_MODULE:= libmmcamera_tuning_lookup
include $(SDCLANG_COMMON_DEFS)
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)

endif
