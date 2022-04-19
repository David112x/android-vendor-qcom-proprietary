LOCAL_PATH := $(call my-dir)
QCRIL_DIR := ${LOCAL_PATH}/../..

## Build header library
include $(CLEAR_VARS)
LOCAL_MODULE               := libril-qc-hal-android-ims-radio-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)/src
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)

LOCAL_CFLAGS               += -Wall -Werror $(qcril_cflags)
LOCAL_CXXFLAGS             += -std=c++17 $(qcril_cppflags)
LOCAL_CPPFLAGS             += -std=c++17 $(qcril_cppflags)
LOCAL_SRC_FILES            += $(call all-cpp-files-under, src)

LOCAL_MODULE               := qcrilAndroidImsRadio
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_HEADER_LIBRARIES     += libril-qti-hal-qmi-headers
LOCAL_SHARED_LIBRARIES     += liblog
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.ims@1.0
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.ims@1.1
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.ims@1.2
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.ims@1.3
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.ims@1.4
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.ims@1.5
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.ims@1.6

include $(BUILD_STATIC_LIBRARY)
