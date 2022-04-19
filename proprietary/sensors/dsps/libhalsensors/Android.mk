LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

commonSources  :=
commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi-framework/inc
commonIncludes += $(TARGET_OUT_HEADERS)/diag/include
commonIncludes += $(TARGET_OUT_HEADERS)/liblocationservice/lcp/inc
commonIncludes += $(TARGET_OUT_HEADERS)/gps.utils

commonCflags   := $(remote_api_defines)
commonCflags   += $(remote_api_enables)

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensortest/jni/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensordaemon/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensordaemon/common/util/mathtools/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensordaemon/ucos-ii_posix/inc
LOCAL_C_INCLUDES += hardware/libhardware/include
LOCAL_C_INCLUDES += hardware/libhardware/include/hardware
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libloc_core
ifeq ($(TARGET_SUPPORT_DIRECT_REPORT),true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sns_low_lat/inc
endif

# Figure out if this getting built with the MSM kernel. If so, include
# the path to the sanitized headers in the target "out" dir.
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

ifneq ($(call is-board-platform-in-list,sdm845 apq8098_latv),true)
ifeq ($(USE_SENSOR_MULTI_HAL),true)
LOCAL_MODULE := sensors.ssc
LOCAL_CLANG := true
else
LOCAL_MODULE := sensors.$(TARGET_DEVICE)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_CLANG := true
endif

LOCAL_SHARED_LIBRARIES += \
    libc \
    libutils \
    libcutils \
    liblog \
    libhardware \
    libsensor1 \
    libsensor_reg \
    libdl \
    libdiag \
    libpower

ifeq ($(TARGET_SUPPORT_DIRECT_REPORT),true)
LOCAL_SHARED_LIBRARIES += \
    libsns_low_lat_stream_stub
endif

ifneq ($(TARGET_SUPPORTS_WEARABLES),true)
#Enable location service for msm8998
#Inclusion of location service lib will be added once it is enabled.
#  ifeq ($(call is-board-platform-in-list,msm8998),true)
#    LOCAL_SHARED_LIBRARIES += liblocationservice
#Enable IOD sensor Feature
#    LOCAL_CFLAGS += -DFEATURE_SNS_IOD
#  endif
endif

#Find all the cpp files in the src folder and add to the LOCAL_SRC_FILES.
SRC_CPP_LIST := $(wildcard $(LOCAL_PATH)/src/*.cpp)
LOCAL_SRC_FILES +=$(SRC_CPP_LIST:$(LOCAL_PATH)/%=%)

ifeq ($(TARGET_SUPPORT_DIRECT_REPORT),true)
SRC_C_LIST := $(LOCAL_PATH)/../sns_low_lat/test/src/rpcmem_android.c
LOCAL_SRC_FILES +=$(SRC_C_LIST:$(LOCAL_PATH)/%=%)
LOCAL_CFLAGS += -DSNS_DIRECT_REPORT_SUPPORT=\"$(TARGET_SUPPORT_DIRECT_REPORT)\"
else
LOCAL_SRC_FILES := $(filter-out src/DirectChannel.cpp, $(LOCAL_SRC_FILES))
endif

LOCAL_CFLAGS += -DSNS_LA
LOCAL_CFLAGS += -Werror

ifeq ($(TARGET_SUPPORTS_WEARABLES),true)
LOCAL_CFLAGS += -DWEAR_SUPPORT=\"$(TARGET_SUPPORTS_WEARABLES)\"
endif

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

LOCAL_HEADER_LIBRARIES := jni_headers
include $(BUILD_SHARED_LIBRARY)
endif
