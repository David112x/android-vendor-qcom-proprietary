LOCAL_PATH := $(call my-dir)

## Build header libraries
include $(CLEAR_VARS)
LOCAL_MODULE               := qtimutex-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_EXPORT_C_INCLUDE_DIRS:= $(LOCAL_PATH)/include
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE               := qtimutex-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_EXPORT_C_INCLUDE_DIRS:= $(LOCAL_PATH)/include
LOCAL_IS_HOST_MODULE       := true
include $(BUILD_HEADER_LIBRARY)

## Build shared libraries
include $(CLEAR_VARS)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE			   := $(qcril_sanitize)
endif

LOCAL_MODULE               := qtimutex
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_SRC_FILES            := $(call all-cpp-files-under, src)
LOCAL_C_INCLUDES           := $(LOCAL_PATH)/include
LOCAL_HEADER_LIBRARIES     := qtimutex-headers
LOCAL_SHARED_LIBRARIES     := libcutils
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS += qtimutex-headers
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE			   := $(qcril_sanitize)
endif

LOCAL_MODULE               := qtimutex
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_CPPFLAGS             := -DQTIMUTEX_HOST_BUILD $(qcril_cflags)
LOCAL_SRC_FILES            := $(call all-cpp-files-under, src)
LOCAL_C_INCLUDES           := $(LOCAL_PATH)/include
LOCAL_HEADER_LIBRARIES     := qtimutex-headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS += qtimutex-headers
include $(BUILD_HOST_SHARED_LIBRARY)
