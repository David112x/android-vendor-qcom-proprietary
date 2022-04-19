LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    qti-activity_trigger.cpp

LOCAL_HEADER_LIBRARIES := libcutils_headers

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libdl \
    libutils \
    liblog \
    libxml2 \
    libqti-perfd-client_system

ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS), true)
LOCAL_SHARED_LIBRARIES += libqspmsvc
LOCAL_CFLAGS += -DQVA
endif

LOCAL_CFLAGS += \
    -Wall \
    -DWHITE_LISTS_PATH=\"/system/etc\" \
    -DQC_DEBUG=0

LOCAL_C_INCLUDES := external/connectivity/stlport/stlport \
                    external/libxml2/include \
                    external/icu/icu4c/source/common \
                    vendor/qcom/proprietary/commonsys-intf/android-perf/mp-ctl \
                    vendor/qcom/proprietary/android-perf/mp-ctl \
                    $(LOCAL_PATH)/inc


LOCAL_MODULE := libqti-at
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti

include $(BUILD_SHARED_LIBRARY)

## Build header library
include $(CLEAR_VARS)
LOCAL_MODULE               := libactivity-trigger-headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc
include $(BUILD_HEADER_LIBRARY)
