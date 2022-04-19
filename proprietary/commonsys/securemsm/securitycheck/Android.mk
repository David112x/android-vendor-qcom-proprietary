TARGET_DISABLE_SECURITYTEST := false
ifeq ($(call is-board-platform-in-list,msm8909 msm8916 msm8996 msm8937 msm8953 sdm660 sdm845 sdm710 qcs605 msm8998 apq8098_latv sdmshrike msmnile kona $(MSMSTEPPE) atoll lahaina $(TRINKET) bengal),true)
TARGET_DISABLE_SECURITYTEST := true
else
ifeq ($(TARGET_USES_QMAA), true)
TARGET_DISABLE_SECURITYTEST := true
endif
endif

ifeq ($(TARGET_DISABLE_SECURITYTEST), false)
LOCAL_PATH:= $(call my-dir)
ifeq "$(findstring msm8960,$(TARGET_PRODUCT))" "msm8960"
  QSEECOM_CFLAGS := -DQSEECOM_TARGET_MSM8960
endif

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
                  $(TARGET_OUT_HEADERS)/common/inc \
                  $(LOCAL_PATH)/../../securemsm-noship/drm/widevine/inc \
                  $(LOCAL_PATH)/../QSEEComAPI \
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils \
        libdl \
        libQSEEComAPI \
        libdrmfs \

OEMCRYPTO_LVL-def := -D_OEMCRYPTO_L1
LOCAL_CFLAGS    := $(OEMCRYPTO_LVL-def)
LOCAL_MODULE := SecConfigCheck
LOCAL_SRC_FILES := SecConfigCheck.c
LOCAL_MODULE_TAGS := optional
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_SHARED_LIBRARIES += $(commonSharedLibraries)
LOCAL_STATIC_LIBRARIES := liboemcrypto_L1
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
