LOCAL_PATH := $(call my-dir)
QCRIL_DIR := ${LOCAL_PATH}/..

## Build header library
include $(CLEAR_VARS)
LOCAL_MODULE               := libril-qc-hal-framework-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_EXPORT_C_INCLUDE_DIRS += $(QCRIL_DIR)/include
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE               := libril-qc-hal-framework-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_EXPORT_C_INCLUDE_DIRS += $(QCRIL_DIR)/include
LOCAL_IS_HOST_MODULE		:= true
LOCAL_HEADER_LIBRARIES      += qtimutex-headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS += qtimutex-headers
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE			   := $(qcril_sanitize)
endif
LOCAL_CFLAGS               += -Wall -Werror -DRIL_SHLIB $(qcril_cflags)
LOCAL_CXXFLAGS             += -std=c++17
LOCAL_SRC_FILES            += $(call all-cpp-files-under, src)
LOCAL_SRC_FILES            += $(call all-c-files-under, src)
LOCAL_SRC_FILES            += $(call all-cc-files-under, src)

LOCAL_MODULE               := libqcrilFramework
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_HEADER_LIBRARIES     += libril-qti-hal-qmi-headers \
                              libcutils_headers \
                              libutils_headers \
                              qtimutex-headers

LOCAL_EXPORT_HEADER_LIBRARY_HEADERS += qtimutex-headers

LOCAL_SHARED_LIBRARIES += libdiag \
     liblog \
     qtimutex \

LOCAL_SHARED_LIBRARIES += libsettings

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_CFLAGS               += -Wall -Werror -DRIL_SHLIB $(qcril_cflags)
LOCAL_CXXFLAGS             += -std=c++17
LOCAL_SRC_FILES            += $(call all-cpp-files-under, src)
LOCAL_SRC_FILES            += $(call all-c-files-under, src)
LOCAL_SRC_FILES            += $(call all-cc-files-under, src)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TARGET_OUT_HEADERS)/common/inc/
LOCAL_C_INCLUDES		   += $(TARGET_OUT_HEADERS)/common/inc/

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE			   := $(qcril_sanitize)
endif

LOCAL_MODULE               := libqcrilFramework
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_HEADER_LIBRARIES     += libril-qc-hal-framework-headers libril-qc-settingsd-headers libcutils_headers libutils_headers qtimutex-headers

LOCAL_SHARED_LIBRARIES += liblog

LOCAL_SHARED_LIBRARIES += libsettings qtimutex

include $(BUILD_HOST_SHARED_LIBRARY)
