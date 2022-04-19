LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libutils \
        liblog

LOCAL_CFLAGS := -O3

#check display config version. must be implemented display config v 1.3 since Android P (platform sdk version 28).
ifeq ($(call is-platform-sdk-version-at-least, 28),true)
display_config_version := $(shell \
if [ -d "$(TOP)/vendor/qcom/opensource/interfaces/display/config/1.3" ];\
then echo DISPLAY_CONFIG_1_3; fi)
ifeq ($(display_config_version), DISPLAY_CONFIG_1_3)
LOCAL_CFLAGS += -DDISPLAY_CONFIG_1_3
endif
endif


LOCAL_SHARED_LIBRARIES += libhidlbase libhidltransport libhidlmemory libhwbinder \
        libbase vendor.display.config@1.3 vendor.qti.hardware.trustedui@1.0

LOCAL_MODULE := TrustedUISampleTest
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := TrustedUISampleClient.cpp
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qti
include $(BUILD_NATIVE_TEST)
