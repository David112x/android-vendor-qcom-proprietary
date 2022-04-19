# ---------------------------------------------------------------------------
#                  Make the shared library (libmmcamera_cpp)
# ---------------------------------------------------------------------------

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

PPROC_MODULE_PATH := $(LOCAL_PATH)/../../pproc-new
MM_CAMERA_PATH := $(LOCAL_PATH)/../../../../../mm-camera2

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(PPROC_MODULE_PATH)
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../../system/core/libion/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../../system/core/libion/kernel-headers/
endif
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
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/debug
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/debug/fdleak
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/mct/pipeline
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/modules/includes
LOCAL_C_INCLUDES += $(MM_CAMERA_PATH)/media-controller/modules/pproc-new/buf_mgr
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) $(mmcamera_debug_cflags)

ifeq ($(CHROMATIX_VERSION), 0304)
  LOCAL_CFLAGS += -DCHROMATIX_304
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
ifeq ($(CHROMATIX_VERSION), 0310)
  LOCAL_CFLAGS += -DCHROMATIX_310
endif
ifeq ($(CHROMATIX_VERSION), 0310E)
  LOCAL_CFLAGS += -DCHROMATIX_310E
endif
ifeq ($(CHROMATIX_VERSION), 0308E)
  LOCAL_CFLAGS += -DCHROMATIX_308E
endif
ifeq ($(CHROMATIX_VERSION), 0309)
  LOCAL_CFLAGS += -DCHROMATIX_309
endif

ifeq ($(strip $(OEM_CHROMATIX)),true)
    LOCAL_CFLAGS += -DCHROMATIX_EXT
    LOCAL_CFLAGS += -DOEM_CHROMATIX
    LOCAL_CFLAGS += -DBEAUTY_FACE
    LOCAL_CFLAGS += -DREQUEST_FRAMES
    LOCAL_C_INCLUDES += $(LOCAL_EXTEN_PPROC_INCLUDES)
endif

ifeq ($(call is-board-platform-in-list,msm8226 msm8610),true)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

ifeq  ($(call is-board-platform-in-list,msm8996 sdm660 msm8998 apq8098_latv $(TRINKET)),true)
  LOCAL_CFLAGS += -DCAMERA_USE_CHROMATIX_HW_WNR_TYPE
endif

#LOCAL_CFLAGS += -DCAMERA_FEATURE_THIRDPARTYEIS

LOCAL_CFLAGS += -Werror
LOCAL_SRC_FILES := cpp_module.c
LOCAL_SRC_FILES += cpp_module_events.c
LOCAL_SRC_FILES += cpp_module_util.c
LOCAL_SRC_FILES += cpp_port.c
LOCAL_SRC_FILES += cpp_thread.c
LOCAL_SRC_FILES += cpp_hardware.c
LOCAL_SRC_FILES += cpp_hw_params.c
LOCAL_SRC_FILES += cpp_hw_params_1_8.c
LOCAL_SRC_FILES += cpp_tnr_params.c
LOCAL_SRC_FILES += cpp_hw_params_1_10.c
LOCAL_SRC_FILES += cpp_hw_params_1_12.c
LOCAL_SRC_FILES += cpp_pbf_params.c
LOCAL_SRC_FILES += cpp_wnr_params.c
LOCAL_SRC_FILES += cpp_hw_params_1_5.c
ifeq ($(CHROMATIX_VERSION), 0308E)
LOCAL_SRC_FILES += cpp_hw_params_utils_ext.c
endif
ifeq ($(OEM_CHROMATIX), true)
LOCAL_SRC_FILES += ../../../../../../mm-camera-ext/mm-camera2/media-controller/modules/pproc-new/cpp/cpp_hw_params_ext.c
endif

LOCAL_SRC_FILES += cpp_chromatix_int.c
LOCAL_MODULE           := libmmcamera2_cpp_module
include $(SDCLANG_COMMON_DEFS)
LOCAL_SHARED_LIBRARIES := libcutils libmmcamera2_mct libmmcamera2_pp_buf_mgr \
                          libmmcamera_dbg
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
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

# -------- for firmware ---------- #
LOCAL_PATH:= $(LOCAL_DIR_PATH)
include $(PPROC_MODULE_PATH)/cpp/firmware/Android.mk
