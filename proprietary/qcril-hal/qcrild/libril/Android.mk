# Copyright 2006 The Android Open Source Project

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES := \
    ril.cpp \
    ril_event.cpp\
    ril_service.cpp \
    ril_service_1_2.cpp \
    RilServiceModule.cpp \
    RilServiceModule_1_2.cpp

ifeq ($(QCRIL_IRADIO_1_4), true)
    LOCAL_SRC_FILES += ril_service_1_3.cpp
    LOCAL_SRC_FILES += RilServiceModule_1_3.cpp
    LOCAL_SRC_FILES += ril_service_1_4.cpp
    LOCAL_SRC_FILES += RilServiceModule_1_4.cpp
else ifeq ($(QCRIL_IRADIO_1_3), true)
    LOCAL_SRC_FILES += ril_service_1_3.cpp
    LOCAL_SRC_FILES += RilServiceModule_1_3.cpp
endif

LOCAL_HEADER_LIBRARIES := qcrild_librilutils-headers
LOCAL_HEADER_LIBRARIES += libril-qti-hal-qmi-headers qcrilDataModule_headers qtimutex-headers radioconfig-headers
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libcutils \
    libhardware_legacy \
    android.hardware.radio@1.0 \
    android.hardware.radio@1.1 \
    android.hardware.radio@1.2 \
    android.hardware.radio.deprecated@1.0 \
    android.hardware.radio.config@1.0 \
    android.hardware.radio.config@1.1 \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    qcrild_librilutils \
    libqcrilFramework \
    libsettings \
    libhwbinder \
    qtimutex

ifeq ($(QCRIL_IRADIO_1_4), true)
    LOCAL_SHARED_LIBRARIES += android.hardware.radio@1.3
    LOCAL_SHARED_LIBRARIES += android.hardware.radio@1.4
else ifeq ($(QCRIL_IRADIO_1_3), true)
    LOCAL_SHARED_LIBRARIES += android.hardware.radio@1.3
endif

ifneq ($(qcril_sanitize_diag),)
LOCAL_SANITIZE_DIAG := $(qcril_sanitize_diag)
endif

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE := $(qcril_sanitize)
endif
LOCAL_CFLAGS += -Wno-unused-parameter $(qcril_cflags)
LOCAL_CPPFLAGS += -std=c++17 $(qcril_cppflags)
LOCAL_CXXFLAGS += -std=c++17 $(qcril_cppflags)
LOCAL_LDFLAGS += $(qcril_ldflags)

ifeq ($(SIM_COUNT), 2)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM -DDSDA_RILD1
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

ifneq ($(DISABLE_RILD_OEM_HOOK), false)
    LOCAL_CFLAGS += -DOEM_HOOK_DISABLED
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include/
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/../include

LOCAL_MODULE := qcrild_libril
LOCAL_CLANG := true
LOCAL_SANITIZE += integer

include $(BUILD_STATIC_LIBRARY)

