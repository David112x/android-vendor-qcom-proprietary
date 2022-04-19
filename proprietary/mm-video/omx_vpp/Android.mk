LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#               Common definitons
# ---------------------------------------------------------------------------------
include $(BUILD_COPY_HEADERS)

libvpp-omx-def += -g -O3
libvpp-omx-def += -Werror
ifeq ($(TARGET_KERNEL_VERSION), 4.4)
libvpp-omx-def += -D_TARGET_KERNEL_VERSION_44_
endif

ifeq ($(TARGET_USES_ION),true)
libvpp-omx-def += -DUSE_ION
endif
#libvpp-omx-def += -Wno-parentheses


include $(CLEAR_VARS)

# Common Includes
libvpp-omx-inc          := $(LOCAL_PATH)/inc
libvpp-omx-inc          += $(TOP)/system/core/libion/include
libvpp-omx-inc          += $(TOP)/system/core/libion/kernel-headers
libvpp-omx-inc          += $(TOP)/hardware/qcom/media/mm-video-v4l2/vidc/common/inc
libvpp-omx-inc          += $(TOP)/hardware/qcom/media/mm-video-v4l2/vidc/vdec/inc
libvpp-omx-inc          += $(TOP)/hardware/qcom/media/mm-core/inc
libvpp-omx-inc          += $(TOP)/hardware/qcom/media/mm-core/src/common
libvpp-omx-inc          += $(TOP)/hardware/qcom/media/libc2dcolorconvert
libvpp-omx-inc          += $(TOP)/vendor/qcom/proprietary/vpp/inc/
libvpp-omx-inc          += $(TOP)/vendor/qcom/proprietary/vpp/lib/core/inc/
libvpp-omx-inc          += $(TOP)/vendor/qcom/proprietary/vpp/ipc/inc
libvpp-omx-inc          += $(TARGET_OUT_HEADERS)/qcom/display
libvpp-omx-inc          += $(TOP)/frameworks/native/include/media/openmax
libvpp-omx-inc          += $(TOP)/frameworks/native/include/media/hardware
ifeq ($(call is-platform-sdk-version-at-least,27),true) # O-MR1
libvpp-omx-inc          += $(TOP)/frameworks/native/libs/nativebase/include
endif
libvpp-omx-inc          += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

libvpp-omx-inc += $(TOP)/hardware/qcom/media/libstagefrighthw

# Common Dependencies
libvpp-omx-add-dep := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr


# ---------------------------------------------------------------------------------
#           Make the Shared library (libOmxVdec)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE                    := libOmxVpp
LOCAL_MODULE_TAGS               := optional
LOCAL_CFLAGS                    := $(libvpp-omx-def)
LOCAL_C_INCLUDES                += $(libvpp-omx-inc)
LOCAL_ADDITIONAL_DEPENDENCIES   := $(libvpp-omx-add-dep)

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := liblog libutils libbinder libcutils libdl libmm-omxcore

LOCAL_SHARED_LIBRARIES  += libqdMetaData
LOCAL_SHARED_LIBRARIES  += libvpplibrary
LOCAL_STATIC_LIBRARIES  := libOmxVidcCommon

LOCAL_HEADER_LIBRARIES := libhardware_headers media_plugin_headers

LOCAL_SRC_FILES         := src/vpp_omx_component.cpp
LOCAL_SRC_FILES         += src/vpp_buffer_manager.cpp

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti
include $(BUILD_SHARED_LIBRARY)


# ---------------------------------------------------------------------------------
#                END
# ---------------------------------------------------------------------------------
