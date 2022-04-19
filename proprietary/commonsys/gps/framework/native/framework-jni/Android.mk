#=#====#====#====#====#====#====#====#====#====#====#====#====#====#====#====#
#
#        Location Service module - jni
#
# GENERAL DESCRIPTION
#   JNI location service module makefile
#
#=============================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := liblocationservice_jni
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := src/LocationService_jni.cpp
LOCAL_SRC_FILES += src/RilInfoMonitor_jni.cpp
LOCAL_SRC_FILES += src/utils_jni.cpp
LOCAL_SRC_FILES += src/Subscription_jni.cpp
LOCAL_SRC_FILES += src/IzatProvider_jni.cpp
LOCAL_SRC_FILES += src/OsNpGlue_jni.cpp
LOCAL_SRC_FILES += src/FlpServiceProvider_jni.cpp
LOCAL_SRC_FILES += src/GeofenceServiceProvider_jni.cpp
LOCAL_SRC_FILES += src/DebugReportService_jni.cpp
LOCAL_SRC_FILES += src/WiFiDBReceiver_jni.cpp
LOCAL_SRC_FILES += src/WWANDBReceiver_jni.cpp
LOCAL_SRC_FILES += src/GnssConfigService_jni.cpp
LOCAL_SRC_FILES += src/WWANDBProvider_jni.cpp
LOCAL_SRC_FILES += src/WiFiDBProvider_jni.cpp

LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libandroid_runtime
LOCAL_SHARED_LIBRARIES += libnativehelper
LOCAL_SHARED_LIBRARIES += libhardware_legacy

LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += libhwbinder
LOCAL_SHARED_LIBRARIES += android.hardware.gnss@1.0
LOCAL_SHARED_LIBRARIES += vendor.qti.gnss@1.0
LOCAL_SHARED_LIBRARIES += vendor.qti.gnss@1.1
LOCAL_SHARED_LIBRARIES += vendor.qti.gnss@1.2
LOCAL_SHARED_LIBRARIES += vendor.qti.gnss@2.0
LOCAL_SHARED_LIBRARIES += vendor.qti.gnss@2.1
LOCAL_SHARED_LIBRARIES += vendor.qti.gnss@3.0

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_CFLAGS +=  $(GNSS_CFLAGS)

LOCAL_MODULE_OWNER := qti
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_USE_VNDK:=true
LOCAL_MODULE := liblocationservice_jni_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc
include $(BUILD_HEADER_LIBRARY)
