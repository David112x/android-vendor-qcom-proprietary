#======================================================================
#makefile for libmmcamera2_isp2_abcc44.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)
BOARD_PLATFORM_LIST := msm8992
BOARD_PLATFORM_LIST += msm8084
BOARD_PLATFORM_LIST += msm8952
BOARD_PLATFORM_LIST += msm8976

ifeq ($(call is-board-platform-in-list, $(BOARD_PLATFORM_LIST)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS  := -D_ANDROID_

ifeq ($(filter $(PLATFORM_SDK_VERSION),20),)
  LOCAL_CFLAGS  += -Werror
endif

LOCAL_MMCAMERA_PATH := $(LOCAL_PATH)/../../../../../../

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/server-tuning/tuning/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/


LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/common/
LOCAL_C_INCLUDES += $(LOCAL_CHROMATIX_PATH)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/hw/sub_module

LOCAL_C_INCLUDES += $(SRC_CAMERA_HAL_DIR)/QCamera2/stack/common

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/mm-camera-interface
ifeq ($(call is-board-platform-in-list,apq8084 msm8992),true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include44
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp44
else ifeq ($(call is-board-platform-in-list,msm8952 msm8976),true)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp2/module/isp41
endif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include44
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../abcc_algo
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_SRC_FILES        := module_abcc44.c \
                          abcc44.c \
                          ../abcc_algo/abcc_algo.c
LOCAL_MODULE           := libmmcamera_isp_abcc44
LOCAL_SHARED_LIBRARIES := libcutils \
                          libmmcamera_isp_sub_module \
                          libmmcamera_dbg \
                          libmmcamera2_isp_modules

ifeq ($(call is-board-platform-in-list,apq8084 msm8992),true)
LOCAL_SHARED_LIBRARIES += liboemcamera
else ifeq ($(call is-board-platform-in-list,msm8952 msm8976),true)
LOCAL_SHARED_LIBRARIES += libmmcamera2_mct
endif
ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)

endif #
endif # is-vendor-board-platform,QCOM
