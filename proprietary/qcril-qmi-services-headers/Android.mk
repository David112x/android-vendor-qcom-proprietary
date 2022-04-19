LOCAL_PATH := $(call my-dir)

## Build header library
include $(CLEAR_VARS)
LOCAL_MODULE               := libril-qc-qmi-services-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)
include $(BUILD_HEADER_LIBRARY)

## Build host header library
include $(CLEAR_VARS)
LOCAL_MODULE               := libril-qc-qmi-services-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_IS_HOST_MODULE       := true
LOCAL_EXPORT_C_INCLUDE_DIRS += $(LOCAL_PATH)
include $(BUILD_HEADER_LIBRARY)
