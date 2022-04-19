# ==============================================================================
# ---------------------------------------------------------------------------------
#           Make the libmmcamera_dbg.so
# ---------------------------------------------------------------------------------


LOCAL_PATH:= $(call my-dir)

LOCAL_MMCAMERA_PATH  := $(QC_PROP_ROOT)/mm-camera

# trace logging lib
include $(CLEAR_VARS)
LOCAL_CFLAGS  := -D_ANDROID_
LOCAL_CFLAGS += -DMEMLEAK_FLAG
LOCAL_CFLAGS += -Werror -Wunused-parameter

#************* MCT headers ************#
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/mm-camera2/includes
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/mct/debug/memleak/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../media-controller/mct/debug/fdleak/

#************* HAL headers ************#
LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/../../../../../../hardware/qcom/camera/QCamera2/stack/common


#************* Kernel headers ************#

LOCAL_HEADER_LIBRARIES := libutils_headers
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_SRC_FILES = android/camera_dbg.c
LOCAL_SRC_FILES += android/camscope_packet_type.c
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/../media-controller/mct/debug/memleak/ -name '*.cpp' | sed s:^$(LOCAL_PATH)::g )
LOCAL_SRC_FILES += $(shell find $(LOCAL_PATH)/../media-controller/mct/debug/fdleak/ -name '*.cpp' | sed s:^$(LOCAL_PATH)::g )
LOCAL_SHARED_LIBRARIES := libdl libcutils liblog
LOCAL_MODULE := libmmcamera_dbg
include $(SDCLANG_COMMON_DEFS)
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

