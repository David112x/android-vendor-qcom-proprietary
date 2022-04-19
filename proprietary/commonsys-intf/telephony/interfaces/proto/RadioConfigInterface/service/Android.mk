#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling the source files
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)


# ==========================================================================

# Build Library with nano-pb generated .c/.h files from proto
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= src/radio_config_interface.proto

LOCAL_STATIC_LIBRARIES += libril-protobuf-c-nano-enable_malloc

LOCAL_PROTOC_OPTIMIZE_TYPE := nanopb-c-enable_malloc

LOCAL_MODULE:= libril-qc-radioconfig

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

radioconfig_generated_dir := $(call generated-sources-dir-for,SHARED_LIBRARIES,$(LOCAL_MODULE),,)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(radioconfig_generated_dir)/proto/$(LOCAL_PATH)/src
LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_SHARED_LIBRARY)

# ==========================================================================
ifneq ($(TARGET_HAS_LOW_RAM),true)

# Build the proto interface library
include $(CLEAR_VARS)

src_proto := src
LOCAL_SRC_FILES += src/radio_config_interface.proto

LOCAL_PROTOC_OPTIMIZE_TYPE := micro

LOCAL_MODULE := radioconfig-proto
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_STATIC_JAVA_LIBRARY)

endif
