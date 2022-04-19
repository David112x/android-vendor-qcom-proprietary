LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE						:= qtibus-headers
LOCAL_VENDOR_MODULE					:= true
LOCAL_EXPORT_C_INCLUDE_DIRS			:= $(LOCAL_PATH)/include
LOCAL_HEADER_LIBRARIES				:= libril-qc-hal-framework-headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS	:= libril-qc-hal-framework-headers
LOCAL_CFLAGS						+= $(qcril_cflags)
LOCAL_CPPFLAGS						+= $(qcril_cppflags)
LOCAL_LDFLAGS						+= $(qcril_ldflags)
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE						:= qtibus-headers
LOCAL_VENDOR_MODULE					:= true
LOCAL_EXPORT_C_INCLUDE_DIRS			:= $(LOCAL_PATH)/include
LOCAL_HEADER_LIBRARIES				:= libril-qc-hal-framework-headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS	+= libril-qc-hal-framework-headers
LOCAL_IS_HOST_MODULE				:= true
LOCAL_CFLAGS						+= $(qcril_cflags)
LOCAL_CPPFLAGS						+= $(qcril_cppflags)
LOCAL_LDFLAGS						+= $(qcril_ldflags)
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE						:= qtibus-headers-private
LOCAL_VENDOR_MODULE					:= true
LOCAL_EXPORT_C_INCLUDE_DIRS			:= $(LOCAL_PATH)/src
LOCAL_HEADER_LIBRARIES				:= libril-qc-hal-framework-headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS	:= libril-qc-hal-framework-headers
LOCAL_CFLAGS						+= $(qcril_cflags)
LOCAL_CPPFLAGS						+= $(qcril_cppflags)
LOCAL_LDFLAGS						+= $(qcril_ldflags)
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE						:= qtibus-headers-private
LOCAL_VENDOR_MODULE					:= true
LOCAL_EXPORT_C_INCLUDE_DIRS			:= $(LOCAL_PATH)/src
LOCAL_HEADER_LIBRARIES				:= libril-qc-hal-framework-headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS	+= libril-qc-hal-framework-headers
LOCAL_IS_HOST_MODULE				:= true
LOCAL_CFLAGS						+= $(qcril_cflags)
LOCAL_CPPFLAGS						+= $(qcril_cppflags)
LOCAL_LDFLAGS						+= $(qcril_ldflags)
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE := $(qcril_sanitize)
endif

LOCAL_MODULE						:= qtibus
LOCAL_VENDOR_MODULE					:= true
LOCAL_MODULE_OWNER					:= qti
LOCAL_PROPRIETARY_MODULE			:= true
LOCAL_SRC_FILES						:= $(call all-cpp-files-under, src)
LOCAL_HEADER_LIBRARIES				:= qtibus-headers qtibus-headers-private
LOCAL_CFLAGS						+= $(qcril_cflags)
LOCAL_CPPFLAGS						+= $(qcril_cppflags)
LOCAL_LDFLAGS						+= $(qcril_ldflags)
LOCAL_HEADER_LIBRARIES				+= qtimutex-headers
LOCAL_HEADER_LIBRARIES				+= libril-qc-settingsd-headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS	+= qtimutex-headers
LOCAL_SHARED_LIBRARIES				+= qtimutex
LOCAL_SHARED_LIBRARIES				+= libqcrilFramework
LOCAL_SHARED_LIBRARIES				+= libsettings
LOCAL_SHARED_LIBRARIES				+= liblog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE := $(qcril_sanitize)
endif

LOCAL_MODULE						:= qtibus
LOCAL_VENDOR_MODULE					:= true
LOCAL_MODULE_OWNER					:= qti
LOCAL_PROPRIETARY_MODULE			:= true
LOCAL_SRC_FILES						:= $(call all-cpp-files-under, src)
LOCAL_C_INCLUDES                    += $(LOCAL_PATH)/src
LOCAL_HEADER_LIBRARIES				:= qtibus-headers qtibus-headers-private
LOCAL_EXPORT_HEADER_LIBRARIES       = qtibus-headers
LOCAL_CFLAGS						+= $(qcril_cflags)
LOCAL_CPPFLAGS						+= $(qcril_cppflags)
LOCAL_LDFLAGS						+= $(qcril_ldflags)
LOCAL_HEADER_LIBRARIES				+= qtimutex-headers
LOCAL_HEADER_LIBRARIES				+= libril-qc-settingsd-headers
LOCAL_EXPORT_HEADER_LIBRARY_HEADERS	+= qtimutex-headers
LOCAL_SHARED_LIBRARIES				+= libsettings
LOCAL_SHARED_LIBRARIES				+= qtimutex
LOCAL_SHARED_LIBRARIES				+= libqcrilFramework
LOCAL_SHARED_LIBRARIES				+= liblog
include $(BUILD_HOST_STATIC_LIBRARY)

