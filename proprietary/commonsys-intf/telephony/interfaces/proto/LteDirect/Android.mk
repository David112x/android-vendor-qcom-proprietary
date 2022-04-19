#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling the source files
#*******************************************************************************/
ifneq ($(TARGET_NO_TELEPHONY), true)

LOCAL_PATH:= $(call my-dir)

# ==========================================================================
# Build Library with nano-pb generated .c/.h files from proto
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= src/LteDirectDiscovery.proto

LOCAL_STATIC_LIBRARIES += libril-protobuf-c-nano-enable_malloc

LOCAL_PROTOC_OPTIMIZE_TYPE := nanopb-c-enable_malloc

LOCAL_MODULE:= libril-qc-ltedirectdisc

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ltedirectdisc_generated_dir := $(call generated-sources-dir-for,SHARED_LIBRARIES,$(LOCAL_MODULE),,)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(ltedirectdisc_generated_dir)/proto/$(LOCAL_PATH)/src

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PROTOC_OPTIMIZE_TYPE := micro

LOCAL_MODULE := LteDirectDiscoveryLibrary-proto
LOCAL_SDK_VERSION := system_current

LOCAL_SRC_FILES += src/LteDirectDiscovery.proto
include $(BUILD_JAVA_LIBRARY)

endif # TARGET_NO_TELEPHONY
