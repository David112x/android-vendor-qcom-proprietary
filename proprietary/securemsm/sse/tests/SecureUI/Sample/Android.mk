ifneq ($(ENABLE_HYP),true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../QSEEComAPI \
                    $(LOCAL_PATH)/../../../../sse/SecureUILib \
                    $(LOCAL_PATH)/../../../SecureUI \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libutils \
        liblog \
        libQSEEComAPI \
        libsecureui_svcsock \
        libSecureUILib \
        libStDrvInt


#check display config version. must be implemented display config v 1.3 since Android P (platform sdk version 28).
ifeq ($(call is-platform-sdk-version-at-least, 28),true)
display_config_version := $(shell \
if [ -d "$(TOP)/vendor/qcom/opensource/interfaces/display/config/1.3" ];\
then echo DISPLAY_CONFIG_1_3; fi)
ifeq ($(display_config_version), DISPLAY_CONFIG_1_3)
LOCAL_CFLAGS += -DDISPLAY_CONFIG_1_3
endif
endif


LOCAL_SHARED_LIBRARIES += libhidlbase vendor.display.config@1.3

LOCAL_MODULE := secure_ui_sample_client
ifeq ($(PRODUCT_VENDOR_MOVE_ENABLED),true)
LOCAL_PROPRIETARY_MODULE := true
endif
LOCAL_SRC_FILES := secure_ui_sample_client.cpp
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(QSEECOM_CFLAGS)
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)
endif
