LOCAL_PATH:= $(call my-dir)

ifeq ($(GENERIC_ODM_IMAGE),true)
include $(CLEAR_VARS)
LOCAL_MODULE := init.pixel.vendor.qti.hardware.perf@2.0-service.rc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT_ODM_ETC)/init
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := vendor.qti.hardware.perf@2.0-service
LOCAL_INIT_RC := vendor.qti.hardware.perf@2.0-service.rc

LOCAL_SRC_FILES := \
        service.cpp Perf.cpp

LOCAL_C_INCLUDES := \
        external/libxml2/include \
        external/icu/icu4c/source/common \
        external/icu/libandroidicu/include \
        vendor/qcom/proprietary/commonsys-intf/android-perf/perf-util \
        vendor/qcom/proprietary/android-perf/mp-ctl

LOCAL_HEADER_LIBRARIES := libcutils_headers

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
        libdl \
        libbase \
        libutils \
        libhardware_legacy \
        libhardware \
        libhwbinder \
        libhidlbase \
        libhidltransport \
        vendor.qti.hardware.perf@2.0 \
        libperfconfig \
        libperfgluelayer

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -DENABLE_BINDER_BUFFER_TUNING_FOR_32_BIT
endif

include $(BUILD_EXECUTABLE)
