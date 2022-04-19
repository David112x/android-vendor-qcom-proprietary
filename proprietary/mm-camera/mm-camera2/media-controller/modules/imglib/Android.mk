#======================================================================
# makefile for libmmcamera2_imglib_modules.so for mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH := $(call my-dir)
LOCAL_IMGLIB_PATH  := $(LOCAL_PATH)

COMMON_DEFINES := -DHDR_LIB_GHOSTBUSTER

ifeq ($(call is-board-platform-in-list, msm8992 msm8996 msm8952 msm8998 apq8098_latv sdm660 msm8953),true)
  FD_USES_DSP := true
else
  FD_USES_DSP := false
endif
ifeq ($(call is-board-platform-in-list, sdm660 $(TRINKET)),true)
  USE_CDSP := true
else
  USE_CDSP := false
endif

FD_ENABLE_SW_INTERMITTENT := false

ifeq ($(call is-board-platform-in-list, msm8998 apq8098_latv msm8953 sdm660),true)
  FD_USE_V65 := true
else
  FD_USE_V65 := false
endif

ifeq ($(call is-board-platform-in-list, $(TRINKET)),true)
    FD_USE_V8 := true
else
    FD_USE_V8 := false
endif

ifeq ($(call is-board-platform-in-list, sdm660 msm8998 apq8098_latv $(TRINKET)),true)
 DSP_CLK_ENABLE_DCVS := true
else
 DSP_CLK_ENABLE_DCVS := false
endif

ifneq ($(call is-board-platform-in-list, msm8996 msm8998 apq8098_latv),true)
  USES_HVX := false
else
  USES_HVX := true
endif

FACEPROC_USE_NEW_WRAPPER := true
IMGLIB_USES_FASTCV ?= true
LE_PREBUILD_LIB ?= false

ifeq ($(strip $(FACEPROC_USE_NEW_WRAPPER)),true)
  ifeq ($(strip $(FD_USE_V65)),true)
    ifeq ($(strip $(FD_USES_DSP)),true)
      FDDSP_USE_SW_WRAPPER_FOR_ALL := true
    endif
  endif
endif

include $(CLEAR_VARS)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS :=  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)
LOCAL_CFLAGS += $(COMMON_DEFINES)

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
ifeq ($(CHROMATIX_VERSION), 0309)
  LOCAL_CFLAGS += -DCHROMATIX_309
endif
ifeq ($(CHROMATIX_VERSION), 0310)
  LOCAL_CFLAGS += -DCHROMATIX_310
endif
ifeq ($(CHROMATIX_VERSION), 0308E)
  LOCAL_CFLAGS += -DCHROMATIX_308E
  LOCAL_CFLAGS += -D_USE_OEM_CHROMATIX_
endif

ifeq ($(OEM_CHROMATIX_0310), 1)
  LOCAL_CFLAGS += -DCHROMATIX_310E
  LOCAL_CFLAGS += -D_USE_OEM_CHROMATIX_
endif

ifeq ($(MSM_VERSION), 8952)
  LOCAL_CFLAGS += -D__IMGLIB_USE_OEM_FEAT1_STUB__
endif

ifeq ($(strip $(DSP_CLK_ENABLE_DCVS)),true)
  LOCAL_CFLAGS += -D_ENABLE_DCVS_
endif

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2

USE_CAC_V1:= false
USE_CAC_V2:= false
USE_CAC_V3:= true

LOCAL_C_INCLUDES += $(LOCAL_PATH)
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../system/core/libion/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../system/core/libion/kernel-headers/
endif
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
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/common
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/hdr
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/wd
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/faceproc
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/frameproc
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/cac
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/cac_v2
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/cac_v3
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/include
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/lib
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/lib/faceproc
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/components/lib/cac
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/modules
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/modules/common
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/modules/base
LOCAL_C_INCLUDES += $(LOCAL_IMGLIB_PATH)/utils
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera-lib/cp/prebuilt

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(LOCAL_EXTEN_CHROMATIX_FILE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_CFLAGS += -Werror -Wno-unused-parameter -Wno-unused-function
LOCAL_CFLAGS += -Wno-unused-variable -Wall

ifeq ($(strip $(FACEPROC_USE_NEW_WRAPPER)),true)
  ifeq ($(strip $(FD_USE_V65)),true)
    LOCAL_CFLAGS  += -DFD_USE_V65
  endif
  ifeq ($(strip $(FD_ENABLE_SW_INTERMITTENT)),true)
    LOCAL_CFLAGS  += -DFD_ENABLE_SW_INTERMITTENT
  endif
endif
ifeq ($(strip $(PROFILE_FD)),true)
  LOCAL_CFLAGS  += -DPROFILE_FD
endif

ifeq ($(strip $(USE_CAC_V1)),true)
  LOCAL_CFLAGS += -DUSE_CAC_V1
endif
ifeq ($(strip $(USE_CAC_V2)),true)
  LOCAL_CFLAGS += -DUSE_CAC_V2
endif
ifeq ($(strip $(USE_CAC_V3)),true)
  LOCAL_CFLAGS += -DUSE_CAC_V3
endif

ifeq ($(strip $(FD_USES_DSP)),true)
mmimg_defines += -DFD_WITH_DSP_NEW
endif

ifeq ($(strip $(USES_HVX)),true)
mmimg_defines += -DHVX_ENABLED
endif

LOCAL_CFLAGS += $(mmimg_defines)

#LOCAL_CFLAGS += -DCAMERA_FEATURE_THIRDPARTYEIS

LOCAL_SRC_DIR := $(LOCAL_PATH)/modules
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_MODULE           := libmmcamera2_imglib_modules
include $(SDCLANG_COMMON_DEFS)
#LOCAL_CLANG            := true


LOCAL_SHARED_LIBRARIES := liblog libcutils libmmcamera2_mct libmmcamera_imglib libmmcamera_dbg
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif

ifeq ($(strip $(USE_CDSP)),true)
LOCAL_SHARED_LIBRARIES += libcdsprpc
else
LOCAL_SHARED_LIBRARIES += libadsprpc
endif
LOCAL_SHARED_LIBRARIES += libdl
ifeq ($(INC_PERF_API),true)
  LOCAL_SHARED_LIBRARIES+= libqti-perfd-client
endif

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
################ faceproc prebuilt library #############

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

ifeq ($(strip $(LE_PREBUILD_LIB)),true)
  LOCAL_CFLAGS += -DFD_DISABLE_UPFRONT
endif

ifeq ($(FEATURE_FACE_PROC),true)
  include $(CLEAR_VARS)
  LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
ifeq ($(strip $(LE_PREBUILD_LIB)),true)
    LOCAL_PATH := $(LOCAL_IMGLIB_PATH)/components/lib/faceproc
else
  ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= 23 ))" )))
    LOCAL_PATH := $(LOCAL_IMGLIB_PATH)/components/lib/faceproc
  else
    LOCAL_PATH := $(LOCAL_MMCAMERA_PATH)/../server/frameproc/face_proc/engine
  endif
endif
  LOCAL_MODULE       := libmmcamera_faceproc
  include $(SDCLANG_COMMON_DEFS)
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
ifeq ($(strip $(LE_PREBUILD_LIB)),true)
  LOCAL_SRC_FILES := libmmcamera_faceproc_le.so
  LOCAL_MULTILIB := 32
else
  LOCAL_SRC_FILES_32 := libmmcamera_faceproc.so
  LOCAL_SRC_FILES_64 := libmmcamera_faceproc_la_v51_64bit.so
  LOCAL_MULTILIB := both
endif
  LOCAL_MODULE_TAGS := optional
  LOCAL_MODULE_OWNER := qti
  LOCAL_PROPRIETARY_MODULE := true

  ifeq ($(strip $(LE_PREBUILD_LIB)),true)
    LOCAL_32_BIT_ONLY := true
  endif

  include $(BUILD_PREBUILT)
endif
###############################################################
LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

ifeq ($(strip $(LE_PREBUILD_LIB)),true)
  LOCAL_CFLAGS += -DFD_DISABLE_UPFRONT
endif

ifeq ($(FEATURE_FACE_PROC),true)
  include $(CLEAR_VARS)
  LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
ifeq ($(strip $(LE_PREBUILD_LIB)),true)
    LOCAL_PATH := $(LOCAL_IMGLIB_PATH)/components/lib/faceproc
else
  ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= 23 ))" )))
    LOCAL_PATH := $(LOCAL_IMGLIB_PATH)/components/lib/faceproc
  else
    LOCAL_PATH := $(LOCAL_MMCAMERA_PATH)/../server/frameproc/face_proc/engine
  endif
endif
  LOCAL_MODULE       := libmmcamera_faceproc_system
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
ifeq ($(strip $(LE_PREBUILD_LIB)),true)
  LOCAL_SRC_FILES := libmmcamera_faceproc_le.so
  LOCAL_MULTILIB := 32
else
  LOCAL_SRC_FILES_32 := libmmcamera_faceproc.so
  LOCAL_SRC_FILES_64 := libmmcamera_faceproc_la_v51_64bit.so
  LOCAL_MULTILIB := both
endif
  LOCAL_MODULE_TAGS := optional
  LOCAL_INSTALLED_MODULE_STEM := libmmcamera_faceproc.so

  LOCAL_HEADER_LIBRARIES := libutils_headers

  ifeq ($(strip $(LE_PREBUILD_LIB)),true)
    LOCAL_32_BIT_ONLY := true
  endif

  include $(BUILD_PREBUILT)
endif


################ faceproc prebuilt v6 library #############

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

ifeq ($(strip $(FACEPROC_USE_NEW_WRAPPER)),true)
  include $(CLEAR_VARS)
  LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
  LOCAL_PATH := $(LOCAL_IMGLIB_PATH)/components/lib/faceproc/lib
  LOCAL_MODULE       := libmmcamera_faceproc2
  include $(SDCLANG_COMMON_DEFS)
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
ifeq ($(strip $(LE_PREBUILD_LIB)),true)
  LOCAL_SRC_FILES := libmmcamera_faceproc_le.so
  LOCAL_MULTILIB := 32
else
  ifeq ($(strip $(FD_USE_V65)),true)
    LOCAL_SRC_FILES_32 := libmmcamera_faceproc2_la_v65.so
    LOCAL_SRC_FILES_64 := libmmcamera_faceproc2_la_v65_64bit.so
  else ifeq ($(strip $(FD_USE_V8)),true)
    LOCAL_SRC_FILES_32 := libmmcamera_faceproc2_la_v8.so
    LOCAL_SRC_FILES_64 := libmmcamera_faceproc2_la_v8_64bit.so
  else
    LOCAL_SRC_FILES_32 := libmmcamera_faceproc.so
    LOCAL_SRC_FILES_64 := libmmcamera_faceproc2_la_v60_64bit.so
  endif
  LOCAL_MULTILIB := both
endif
  LOCAL_MODULE_TAGS := optional
  LOCAL_MODULE_OWNER := qti
  LOCAL_PROPRIETARY_MODULE := true

  LOCAL_HEADER_LIBRARIES := libutils_headers

  ifeq ($(strip $(LE_PREBUILD_LIB)),true)
    LOCAL_32_BIT_ONLY := true
  endif

  include $(BUILD_PREBUILT)
endif

#######################################################################

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

ifeq ($(strip $(FACEPROC_USE_NEW_WRAPPER)),true)
  include $(CLEAR_VARS)
  LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
  LOCAL_PATH := $(LOCAL_IMGLIB_PATH)/components/lib/faceproc/lib
  LOCAL_MODULE       := libmmcamera_faceproc2_system
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
ifeq ($(strip $(LE_PREBUILD_LIB)),true)
  LOCAL_SRC_FILES := libmmcamera_faceproc_le.so
  LOCAL_MULTILIB := 32
else
  ifeq ($(strip $(FD_USE_V65)),true)
    LOCAL_SRC_FILES_32 := libmmcamera_faceproc2_la_v65.so
    LOCAL_SRC_FILES_64 := libmmcamera_faceproc2_la_v65_64bit.so
  else ifeq ($(strip $(FD_USE_V8)),true)
    LOCAL_SRC_FILES_32 := libmmcamera_faceproc2_la_v8.so
    LOCAL_SRC_FILES_64 := libmmcamera_faceproc2_la_v8_64bit.so
  else
    LOCAL_SRC_FILES_32 := libmmcamera_faceproc.so
    LOCAL_SRC_FILES_64 := libmmcamera_faceproc2_la_v60_64bit.so
  endif
  LOCAL_MULTILIB := both
endif
  LOCAL_MODULE_TAGS := optional
  LOCAL_INSTALLED_MODULE_STEM := libmmcamera_faceproc2.so
  ifeq ($(strip $(LE_PREBUILD_LIB)),true)
    LOCAL_32_BIT_ONLY := true
  endif

  include $(BUILD_PREBUILT)
endif



################ component library ######################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

mmimg_defines :=  -DAMSS_VERSION=$(AMSS_VERSION) \
       $(mmcamera_debug_defines) \
       $(mmcamera_debug_cflags) \
       -g -O0 \
       -include img_dbg.h

mmimg_defines += -Werror -Wno-unused-parameter -Wno-unused-function
mmimg_defines += -Wno-unused-variable -Wall

mmimg_defines += -DLOGE=ALOGE

INC_PERF_API ?= true

ifeq ($(strip $(TARGET_USES_ION)),true)
mmimg_defines += -DUSE_ION
endif

ifeq ($(call is-board-platform-in-list, sdm660 msm8998 apq8098_latv $(TRINKET)),true)
  JPEG_DMA := true
else
  JPEG_DMA := false
endif

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../system/core/libion/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/hdr
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/wd
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/faceproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/faceproc_dsp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/faceproc_hw
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/frameproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/cac
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/cac_v2
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/cac_v3
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib2d
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib/faceproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib/faceproc/libincludes/track
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib/faceproc/wrappers
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib/faceproc/common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib/cac
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_EXTEN_CHROMATIX_FILE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qcom/display
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/adreno/
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/adreno200/
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_HEADER_LIBRARIES += libnativebase_headers
LOCAL_HEADER_LIBRARIES += display_headers

ifeq ($(strip $(JPEG_DMA)),true)
  LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/jpegdma
endif

ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
mmimg_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
mmimg_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/videodev2.h
endif
endif

ifeq ($(strip $(FD_USES_DSP)),true)
mmimg_defines += -DFD_WITH_DSP_NEW
endif
LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
LOCAL_CFLAGS := $(mmimg_defines)
LOCAL_CFLAGS += $(COMMON_DEFINES)
LOCAL_CFLAGS += -D__ANDROID__

ifeq ($(strip $(FACEPROC_USE_NEW_WRAPPER)),true)
  LOCAL_CFLAGS  += -DFACEPROC_USE_NEW_WRAPPER
  ifeq ($(strip $(FD_USE_V65)),true)
    LOCAL_CFLAGS  += -DFD_USE_V65
    LOCAL_CFLAGS  += -DFD_VERSION=6.5
  else
    LOCAL_CFLAGS  += -DFD_VERSION=6.0
  endif
else
  LOCAL_CFLAGS  += -DFD_VERSION=5
endif

ifeq ($(strip $(FDDSP_USE_SW_WRAPPER_FOR_ALL)),true)
  LOCAL_CFLAGS  += -DFDDSP_USE_SW_WRAPPER_FOR_ALL
endif

ifeq ($(strip $(PROFILE_FD)),true)
  LOCAL_CFLAGS  += -DPROFILE_FD
endif

ifeq ($(call is-board-platform-in-list,msm8610),true)
  LOCAL_CFLAGS  += -DUSE_SMMU_BUFFERS_FOR_WNR
endif #msm8610

ifeq ($(call is-board-platform-in-list,msm8952),true)
  LOCAL_CFLAGS  += -DUSE_SMMU_BUFFERS_FOR_FDDSP
endif

ifeq ($(call is-board-platform-in-list,msm8937),true)
  LOCAL_CFLAGS  += -DUSE_SMMU_BUFFERS_FOR_FDDSP
endif

ifeq ($(call is-board-platform-in-list,msm8953),true)
  LOCAL_CFLAGS  += -DUSE_SMMU_BUFFERS_FOR_FDDSP
endif
ifeq ($(strip $(IMGLIB_USES_FASTCV)),true)
  LOCAL_CFLAGS += -D__CAM_FCV__
endif

ifeq ($(INC_PERF_API),true)
  LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/android-perf/mp-ctl/
  LOCAL_CFLAGS += -DUSE_PERF_API
endif

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
ifeq ($(CHROMATIX_VERSION), 0309)
  LOCAL_CFLAGS += -DCHROMATIX_309
endif
ifeq ($(CHROMATIX_VERSION), 0310)
  LOCAL_CFLAGS += -DCHROMATIX_310
endif
ifeq ($(CHROMATIX_VERSION), 0308E)
  LOCAL_CFLAGS += -DCHROMATIX_308E
endif
ifeq ($(OEM_CHROMATIX_0310), 1)
  LOCAL_CFLAGS += -DCHROMATIX_310E
endif

ifeq ($(strip $(JPEG_DMA)),true)
  LOCAL_CFLAGS += -DJPEG_DMA_AVIL
endif

LOCAL_SRC_FILES := utils/img_queue.c
LOCAL_SRC_FILES += utils/img_buffer.c
LOCAL_SRC_FILES += utils/img_list.c
LOCAL_SRC_FILES += utils/img_thread.c
LOCAL_SRC_FILES += utils/img_rb.c
LOCAL_SRC_FILES += utils/img_dsp_dl_mgr.c
LOCAL_SRC_FILES += utils/img_fcv.c
LOCAL_SRC_FILES += components/common/img_thread_mgr.c
LOCAL_SRC_FILES += components/common/img_common.c
LOCAL_SRC_FILES += components/common/img_meta.c
LOCAL_SRC_FILES += components/common/img_comp.c
LOCAL_SRC_FILES += components/common/img_comp_factory.c
LOCAL_SRC_FILES += components/common/ds_2by2_asm.s
LOCAL_SRC_FILES += components/common/boost_armv7_gcc.s

LOCAL_SRC_FILES += components/wd/wd_comp.c
LOCAL_SRC_FILES += components/hdr/hdr_comp.c
LOCAL_SRC_FILES += components/faceproc/faceproc_comp.c
LOCAL_SRC_FILES += components/faceproc_dsp/faceproc_dsp_comp.c
LOCAL_SRC_FILES += components/faceproc_hw/faceproc_hw_comp.c
LOCAL_SRC_FILES += components/faceproc_hw/faceproc_hw_tracker_wrapper.c
LOCAL_SRC_FILES += components/lib/faceproc/common/faceproc_common.c
LOCAL_SRC_FILES += components/lib/faceproc/wrappers/facial_parts_wrapper.c
LOCAL_SRC_FILES += components/frameproc/frameproc_comp.c
LOCAL_SRC_FILES += components/frameproc/dual_frameproc_comp.c
LOCAL_SRC_FILES += components/multiframeproc/multiframeproc_comp.c
LOCAL_SRC_FILES += components/faceproc/dynamic_work_partition.c
LOCAL_SRC_FILES += components/cac/cac_comp.c
LOCAL_SRC_FILES += components/cac_v2/cac_v2_comp.c
LOCAL_SRC_FILES += components/cac_v3/cac_v3_comp.c
LOCAL_SRC_FILES += components/lib2d/lib2d_comp.c
LOCAL_SRC_FILES += components/lib2d/c2d_utils.c

ifeq ($(strip $(JPEG_DMA)),true)
  LOCAL_SRC_FILES += components/jpegdma/jpegdma_comp.c
  LOCAL_SRC_FILES += components/jpegdma/jpegdma_util.c
endif

ifeq ($(strip $(FACEPROC_USE_NEW_WRAPPER)),true)
  LOCAL_SRC_FILES += components/lib/faceproc/wrappers/faceproc_sw_wrapper.c
  LOCAL_SRC_FILES += components/lib/faceproc/wrappers/faceproc_lib_wrapper.c
  LOCAL_SRC_FILES += components/faceproc/faceproc_comp_engine.c
else
  LOCAL_SRC_FILES += components/faceproc/faceproc_comp_eng.c
endif

ifeq ($(strip $(FDDSP_USE_SW_WRAPPER_FOR_ALL)),true)
  LOCAL_SRC_FILES += components/faceproc_dsp/faceproc_dsp_comp_engine.c
else
  LOCAL_SRC_FILES += components/faceproc_dsp/faceproc_dsp_comp_eng.c
endif

LOCAL_MODULE           := libmmcamera_imglib
include $(SDCLANG_COMMON_DEFS)
#LOCAL_CLANG            := true
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libdl libcutils liblog libmmcamera_dbg
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_SHARED_LIBRARIES  += libion
endif

ifeq ($(strip $(USE_CDSP)),true)
LOCAL_SHARED_LIBRARIES += libcdsprpc
else
LOCAL_SHARED_LIBRARIES += libadsprpc
endif


ifeq ($(strip $(IMGLIB_USES_FASTCV)),true)
LOCAL_SHARED_LIBRARIES += libfastcvopt
endif


ifeq ($(strip $(IMGLIB_USES_FASTCV)),true)
LOCAL_SHARED_LIBRARIES += libfastcvopt
endif

LOCAL_MODULE_TAGS := optional
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr 
include $(BUILD_SHARED_LIBRARY)


################ Imglib headers copy ############

OLD_LOCAL_COPY_HEADERS_TO := $(LOCAL_COPY_HEADERS_TO)
OLD_LOCAL_COPY_HEADERS := $(LOCAL_COPY_HEADERS)

IMGLIB_HEADER_PATH := mm-camera/imglib
LOCAL_COPY_HEADERS_TO := $(IMGLIB_HEADER_PATH)
LOCAL_COPY_HEADERS := ./utils/img_buffer.h
LOCAL_COPY_HEADERS += ./components/include/img_common.h
LOCAL_COPY_HEADERS += ./components/include/img_comp.h
LOCAL_COPY_HEADERS += ./components/include/img_meta.h
LOCAL_COPY_HEADERS += ./components/include/img_comp_factory.h
LOCAL_COPY_HEADERS += ./components/include/lib2d.h
LOCAL_COPY_HEADERS += ./components/lib/img_mem_ops.h
LOCAL_COPY_HEADERS += ./components/lib/img_thread_ops.h
include $(BUILD_COPY_HEADERS)

LOCAL_COPY_HEADERS_TO := $(OLD_LOCAL_COPY_HEADERS_TO)
LOCAL_COPY_HEADERS := $(OLD_LOCAL_COPY_HEADERS)

################ Face Detection adsp stub library ############

ifeq ($(strip $(FD_USES_DSP)),true)
  include $(CLEAR_VARS)
  LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
  LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)
  LOCAL_MODULE       := libmmcamera_imglib_faceproc_adspstub
  include $(SDCLANG_COMMON_DEFS)
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
  LOCAL_C_INCLUDES = $(TARGET_OUT_HEADERS)/adsprpc/inc
  LOCAL_SRC_FILES := components/faceproc_dsp/adsp_fd_stub.c
  ifeq ($(strip $(USE_CDSP)),true)
  LOCAL_SHARED_LIBRARIES := libcdsprpc libcutils libdl liblog
  else
  LOCAL_SHARED_LIBRARIES := libadsprpc libcutils libdl liblog
  endif
  LOCAL_MODULE_TAGS := optional
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
  LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif
  LOCAL_MODULE_OWNER := qti
  LOCAL_PRELINK_MODULE   := false
  LOCAL_PROPRIETARY_MODULE := true

  ifeq ($(32_BIT_FLAG), true)
  LOCAL_32_BIT_ONLY := true
  endif

  include $(BUILD_SHARED_LIBRARY)
endif

################ Face Detection adsp stub v6.x library ############

ifeq ($(strip $(FD_USES_DSP)),true)
  include $(CLEAR_VARS)
  LOCAL_LDFLAGS := $(mmcamera_debug_lflags)
  LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)
  LOCAL_MODULE       := libmmcamera_imglib_faceproc_adspstub2
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := SHARED_LIBRARIES
  LOCAL_C_INCLUDES = $(TARGET_OUT_HEADERS)/adsprpc/inc
  LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/faceproc_dsp
  LOCAL_SRC_FILES := components/faceproc_dsp/stub/adsp_fd_stub.c
  ifeq ($(strip $(USE_CDSP)),true)
  LOCAL_SHARED_LIBRARIES := libcdsprpc libcutils libdl liblog
  else
  LOCAL_SHARED_LIBRARIES := libadsprpc libcutils libdl liblog
  endif
  LOCAL_MODULE_TAGS := optional
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
  LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif
  LOCAL_MODULE_OWNER := qti
  LOCAL_PRELINK_MODULE   := false
  LOCAL_PROPRIETARY_MODULE := true

  ifeq ($(32_BIT_FLAG), true)
  LOCAL_32_BIT_ONLY := true
  endif

  include $(BUILD_SHARED_LIBRARY)
endif

################ Face Detection Q6 prebuilt library #############

ifeq ($(strip $(FD_USES_DSP)),true)
 include $(CLEAR_VARS)
  LOCAL_MODULE       := libadsp_fd_skel
  include $(SDCLANG_COMMON_DEFS)
  LOCAL_MODULE_SUFFIX := .so
  LOCAL_MODULE_CLASS := ETC
  ifeq ($(strip $(FD_USE_V65)),true)
    LOCAL_SRC_FILES := components/lib/faceproc/lib/libadsp_fd_skel_la_v65.so
  else
    LOCAL_SRC_FILES := components/lib/faceproc/libadsp_fd_skel.so
  endif
  LOCAL_MODULE_TAGS := optional
  LOCAL_MODULE_OWNER := qti
  LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib/rfsa/adsp

  ifeq ($(32_BIT_FLAG), true)
  LOCAL_32_BIT_ONLY := true
  endif

  include $(BUILD_PREBUILT)
endif
##########################test app ##############################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

mmimg_defines:= \
       $(mmcamera_debug_defines) \
       $(mmcamera_debug_cflags) \
       -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \
       -include img_dbg.h

mmimg_defines += -DLOGE=ALOGE -Werror
mmimg_defines += -Werror -Wno-unused-parameter -Wno-unused-function
mmimg_defines += -Wno-unused-variable -Wall
ifeq ($(strip $(TARGET_USES_ION)),true)
mmimg_defines += -DUSE_ION
endif

ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/include/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/hdr
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/wd
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/faceproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/cac
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/frameproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/include/faceproc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/test
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_EXTEN_CHROMATIX_FILE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
mmimg_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h
mmimg_defines += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/videodev2.h
endif
endif

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(mmimg_defines)
LOCAL_CFLAGS += $(COMMON_DEFINES)
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
ifeq ($(CHROMATIX_VERSION), 0309)
  LOCAL_CFLAGS += -DCHROMATIX_309
endif
ifeq ($(CHROMATIX_VERSION), 0310)
  LOCAL_CFLAGS += -DCHROMATIX_310
endif
ifeq ($(CHROMATIX_VERSION), 0308E)
  LOCAL_CFLAGS += -DCHROMATIX_308E
endif
ifeq ($(OEM_CHROMATIX_0310), 1)
  LOCAL_CFLAGS += -DCHROMATIX_310E
endif
LOCAL_SRC_FILES := components/test/img_test.c
LOCAL_SRC_FILES += components/test/hdr_test.c
LOCAL_SRC_FILES += components/test/denoise_test.c
LOCAL_SRC_FILES += components/test/faceproc_test.c
LOCAL_SRC_FILES += components/test/cac_test.c
LOCAL_SRC_FILES += components/test/dual_frameproc_test.c
LOCAL_SRC_FILES += components/test/buf_pool_test.c

LOCAL_SHARED_LIBRARIES:= \
    libmmcamera_imglib libcutils libdl libmmcamera_dbg
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE:= mm-imglib-test
LOCAL_PROPRIETARY_MODULE := true
#LOCAL_CLANG := true

LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr  
include $(BUILD_EXECUTABLE)

##########################hdr module test app ##############################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

mmimg_defines:= \
       $(mmcamera_debug_defines) \
       $(mmcamera_debug_cflags) \
       -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_

mmimg_defines += -DLOGE=ALOGE -Werror
mmimg_defines += -Werror -Wno-unused-parameter -Wno-unused-function
mmimg_defines += -Wno-unused-variable -Wall

ifeq ($(strip $(TARGET_USES_ION)),true)
mmimg_defines += -DUSE_ION
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
ifeq ($(CHROMATIX_VERSION), 0308E)
  LOCAL_CFLAGS += -DCHROMATIX_308E
endif
ifeq ($(OEM_CHROMATIX_0310), 1)
  LOCAL_CFLAGS += -DCHROMATIX_310E
endif
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

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils

LOCAL_C_INCLUDES += $(LOCAL_EXTEN_CHROMATIX_FILE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(mmimg_defines)
LOCAL_CFLAGS += $(COMMON_DEFINES)

LOCAL_SRC_FILES := test/test_module_hdr.c

LOCAL_SHARED_LIBRARIES:= \
    libmmcamera2_imglib_modules libcutils libdl libmmcamera2_mct libmmcamera_dbg

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE:= mm-module-hdr-test
#LOCAL_CLANG := true

LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_EXECUTABLE)

##########################module test app ##############################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

mmimg_defines:= \
       $(mmcamera_debug_defines) \
       $(mmcamera_debug_cflags) \
       -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \

mmimg_defines += -DLOGE=ALOGE -Werror
mmimg_defines += -Werror -Wno-unused-parameter -Wno-unused-function
mmimg_defines += -Wno-unused-variable -Wall

ifeq ($(strip $(TARGET_USES_ION)),true)
mmimg_defines += -DUSE_ION
endif

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

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/utils

LOCAL_C_INCLUDES += $(LOCAL_EXTEN_CHROMATIX_FILE_PATH)
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_SRC_FILES := test/test_module_imglib.c

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(mmimg_defines)

LOCAL_SHARED_LIBRARIES:= \
    libmmcamera2_imglib_modules libcutils libdl libmmcamera2_mct \
    libmmcamera_dbg

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE:= mm-module-imglib-test
#LOCAL_CLANG := true

LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
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
ifeq ($(CHROMATIX_VERSION), 0308E)
  LOCAL_CFLAGS += -DCHROMATIX_308E
endif
ifeq ($(OEM_CHROMATIX_0310), 1)
  LOCAL_CFLAGS += -DCHROMATIX_310E
endif
include $(BUILD_EXECUTABLE)

##########################module test app ##############################
include $(CLEAR_VARS)

LOCAL_PATH:= $(LOCAL_IMGLIB_PATH)

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

mmimg_defines:= -DAMSS_VERSION=$(AMSS_VERSION) \
       $(mmcamera_debug_defines) \
       $(mmcamera_debug_cflags) \
       -g -O0 \
       -D_ANDROID_ \
       -include img_dbg.h \

mmimg_defines += -DLOGE=ALOGE -Werror
mmimg_defines += -Werror -Wno-unused-parameter -Wno-unused-function
mmimg_defines += -Wno-unused-variable -Wall

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
ifneq ($(TARGET_KERNEL_VERSION),$(filter $(TARGET_KERNEL_VERSION),3.18 4.4 4.9))
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/include/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../../../../system/core/libion/kernel-headers/
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/components/lib
LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_HEADER_LIBRARIES := libutils_headers

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := $(mmimg_defines)
LOCAL_CFLAGS += $(COMMON_DEFINES)

LOCAL_SRC_FILES := test/imgalgo_dummy.c

LOCAL_MODULE           := libmmcamera_dummyalgo
include $(SDCLANG_COMMON_DEFS)
#LOCAL_CLANG            := true
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libdl libcutils liblog libmmcamera_imglib libmmcamera_dbg

LOCAL_MODULE_TAGS := optional
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif
include $(BUILD_SHARED_LIBRARY)


###################### Frameproc Libraries ##########################

########################## SW2D Module ##############################
ifeq ($(strip $(IMGLIB_USES_FASTCV)),true)
include $(LOCAL_IMGLIB_PATH)/components/lib/sw2d/Android.mk
endif

########################## PPEISCore ##############################
ifeq ($(strip $(LE_PREBUILD_LIB)),false)
include $(LOCAL_IMGLIB_PATH)/components/lib/ppeiscore/Android.mk
endif

########################## THIRDPARTYEIS ##############################
ifeq ($(strip $(LE_PREBUILD_LIB)),false)
include $(LOCAL_IMGLIB_PATH)/components/lib/thirdpartyeis/Android.mk
endif

########################## Bokeh ##############################
#ifeq ($(SUPPORT_DUAL_CAMERA), true)
ifneq ($(TARGET_HAS_LOW_RAM), true)
include $(LOCAL_IMGLIB_PATH)/components/lib/bokeh/Android.mk
endif
#endif

########################## jpegdma v4l2 test app ##############################
include $(LOCAL_IMGLIB_PATH)/components/jpegdma_test/Android.mk

########################## QuadraCFA Wrapper  ##############################
include $(LOCAL_IMGLIB_PATH)/components/quadracfa/Android.mk

########################## MultiFrameProc Precpp Wrapper  ##############################
include $(LOCAL_IMGLIB_PATH)/components/mfproc_precpp/Android.mk

########################## MultiFrameProc Postcpp Wrapper  ##############################
include $(LOCAL_IMGLIB_PATH)/components/mfproc_postcpp/Android.mk

#########################  SAT Dummy Wrapper  ############################
ifneq ($(TARGET_HAS_LOW_RAM), true)
include $(LOCAL_IMGLIB_PATH)/components/lib/sat/Android.mk
endif

endif # is-vendor-board-platform,QCOM
