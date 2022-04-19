# ---------------------------------------------------------------------------
#                  Make the shared library (mmcamera_vpu_module)
# ---------------------------------------------------------------------------

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

PPROC_MODULE_PATH := $(LOCAL_PATH)/../../pproc-new
MM_CAMERA_PATH := $(LOCAL_PATH)/../../../../../mm-camera2

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(PPROC_MODULE_PATH)
LOCAL_C_INCLUDES += $(PPROC_MODULE_PATH)/buf_mgr
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/includes
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/server-tuning/tuning/
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/includes
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/tools
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/port
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/object
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/event
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/bus
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/module
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/stream
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/pipeline
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/debug
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/debug/fdleak
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/modules/includes
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
                $(mmcamera_debug_defines) \
                $(mmcamera_debug_cflags)


LOCAL_CFLAGS  += -Werror

LOCAL_SRC_FILES := vpu_client.c vpu_module.c vpu_port.c \
                   vpu_module_events.c vpu_module_util.c \
                   vpu_client_tuning.c

LOCAL_MODULE           := libmmcamera_vpu_module
LOCAL_SHARED_LIBRARIES := libcutils libmmcamera2_mct libmmcamera2_pp_buf_mgr libmmcamera_dbg

LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif
LOCAL_MODULE_TAGS      := optional eng
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif
include $(BUILD_SHARED_LIBRARY)

# vpu test app
include $(LOCAL_PATH)/test/Android.mk
