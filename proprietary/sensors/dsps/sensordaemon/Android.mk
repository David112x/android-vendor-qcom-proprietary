LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/diag/include
commonIncludes += $(TARGET_OUT_HEADERS)/qmi-framework/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(shell find $(LOCAL_PATH) -type d -name 'inc' -print )
LOCAL_C_INCLUDES += $(shell find $(LOCAL_PATH) -type d -name 'src' -print )
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../api
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libsensor1/inc

# Figure out if this getting built with the MSM kernel. If so, include
# the path to the sanitized headers in the target "out" dir.
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
ifeq (, $(filter aarch64 arm64, $(TARGET_ARCH)))
ifeq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) <= 19 ))" )))
LOCAL_CFLAGS     += -include bionic/libc/kernel/arch-arm/asm/posix_types.h
endif
endif
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE:=sensors.qti
LOCAL_CLANG := true
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libqmi_common_so
LOCAL_SHARED_LIBRARIES += libqmi_cci
LOCAL_SHARED_LIBRARIES += libqmi_encdec
LOCAL_SHARED_LIBRARIES += libqmi_csi
LOCAL_SHARED_LIBRARIES += libsensor1

LOCAL_STATIC_LIBRARIES += libsensors_lib

LOCAL_SRC_FILES += \
  $(shell find $(LOCAL_PATH)/* -name '*.c' | grep  'src/.*\.c' | sed s:^$(LOCAL_PATH)/::g )

LOCAL_CFLAGS += -D_GNU_SOURCE -DSNS_LA -std=c99
LOCAL_CFLAGS += -Wall -Wno-missing-field-initializers -Wno-maybe-uninitialized -Werror

LOCAL_MODULE_TAGS := optional

ifeq ($(call is-board-platform-in-list,msm8998 sdm660),true)
  LOCAL_CFLAGS += -DFEATURE_SNS_AON -DSNS_QMAG_DUAL_MODE_SUPPORT
endif
LOCAL_HEADER_LIBRARIES := libcutils_headers libutils_headers
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := aon_enable
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_CLASS = ETC
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/persist/sensors
LOCAL_SRC_FILES := aon_test/$(LOCAL_MODULE)
LOCAL_HEADER_LIBRARIES := libcutils_headers libutils_headers
include $(BUILD_PREBUILT)
