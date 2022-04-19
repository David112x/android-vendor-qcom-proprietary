ifeq ($(call is-vendor-board-platform,QCOM),true)
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MMCAMERASDK_PATH := $(LOCAL_PATH)/../../mm-camerasdk
CHROMATIX_VERSION := 0309

LOCAL_COPY_HEADERS_TO := mm-camera-interface
LOCAL_COPY_HEADERS := hvx_lib.h

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_COPY_HEADERS)

################# Add constant Start ##############################
ifeq ($(call is-board-platform-in-list, msm8996 msm8998 apq8098_latv sdm660 $(TRINKET)),true)
include $(CLEAR_VARS)
LOCAL_CFLAGS := -D_ANDROID_
LOCAL_C_INCLUDES += $(LOCAL_PATH)/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERASDK_PATH)/sensor/includes/$(CHROMATIX_VERSION)/
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/camera/QCamera2/stack/common
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_CFLAGS  += -Werror

LOCAL_SRC_FILES := lib/add_constant/hvx_add_constant.c

LOCAL_SHARED_LIBRARIES := libdl libcutils liblog
LOCAL_MODULE := libmmcamera_hvx_add_constant
LOCAL_MODULE_TAGS      := optional
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

include $(BUILD_SHARED_LIBRARY)

endif # if 8996
################# Add constant End ##############################

################# grid sum Start ##############################
ifeq ($(call is-board-platform-in-list, msm8996 msm8998 apq8098_latv sdm660 $(TRINKET)),true)
include $(CLEAR_VARS)
LOCAL_CFLAGS := -D_ANDROID_
LOCAL_C_INCLUDES += $(LOCAL_PATH)/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERASDK_PATH)/sensor/includes/$(CHROMATIX_VERSION)/
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/camera/QCamera2/stack/common
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_CFLAGS  += -Werror

LOCAL_SRC_FILES := lib/grid_sum/hvx_grid_sum.c

LOCAL_SHARED_LIBRARIES := libdl libcutils liblog
LOCAL_MODULE := libmmcamera_hvx_grid_sum
LOCAL_MODULE_TAGS      := optional
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

include $(BUILD_SHARED_LIBRARY)

endif # if 8996
################# grid sum End ##############################

################# zzHDR Start ##############################
ifeq ($(call is-board-platform-in-list, msm8996),true)
include $(CLEAR_VARS)
LOCAL_CFLAGS := -D_ANDROID_
LOCAL_C_INCLUDES += $(LOCAL_PATH)/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERASDK_PATH)/sensor/includes/$(CHROMATIX_VERSION)/
LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/camera/QCamera2/stack/common
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERASDK_PATH)/hvx/lib/zzHDR
LOCAL_CFLAGS  += -Werror

LOCAL_SRC_FILES := lib/zzHDR/hvx_zzHDR.c

LOCAL_SHARED_LIBRARIES := libdl libcutils liblog
LOCAL_MODULE := libmmcamera_hvx_zzHDR
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

endif # if 8996
################# zzHDR End ##############################

endif # if QCOM
