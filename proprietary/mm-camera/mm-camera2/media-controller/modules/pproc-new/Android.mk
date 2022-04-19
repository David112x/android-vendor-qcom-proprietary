#======================================================================
#makefile for libmmcamera2_pproc_modules.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8974 msm8960 msm8916 msm8226 msm7627a msm8660 msm8952 msm8937 msm8610 apq8084 msm8994 msm8996 msm8992 msm8953 msm8998 apq8098_latv sdm660 $(TRINKET)),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)

LOCAL_PPROC_PATH := $(LOCAL_PATH)
LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2

FEATURE_SVHDR_EN ?= false
LE_PREBUILD_LIB  ?= false

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
ifeq ($(CHROMATIX_VERSION), 0308E)
LOCAL_CFLAGS += -DCHROMATIX_308E
OEM_CHROMATIX:=true
endif
ifeq ($(OEM_CHROMATIX_0310), 1)
OEM_CHROMATIX:=true
endif

ifeq ($(call is-board-platform-in-list,msm8974 apq8084 msm8952 msm8994 msm8992 msm8937 msm8953 sdm660 $(TRINKET)),true)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_CAC
endif

ifeq ($(call is-board-platform-in-list,msm8226 msm8610 msm8909),true)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
  SW_WNR_ENABLE := true
endif

ifeq ($(strip $(FEATURE_SVHDR_EN)),true)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_SVHDR
endif

ifeq ($(call is-board-platform-in-list,msm8960),true)
  VPE_ENABLE := true
endif

ifeq ($(call is-board-platform-in-list,apq8084),true)
  VPU_ENABLE := true
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/server-tuning/tuning/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../system/core/libion/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../system/core/libion/kernel-headers/
endif
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
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/cpp/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/c2d/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/modules/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/modules/cac/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/modules/afs/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/modules/module_imglib/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/base/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/eztune/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/buf_mgr/
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/adreno/
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/adreno200/
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_CFLAGS  += -Werror

ifdef VPU_ENABLE
 LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/vpu/
endif

ifdef SW_WNR_ENABLE
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/wnr/
endif

ifdef VPE_ENABLE
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/vpe/
endif

ifeq ($(call is-board-platform-in-list, msm8952 msm8937 msm8953 msm8996),true)
LOCAL_CFLAGS += -DCAMERA_FEATURE_SW_TNR
endif

LOCAL_CFLAGS += -DCAMERA_FEATURE_CPP
LOCAL_CFLAGS += -DCAMERA_FEATURE_TMOD
LOCAL_CFLAGS += -DCAMERA_FEATURE_LLVD
LOCAL_CFLAGS += -DCAMERA_FEATURE_C2D
LOCAL_CFLAGS += -DCAMERA_FEATURE_PPEISCORE
#LOCAL_CFLAGS += -DCAMERA_FEATURE_THIRDPARTYEIS
LOCAL_CFLAGS += -DCAMERA_FEATURE_AFS
LOCAL_CFLAGS += -DCAMERA_FEATURE_EZT
LOCAL_CFLAGS += -DCAMERA_FEATURE_QUADRA_CFA
LOCAL_CFLAGS += -DCAMERA_FEATURE_SAT
LOCAL_CFLAGS += -DCAMERA_FEATURE_SAC
LOCAL_CFLAGS += -DCAMERA_FEATURE_RTBDM
LOCAL_CFLAGS += -DCAMERA_FEATURE_RTB
LOCAL_CFLAGS += -DCAMERA_FEATURE_DEPTH_MAP
LOCAL_CFLAGS += -DCAMERA_FEATURE_MFPROC_PRECPP


ifeq ($(strip $(LE_PREBUILD_LIB)),true)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_BINCORR
endif

LOCAL_SRC_DIR := $(LOCAL_PATH)

LOCAL_SRC_FILES := pproc_module.c
LOCAL_SRC_FILES += pproc_port.c

LOCAL_MODULE           := libmmcamera2_pproc_modules
include $(SDCLANG_COMMON_DEFS)

LOCAL_SHARED_LIBRARIES := liblog libcutils libmmcamera2_cpp_module \
 libmmcamera2_mct libdl libmmcamera2_imglib_modules \
 libmmcamera_ppbase_module libmmcamera_eztune_module

LOCAL_SHARED_LIBRARIES += libmmcamera2_c2d_module libmmcamera_dbg
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif
LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

ifdef SW_WNR_ENABLE
LOCAL_SHARED_LIBRARIES += libmmcamera2_wnr_module
endif

ifdef VPE_ENABLE
LOCAL_SHARED_LIBRARIES += libmmcamera2_vpe_module
endif

ifdef VPU_ENABLE
LOCAL_SHARED_LIBRARIES += libmmcamera_vpu_module
endif

LOCAL_MODULE_TAGS      := optional
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true


ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif
#temp disable
include $(BUILD_SHARED_LIBRARY)

#************* buf_mgr module start ************#
#LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/buf_mgr/Android.mk
#************* buf_mgr module end ************#

#************* cpp module start ************#
LOCAL_PATH := $(LOCAL_PPROC_PATH)
include $(LOCAL_PATH)/cpp/Android.mk
#************* cpp module end ************#

#************* c2d module start ************#
  LOCAL_PATH := $(LOCAL_PPROC_PATH)
  include $(LOCAL_PATH)/c2d/Android.mk
#************* c2d module end ************#

#************* vpe module start ************#
ifdef VPE_ENABLE
LOCAL_PATH := $(LOCAL_PPROC_PATH)
include $(LOCAL_PATH)/vpe/Android.mk
endif
#************* vpe module end ************#

#************* wnr module start ************#
ifdef SW_WNR_ENABLE
LOCAL_PATH := $(LOCAL_PPROC_PATH)
include $(LOCAL_PATH)/wnr/Android.mk
endif
#************* wnr module end ************#

#************* vpu module start ************#
ifdef VPU_ENABLE
LOCAL_PATH := $(LOCAL_PPROC_PATH)
include $(LOCAL_PATH)/vpu/Android.mk
endif
#************* vpu module end ************#

#************* base module start ************#
LOCAL_PATH := $(LOCAL_PPROC_PATH)
include $(LOCAL_PATH)/base/Android.mk
#************* base module end ************#

#************* eztune module start ************#
LOCAL_PATH := $(LOCAL_PPROC_PATH)
include $(LOCAL_PATH)/eztune/Android.mk
#************* base module end ************#

endif # if 8960
endif # is-vendor-board-platform,QCOM
