LOCAL_PATH := $(call my-dir)
QCRIL_DIR := ${LOCAL_PATH}/../..
#
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

LOCAL_MODULE               := qcrilUimRemoteClientModule
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.uim_remote_client@1.0
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.uim_remote_client@1.1
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.uim_remote_client@1.2
LOCAL_SHARED_LIBRARIES     += qtimutex

LOCAL_REQUIRED_MODULES     += qcril_feature_def.h
LOCAL_HEADER_LIBRARIES     += libril-qti-hal-qmi-headers \
                              qtimutex-headers

include $(BUILD_STATIC_LIBRARY)
