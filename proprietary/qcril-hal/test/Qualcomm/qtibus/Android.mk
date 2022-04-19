LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE			   := $(qcril_sanitize)
endif
LOCAL_MODULE                            := QtiBusTest
LOCAL_VENDOR_MODULE                     := true
LOCAL_HEADER_LIBRARIES                  += qtibus-headers
LOCAL_HEADER_LIBRARIES                  += qtibus-headers-private
LOCAL_HEADER_LIBRARIES                  += libril-qc-settingsd-headers
LOCAL_SHARED_LIBRARIES					+= libqcrilFramework
LOCAL_SHARED_LIBRARIES					+= liblog
LOCAL_SHARED_LIBRARIES                  += qtimutex
LOCAL_STATIC_LIBRARIES					+= libgtest
LOCAL_STATIC_LIBRARIES                  += qtibus
LOCAL_SHARED_LIBRARIES                  += libsettings
LOCAL_CFLAGS                            += $(qcril_cflags)
LOCAL_CPPFLAGS                          += $(qcril_cppflags)
LOCAL_LDFLAGS                           += $(qcril_cppflags)
LOCAL_SRC_FILES							+= $(call all-cpp-files-under,QtiShmBus/src)
include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE			   := $(qcril_sanitize)
endif
LOCAL_MODULE                            := QtiBusMessengerTest
LOCAL_VENDOR_MODULE                     := true
LOCAL_HEADER_LIBRARIES                  += qtibus-headers
LOCAL_HEADER_LIBRARIES                  += qtibus-headers-private
LOCAL_HEADER_LIBRARIES                  += libril-qc-settingsd-headers
LOCAL_SHARED_LIBRARIES					+= libqcrilFramework
LOCAL_SHARED_LIBRARIES					+= liblog
LOCAL_SHARED_LIBRARIES                  += qtimutex
LOCAL_STATIC_LIBRARIES					+= libgtest
LOCAL_STATIC_LIBRARIES					+= qtibus
LOCAL_SHARED_LIBRARIES                  += libsettings
LOCAL_CFLAGS                            += $(qcril_cflags)
LOCAL_CPPFLAGS                          += $(qcril_cppflags)
LOCAL_LDFLAGS                           += $(qcril_cppflags)
LOCAL_SRC_FILES							+= $(call all-cpp-files-under,QtiBusMessenger/src)
include $(BUILD_HOST_EXECUTABLE)
