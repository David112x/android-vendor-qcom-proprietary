#======================================================================
#makefile for libmmcamera2_stats_modules.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(call is-board-platform-in-list, msm8610),true)
  LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

  LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
    $(mmcamera_debug_defines) \
    $(mmcamera_debug_cflags) -DFEATURE_SKIP_STATS
else
  LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

  LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
    $(mmcamera_debug_defines) \
    $(mmcamera_debug_cflags)
ifeq ($(call is-board-platform-in-list,msm8916 $(TRINKET)),true)
  USE_ANDROID_GYRO := true
endif

ifeq ($(call is-board-platform-in-list,msm8917),true)
  LOCAL_CFLAGS += -DUSE_DIS_DEFAULT
endif

ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) <= 25 ))" )))
USE_SENSOR_NATIVE = true
LOCAL_CFLAGS += -DSENSOR_NATIVE_LAYER
ifeq ($(TARGET_USES_AOSP),true)
  USE_ANDROID_GYRO := true
  LOCAL_CFLAGS += -DDISABLE_IS_MODULE
endif
else
USE_SENSOR_HIDL = true
LOCAL_CFLAGS += -DSENSOR_HIDL_INTF
#LOCAL_CFLAGS += -DCAMERA_FEATURE_THIRDPARTYEIS
endif

LOCAL_CFLAGS += -DDSPS_DISABLE_WAIT_FOR_CALLBACK

ifeq ($(USE_ANDROID_GYRO),true)
 FEATURE_GYRO_DSPS := false
else
 LOCAL_CFLAGS += -DFEATURE_GYRO_DSPS
 FEATURE_GYRO_DSPS := true
endif

ifeq ($(CHROMATIX_VERSION), 0308E)
 LOCAL_CFLAGS += -DCHROMATIX_308E
endif

endif

USE_GYRO ?= true
IS_ENABLE ?= true
ifneq ($(USE_GYRO),true)
 FEATURE_GYRO_DSPS := false
 USE_ANDROID_GYRO := false
 LOCAL_CFLAGS += -DDISABLE_GYRO_MODULE
endif

ifeq ($(IS_ENABLE),false)
 LOCAL_CFLAGS += -DDISABLE_IS_MODULE
endif

ifeq ($(OEM_CHROMATIX_0308), 1)
 LOCAL_CFLAGS += -D_ENABLE_AEC_BG_STATS_ -D_OEM_CHROMATIX_0308_
endif

ifeq ($(call is-board-platform-in-list,sdm660 msm8998 apq8098_latv $(TRINKET)),true)
 LOCAL_CFLAGS += -D_ENABLE_AEC_BG_STATS_
endif

#Check if version is LMR1 and below
ifneq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) <= 22 ))" )))
 LOCAL_CFLAGS += -DUPGRADE_SENSOR_MANAGER
endif

LOCAL_CFLAGS += -Wno-undefined-bool-conversion -Wno-format
LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/chromatix-wrapper/

LOCAL_C_INCLUDES += $(LOCAL_PATH)/debug-data

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/../../mm-camerasdk/sensor/includes
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/module/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/include
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/platform/linux
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/aec
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/awb
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/test
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/asd
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/afd
ifeq ($(call is-board-platform-in-list,sdm660 msm8998 apq8098_latv msm8937 msm8953 $(TRINKET)),true)
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/af_v2
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/af_v2/af_algo_interface
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/af_v2/examples
  LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../mm-camera-lib/stats/q3a/af_v2/util
else
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/af
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/af/af_algo_interface
  LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/af/examples
endif

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/is
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../mm-camera-noship/is/sensor_lib
ifeq ($(USE_GYRO),true)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/gyro
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/gyro/dsps
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/gyro/dsps/sensor_native_hidl
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../sensors/dsps/api
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../qmi/core/lib/inc

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
LOCAL_C_INCLUDES += $(SRC_XML_INCLUDE)
LOCAL_C_INCLUDES += external/libxml2/include
LOCAL_C_INCLUDES += external/icu/icu4c/source/common

#add gyro sensor API header locations
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/sensors/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/adreno

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif
LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/../../../../../../../../hardware/qcom/camera/QCamera2/stack/common

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_HEADER_LIBRARIES += libhardware_headers

LOCAL_CFLAGS  += -Werror -Wunused-parameter

LOCAL_SRC_DIR := $(LOCAL_PATH)

CAM_SRC_FILES := $(shell find $(LOCAL_SRC_DIR) -type d \( -name gyro -o -name af -o -name af_v2 -o -name is \) -prune -o -name '*.c' | sed s:^$(LOCAL_PATH)::g)
CAM_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -type d \( -name gyro -o -name af -o -name af_v2 -o -name is \) -prune -o -name '*.cpp' | sed s:^$(LOCAL_PATH)::g)

LOCAL_SRC_FILES := $(filter %.c %.cpp,$(CAM_SRC_FILES))

LOCAL_AF_SRC_FILES_309 := $(LOCAL_PATH)/q3a/af
LOCAL_AF_SRC_FILES_310 := $(LOCAL_PATH)/q3a/af_v2
LOCAL_AF_SRC_FILES_GYRO:= $(LOCAL_PATH)/gyro
LOCAL_IS_SRC_FILES := $(LOCAL_PATH)/is

ifeq ($(call is-board-platform-in-list,sdm660 msm8998 apq8098_latv msm8953 msm8937 $(TRINKET)),true)
  LOCAL_SRC_FILES += $(shell find $(LOCAL_AF_SRC_FILES_310) -name '*.c' | sed s:^$(LOCAL_PATH)::g)
  LOCAL_SRC_FILES += $(shell find $(LOCAL_AF_SRC_FILES_310) -name '*.cpp' | sed s:^$(LOCAL_PATH)::g)
else
  LOCAL_SRC_FILES += $(shell find $(LOCAL_AF_SRC_FILES_309) -name '*.c' | sed s:^$(LOCAL_PATH)::g)
  LOCAL_SRC_FILES += $(shell find $(LOCAL_AF_SRC_FILES_309) -name '*.cpp' | sed s:^$(LOCAL_PATH)::g)
endif
ifeq ($(USE_GYRO),true)
LOCAL_SRC_FILES += $(shell find $(LOCAL_AF_SRC_FILES_GYRO) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
LOCAL_SRC_FILES += $(shell find $(LOCAL_AF_SRC_FILES_GYRO) -name '*.cpp' | sed s:^$(LOCAL_PATH)::g )
endif

ifeq ($(IS_ENABLE),true)
LOCAL_SRC_FILES += $(shell find $(LOCAL_IS_SRC_FILES) -name '*.c' | sed s:^$(LOCAL_PATH)::g )
LOCAL_SRC_FILES += $(shell find $(LOCAL_IS_SRC_FILES) -name '*.cpp' | sed s:^$(LOCAL_PATH)::g )
endif

LOCAL_MODULE           := libmmcamera2_stats_modules
include $(SDCLANG_COMMON_DEFS)

#Shared libraries
LOCAL_SHARED_LIBRARIES := libdl libcutils libmmcamera2_mct libmmcamera2_stats_lib libmmcamera2_stats_algorithm libxml2
LOCAL_SHARED_LIBRARIES += libutils libmmcamera_dbg

ifeq ($(USE_SENSOR_HIDL),true)
LOCAL_SHARED_LIBRARIES += libhidlbase libhidltransport
LOCAL_SHARED_LIBRARIES += android.frameworks.sensorservice@1.0
endif
ifeq ($(USE_SENSOR_NATIVE),true)
LOCAL_SHARED_LIBRARIES += libgui libandroid
endif

ifeq ($(IS_ENABLE),true)
LOCAL_SHARED_LIBRARIES += libmmcamera2_is
endif

LOCAL_LDFLAGS += -Wl,--wrap=malloc -Wl,--wrap=free -Wl,--wrap=realloc -Wl,--wrap=calloc
LOCAL_LDFLAGS += -Wl,--wrap=open -Wl,--wrap=close -Wl,--wrap=socket -Wl,--wrap=pipe -Wl,--wrap=mmap

ifeq ($(FEATURE_GYRO_DSPS), true)
  LOCAL_SHARED_LIBRARIES += libsensor1
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

include $(LOCAL_PATH)/external/ext_awb_build_option.mk

include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/external/ext_awb_add_prebuilt.mk

#off-target sanity compilation for linux host is disabled temporarily for any target other than 8996/8998
ifeq ($(call is-board-platform-in-list,msm8996,msm8998 apq8098_latv),true)
  include $(CLEAR_VARS)
  include $(LOCAL_PATH)/q3a/3a-tools/off-target/chromatix-wrapper/Android.mk
endif

endif # is-vendor-board-platform,QCOM
