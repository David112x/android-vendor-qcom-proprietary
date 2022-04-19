ifneq ($(AUDIO_USE_STUB_HAL), true)
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_AHAL_EXT)),true)

LOCAL_PATH := $(call my-dir)
HIDL_INTERFACE_VERSION := 1.0
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.audiohalext@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_C_INCLUDES += $(LOCAL_PATH) \
                    $(LOCAL_PATH)/.. \
                    $(LOCAL_PATH)/$(HIDL_INTERFACE_VERSION)
LOCAL_SRC_FILES := \
    $(HIDL_INTERFACE_VERSION)/AudioHalExtHidl.cpp

LOCAL_SHARED_LIBRARIES := \
        libaudioconfigstore \
        libbase \
        libcutils \
        libexpat \
        libhidlbase \
        libhidltransport \
        libhwbinder \
        libhardware \
        liblog \
        libutils \
        vendor.qti.hardware.audiohalext@1.0

include $(BUILD_SHARED_LIBRARY)

endif
endif
