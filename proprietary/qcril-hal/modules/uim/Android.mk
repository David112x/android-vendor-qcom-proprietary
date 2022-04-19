LOCAL_PATH := $(call my-dir)
QCRIL_DIR := ${LOCAL_PATH}/../..

include $(CLEAR_VARS)

LOCAL_CFLAGS               += -Wall -Werror $(qcril_cflags)
LOCAL_CXXFLAGS             += -std=c++17 $(qcril_cppflags)
LOCAL_CPPFLAGS             += -std=c++17 $(qcril_cppflags)
LOCAL_LDFLAGS              += $(qcril_ldflags)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE             += $(qcril_sanitize)
endif

LOCAL_SRC_FILES            += $(call all-cpp-files-under, src)
LOCAL_SRC_FILES            += $(call all-c-files-under, src)

LOCAL_MODULE               := qcrilUimModule
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_HEADER_LIBRARIES     += libril-qti-hal-qmi-headers
LOCAL_HEADER_LIBRARIES     += qtimutex-headers
LOCAL_SHARED_LIBRARIES     += qtimutex
include $(BUILD_STATIC_LIBRARY)

