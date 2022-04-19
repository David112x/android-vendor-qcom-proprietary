#=#====#====#====#====#====#====#====#====#====#====#====#====#====#====#====#
#
#        Location Service module - common
#
# GENERAL DESCRIPTION
#   Common location service module makefile
#
#=============================================================================
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := izat_remote_api_test
LOCAL_SANITIZE += $(GNSS_SANITIZE)
# activate the following line for debug purposes only, comment out for production
#LOCAL_SANITIZE_DIAG += $(GNSS_SANITIZE_DIAG)
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    IZatRemoteApiTest.cpp

LOCAL_SHARED_LIBRARIES := \
    libizat_client_api \
    libgps.utils \
    libc++

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(TOP)/external/libcxx/src
LOCAL_HEADER_LIBRARIES := \
    libgps.utils_headers \
    libloc_core_headers \
    izat_remote_api_headers \
    izat_remote_api_prop_headers \
    libloc_pla_headers \
    liblocation_api_headers

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS += $(GNSS_CFLAGS)
include $(BUILD_EXECUTABLE)
