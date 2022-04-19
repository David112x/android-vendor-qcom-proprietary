#=#====#====#====#====#====#====#====#====#====#====#====#====#====#====#====#
#
#        Location Service module - glue
#
# GENERAL DESCRIPTION
#   Glue location service module makefile
#
#=============================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := liblocationservice_glue
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES += src/OSFrameworkCleaner.cpp
LOCAL_SRC_FILES += src/OsNpGlue.cpp
LOCAL_SRC_FILES += src/Subscription.cpp
LOCAL_SRC_FILES += src/FrameworkActionRequest.cpp
LOCAL_SRC_FILES += src/OSFramework.cpp

LOCAL_SHARED_LIBRARIES := liblocationservice
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libloc_core
LOCAL_SHARED_LIBRARIES += libgps.utils
LOCAL_SHARED_LIBRARIES += libhardware_legacy
LOCAL_SHARED_LIBRARIES += liblbs_core
LOCAL_SHARED_LIBRARIES += libdataitems
LOCAL_SHARED_LIBRARIES += liblocation_api

LOCAL_SHARED_LIBRARIES += libutils

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libizat_core
LOCAL_HEADER_LIBRARIES := \
    libutils_headers \
    libgps.utils_headers \
    libloc_core_headers \
    liblbs_core_headers \
    liblocationservice_headers \
    izat_remote_api_headers \
    izat_remote_api_prop_headers \
    libdataitems_headers \
    libloc_pla_headers \
    liblocation_api_headers

LOCAL_CFLAGS += $(GPS_FEATURES) \
    $(GNSS_CFLAGS)

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := liblocationservice_glue_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc
include $(BUILD_HEADER_LIBRARY)
