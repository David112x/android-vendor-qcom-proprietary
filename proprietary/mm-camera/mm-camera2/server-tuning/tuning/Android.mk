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
LOCAL_CPPFLAGS  += -Wno-literal-suffix
endif

ifeq ($(filter OMR1 O 8.1.0, $(PLATFORM_VERSION)), )
  LOCAL_CFLAGS += -DANDROID_COMPILATION_FIX
endif
ifeq ($(CHROMATIX_VERSION), 0306)
  LOCAL_CFLAGS += -DCHROMATIX_306
endif
ifeq ($(CHROMATIX_VERSION), 0307)
  LOCAL_CFLAGS += -DCHROMATIX_307
endif
ifeq ($(CHROMATIX_VERSION), 0308)
  LOCAL_CFLAGS += -DCHROMATIX_308
endif
ifeq ($(CHROMATIX_VERSION), 0309)
  LOCAL_CFLAGS += -DCHROMATIX_309
endif
ifeq ($(CHROMATIX_VERSION), 0310)
  LOCAL_CFLAGS += -DCHROMATIX_310
endif

ifeq  ($(call is-board-platform-in-list,msm8996 sdm660 msm8998 apq8098_latv $(TRINKET)),true)
  LOCAL_CFLAGS += -DCAMERA_USE_CHROMATIX_HW_WNR_TYPE
endif

USE_FASTCV_OPT ?= true
ifeq ($(USE_FASTCV_OPT),true)
 LOCAL_CFLAGS += -DUSE_FASTCV_OPT
endif

LOCAL_SRC_FILES:= eztune_interface.cpp   \
                  eztune_protocol.cpp    \
                  eztune_process.cpp     \
                  eztune_cam_adapter.cpp \
                  mmcam_log_utils.cpp    \
                  mmcam_socket_utils.cpp

LOCAL_HAL_PATH := $(SRC_CAMERA_HAL_DIR)
MM_CAMERA_PATH := $(QC_PROP_ROOT)/mm-camera/mm-camera2

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_C_INCLUDES:= $(LOCAL_PATH)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../includes/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../lookup/
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
LOCAL_C_INCLUDES+= $(MM_CAMERA_PATH)/media-controller/modules/sensors/sensors/
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

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/media
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include
LOCAL_SHARED_LIBRARIES := libmmcamera_tuning_lookup
LOCAL_SHARED_LIBRARIES += libmmcamera2_mct
LOCAL_SHARED_LIBRARIES += libmmcamera2_stats_modules
LOCAL_SHARED_LIBRARIES += libmmcamera2_iface_modules
LOCAL_SHARED_LIBRARIES += libmmcamera2_isp_modules
LOCAL_SHARED_LIBRARIES += libmmcamera2_sensor_modules
LOCAL_SHARED_LIBRARIES += libmmcamera2_pproc_modules
LOCAL_SHARED_LIBRARIES += libmmcamera2_imglib_modules
LOCAL_SHARED_LIBRARIES += libmmcamera_eztune_module
LOCAL_SHARED_LIBRARIES += libmm-qcamera
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_HEADER_LIBRARIES := libutils_headers
ifeq ($(USE_FASTCV_OPT),true)
LOCAL_SHARED_LIBRARIES += libfastcvopt
endif
LOCAL_SHARED_LIBRARIES += libmmcamera_dbg


# Build tuning library
LOCAL_MODULE:= libmmcamera_tuning
include $(SDCLANG_COMMON_DEFS)
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

include $(BUILD_SHARED_LIBRARY)

endif
