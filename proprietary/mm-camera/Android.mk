ifneq ($(TARGET_PRODUCT),qssi)
ifneq ($(TARGET_PRODUCT),qssi_32)
ifneq ($(strip $(USE_CAMERA_STUB)),true)
COMPILE_CAMERA := true
endif #!USE_CAMERA_STUB

LOCAL_HEADER_LIBRARIES := libutils_headers

#default BUILD_CAM_FD to 0 (off)
BUILD_CAM_FD := 0

#define BUILD_SERVER
BUILD_SERVER := false
BUILD_SERVER_BOARD_PLATFORM_LIST := msm7627a
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm7630_surf
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm7630_fusion
BUILD_SERVER_BOARD_PLATFORM_LIST += msm8660
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm8960
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm8974
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm8226
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm8610
ifeq ($(call is-board-platform-in-list,$(BUILD_SERVER_BOARD_PLATFORM_LIST)),true)
  BUILD_SERVER := true
  USE_SERVER_TREE := -D_V4L2_BASED_CAM_
endif

BUILD_MM_CAMERA2 := false
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST := msm8960
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8974
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += apq8084
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8916
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8226
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8610
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8994
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8996
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8952
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8937
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8992
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8953
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += apq8098_latv
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8998
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += sdm660
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += $(TRINKET)
ifeq ($(call is-board-platform-in-list,$(BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST)),true)
  BUILD_MM_CAMERA2 := true
endif

BUILD_GCOV_TOOL := false

TARGET_COMPILE_WITH_MSM_KERNEL := true
mmcamera_debug_defines := -D_ANDROID_

BOARD_SDK_VERSION        :=20
ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= $(BOARD_SDK_VERSION) ))" )))
BOARD_QTI_CAMERA_V2      :=true
else
BOARD_QTI_CAMERA_V2      :=false
endif

ifeq ($(call is-android-codename-in-list,JELLY_BEAN),true)
  FEATURE_GYRO := false
else
  FEATURE_GYRO := true
endif

BUILD_SECURE_APP := false

ifeq ($(call is-board-platform,msm7627a),true)
  VFE_VERS := vfe2x
  MSM_VERSION := 7x27A
  FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := true
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DUSE_GEMINI
  mmcamera_debug_defines += -DUSE_PREVIEW_TABLE2
  mmcamera_debug_defines += -DUSE_HFR_TABLE2
  ifeq ($(BUILD_SERVER), true)
    FEATURE_FACE_PROC := true
    FEATURE_VFE_TEST_VEC := false
  endif
else ifeq ($(call is-chipset-prefix-in-board-platform,msm7627),true)
  VFE_VERS := vfe2x
  MSM_VERSION := 7x2x
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DUSE_GEMINI
  mmcamera_debug_defines += -DUSE_PREVIEW_TABLE2
else ifeq ($(call is-chipset-in-board-platform,msm7630),true)
  VFE_VERS := vfe31
  MSM_VERSION := 7x30
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DUSE_GEMINI
  ifeq ($(BUILD_SERVER), true)
    mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
    FEATURE_ZSL := true
    FEATURE_WAVELET_DENOISE := true
    TARGET_NEON_ENABLED := true
    mmcamera_debug_defines += -DFEATURE_ZSL_SUPPORTED
    FEATURE_FACE_PROC := true
  endif
else ifeq ($(call is-board-platform,msm8660),true)
  VFE_VERS := vfe31
  MSM_VERSION := 8x60
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DUSE_GEMINI
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  FEATURE_ZSL := true
  FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := true
  mmcamera_debug_defines += -DFEATURE_ZSL_SUPPORTED
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8960),true)
  VFE_VERS := vfe32
  MSM_VERSION := 8960
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  mmcamera_debug_defines += -DUSE_GEMINI
  FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := true
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8974),true)
  VFE_VERS := vfe40
  MSM_VERSION := 8974
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
  BUILD_SECURE_APP := true
else ifeq ($(call is-board-platform,apq8084),true)
  VFE_VERS := vfe44
  MSM_VERSION := 8084
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
  BUILD_SECURE_APP := true
else ifeq ($(call is-board-platform,msm8992),true)
  VFE_VERS := vfe44
  MSM_VERSION := 8992
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8994),true)
  VFE_VERS := vfe46
  MSM_VERSION := 8994
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
  BUILD_SECURE_APP := true
else ifeq ($(call is-board-platform,msm8996),true)
  VFE_VERS := vfe47
  MSM_VERSION := 8996
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
  BUILD_SECURE_APP := true
else ifeq ($(call is-board-platform,msm8916),true)
  VFE_VERS := vfe40
  MSM_VERSION := 8916
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8952),true)
  VFE_VERS := vfe40
  MSM_VERSION := 8952
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8937),true)
  VFE_VERS := vfe40
  MSM_VERSION := 8952
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8226),true)
  VFE_VERS := vfe40
  MSM_VERSION := 8226
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8610),true)
  VFE_VERS := vfe32
  MSM_VERSION := 8610
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8953),true)
  VFE_VERS := vfe40
  MSM_VERSION := 8952
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform-in-list,msm8998 apq8098_latv),true)
  VFE_VERS := vfe48
  MSM_VERSION := 8998
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
  BUILD_SECURE_APP := true
else ifeq ($(call is-board-platform-in-list,sdm660 $(TRINKET)),true)
  VFE_VERS := vfe48
  MSM_VERSION := 8998
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
  BUILD_SECURE_APP := true
else
  COMPILE_CAMERA := false
endif

ifeq ($(BUILD_SERVER), true)
  CHROMATIX_VERSION := 0208
else ifeq ($(BUILD_MM_CAMERA2), true)
  ifeq ($(OEM_CHROMATIX_0308), 1)
    CHROMATIX_VERSION := 0308E
  else ifeq ($(call is-board-platform-in-list,msm8937 msm8953 sdm660 msm8998 apq8098_latv $(TRINKET)),true)
    CHROMATIX_VERSION := 0310
  else
    CHROMATIX_VERSION := 0309
  endif
else
  ifeq ($(MSM_VERSION),7x27A)
    CHROMATIX_VERSION := 0207
  else
    CHROMATIX_VERSION := 0205
  endif
endif

ifeq ($(OEM_CHROMATIX_0308), 1)
  OEM_CHROMATIX:=true
endif

ifeq ($(OEM_CHROMATIX_0310), 1)
  OEM_CHROMATIX:=true
endif

LOCAL_PATH := $(call my-dir)

SDCLANG_COMMON_DEFS := $(LOCAL_PATH)/sdllvm-common-defs.mk
SDCLANG_FLAG_DEFS := $(LOCAL_PATH)/sdllvm-flag-defs.mk

LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(OEM_CHROMATIX), true)
  LOCAL_CHROMATIX_PATH := $(LOCAL_PATH)/../mm-camera-ext/mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
  LOCAL_EXTEN_ISP_INCLUDES := $(LOCAL_PATH)/../mm-camera-ext/mm-camera2/media-controller/modules/isp2/
  LOCAL_EXTEN_PPROC_INCLUDES := $(LOCAL_PATH)/../mm-camera-ext/mm-camera2/media-controller/modules/pproc-new/cpp/
  LOCAL_EXTEN_CHROMATIX_FILE_PATH := $(LOCAL_PATH)/../mm-camera-ext/mm-camera2/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)
else
  LOCAL_CHROMATIX_PATH := $(LOCAL_PATH)/../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
  LOCAL_EXTEN_CHROMATIX_FILE_PATH := $(LOCAL_PATH)/mm-camera2/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)
endif

ifeq ($(call is-android-codename-in-list,JELLY_BEAN),true)
  mmcamera_debug_defines += -DLOGE=ALOGE
endif

ifeq ($(MM_DEBUG),true)
  mmcamera_debug_defines += -DLOG_DEBUG -DLOG_TAG=\"CameraService\"
  mmcamera_debug_cflags += -g -O0 -Werror
  mmcamera_debug_libs := liblog libutils
endif #MM_DEBUG

ifeq ($(BUILD_GCOV_TOOL),true)
  mmcamera_debug_defines += -DGCOV_TOOL
  mmcamera_debug_cflags += -ftest-coverage
  mmcamera_debug_cflags += -fprofile-arcs
  mmcamera_debug_cflags += -coverage
  mmcamera_debug_lflags := -lgcov
  mmcamera_debug_lflags += -coverage
endif #BUILD_GCOV_TOOL

# select different camera code tree for newer targets
MY_PATH := $(call my-dir)
QC_PROP_ROOT ?= $(TOP)/vendor/qcom/proprietary
SRC_CAMERA_HAL_DIR ?= $(TOP)/hardware/qcom/camera
SRC_XML_INCLUDE ?= $(TOP)/external/libxml2/include
LOCAL_HEADER_LIBRARIES := libutils_headers

ifeq ($(strip $(COMPILE_CAMERA)),true)

  ifeq ($(BUILD_SERVER), true)
    include $(MY_PATH)/apps/Android.mk
    include $(MY_PATH)/server/Android.mk
  else ifeq ($(BUILD_MM_CAMERA2), true)
    ifeq ($(BUILD_SECURE_APP), true)
      include $(MY_PATH)/apps/secure-test-app/Android.mk
    endif
    include $(MY_PATH)/mm-camera2/Android.mk
      include $(MY_PATH)/apps/hal3-test/Android.mk
  else
    include $(MY_PATH)/apps/Android.mk
    include $(MY_PATH)/targets/Android.mk
  endif
endif #COMPILE_CAMERA

# Clear SDCLANG_FLAG_DEFS after use
SDCLANG_FLAG_DEFS :=
endif
endif
