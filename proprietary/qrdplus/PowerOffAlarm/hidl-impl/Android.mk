ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.alarm@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_SRC_FILES := \
    Alarm.cpp \

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libutils \
    liblog \
    libcutils \
    libhardware \
    libbase \
    vendor.qti.hardware.alarm@1.0 \

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.qti.hardware.alarm@1.0-service
LOCAL_INIT_RC := vendor.qti.hardware.alarm@1.0-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_SRC_FILES := \
    service.cpp \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware \
    libhidlbase \
    libhidltransport \
    vendor.qti.hardware.alarm@1.0 \
    libhwbinder \

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS := -DARCH_ARM_32
endif

include $(BUILD_EXECUTABLE)
endif #ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)