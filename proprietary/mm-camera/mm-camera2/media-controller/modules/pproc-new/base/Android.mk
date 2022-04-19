# ---------------------------------------------------------------------------
#                  Make the shared library (mmcamera_ppbase_module)
# ---------------------------------------------------------------------------

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

PPROC_MODULE_PATH := $(LOCAL_PATH)/../../pproc-new
MM_CAMERA_PATH := $(LOCAL_PATH)/../../../../../mm-camera2
HAL_PATH := $(LOCAL_PATH)/../../../../../../../../../hardware/qcom/camera

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(PPROC_MODULE_PATH)
LOCAL_C_INCLUDES += $(PPROC_MODULE_PATH)/buf_mgr
LOCAL_C_INCLUDES += $(PPROC_MODULE_PATH)/utils
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/includes
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/server-tuning/tuning/
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/includes
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/tools
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/debug
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/debug/fdleak
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/port
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/object
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/event
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/bus
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/module
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/stream
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/pipeline
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/modules/includes
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(HAL_PATH)/QCamera2/stack/common
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
                $(mmcamera_debug_defines) \
                $(mmcamera_debug_cflags)

LOCAL_CFLAGS  += -Werror

LOCAL_SRC_FILES := base_module.c base_port.c base_module_util.c


LOCAL_MODULE           := libmmcamera_ppbase_module
LOCAL_SHARED_LIBRARIES := libcutils libmmcamera2_mct libmmcamera_dbg
LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE_TAGS      := optional
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true


ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
