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

LOCAL_MODULE               := qcrilQtiRadio
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_COPY_HEADERS_TO      := qcril/modules/qti_radio
LOCAL_COPY_HEADERS         += $(call all-named-files-under,*.h,../../include/modules/qti_radio)
LOCAL_HEADER_LIBRARIES     += libril-qti-hal-qmi-headers \
                              qtimutex-headers
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.qtiradio@1.0 \
                              vendor.qti.hardware.radio.qtiradio@2.0 \
                              vendor.qti.hardware.radio.qtiradio@2.1 \
                              vendor.qti.hardware.radio.qtiradio@2.2 \
                              vendor.qti.hardware.radio.qtiradio@2.3 \
                              vendor.qti.hardware.radio.qtiradio@2.4 \
                              qtimutex
LOCAL_LDLIBS := -llog
include $(BUILD_STATIC_LIBRARY)
