LOCAL_PATH := $(call my-dir)

# Build the java code
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, java)

LOCAL_STATIC_JAVA_LIBRARIES := wigig_manager
LOCAL_JAVA_LIBRARIES := services android.hidl.base-V1.0-java  android.hidl.manager-V1.0-java  vendor.qti.hardware.wigig.supptunnel-V1.0-java vendor.qti.hardware.wigig.netperftuner-V1.0-java
LOCAL_JAVA_LIBRARIES += vendor.qti.hardware.capabilityconfigstore-V1.0-java
LOCAL_REQUIRED_MODULES := services
LOCAL_MODULE := wigig-service

include $(BUILD_JAVA_LIBRARY)
