LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
commonSources  :=
commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jni/inc
LOCAL_MODULE    := libsensor_reg
LOCAL_CLANG := true
LOCAL_SRC_FILES += jni/src/sensor_reg.c

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libsensor1
LOCAL_HEADER_LIBRARIES := jni_headers

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE:=false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
commonSources  :=
commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jni/inc
LOCAL_MODULE    := libsensor_reg_system
LOCAL_CLANG := true
LOCAL_SRC_FILES += jni/src/sensor_reg.c

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libsensor1_system
LOCAL_HEADER_LIBRARIES := jni_headers

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE:=false

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
commonSources  :=
commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jni/inc
LOCAL_MODULE    := libsensor_test
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CLANG := true
LOCAL_SRC_FILES += jni/src/sensor_test.c

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libsensor1
LOCAL_SHARED_LIBRARIES += libsensor_reg
LOCAL_HEADER_LIBRARIES := jni_headers

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE:=false

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/jni/inc \
                    $(TOP)/libnativehelper/include/nativehelper
LOCAL_MODULE := libAR_jni
LOCAL_CLANG := true
LOCAL_SRC_FILES := jni/src/AR_jni.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -DBOARD_PLATFORM=\"$(TARGET_BOARD_PLATFORM)\"
LOCAL_SHARED_LIBRARIES := libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libc++

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE:=false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

LOCAL_HEADER_LIBRARIES := jni_headers libhardware_headers
include $(BUILD_SHARED_LIBRARY)

ifeq ($(TARGET_SUPPORTS_WEARABLES),true)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/jni/inc
LOCAL_MODULE := libarwear_jni
LOCAL_CLANG := true
LOCAL_SRC_FILES := jni/src/arwear_jni.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -DBOARD_PLATFORM=\"$(TARGET_BOARD_PLATFORM)\"
LOCAL_SHARED_LIBRARIES := libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libc++
LOCAL_HEADER_LIBRARIES := jni_headers

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE:=false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
endif

include $(CLEAR_VARS)
commonSources  :=
commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jni/inc
LOCAL_MODULE    := libsensor_user_cal
LOCAL_CLANG := true
LOCAL_SRC_FILES += jni/src/sensor_user_cal.c
LOCAL_SRC_FILES += jni/src/sensor_user_cal_algo.c

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libsensor1
LOCAL_SHARED_LIBRARIES += libsensor_reg
LOCAL_HEADER_LIBRARIES := jni_headers

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE:=false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)



include $(CLEAR_VARS)
commonSources  :=
commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jni/inc
LOCAL_MODULE    := libsensor_thresh
LOCAL_SRC_FILES += jni/src/sensor_thresh.c

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libsensor1
LOCAL_HEADER_LIBRARIES := jni_headers

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE:=false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


ifeq ($(call is-board-platform-in-list,msm8998),true)
include $(CLEAR_VARS)
commonSources  :=
commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc
commonIncludes += $(TARGET_OUT_HEADERS)/diag/include
commonIncludes += $(TARGET_OUT_HEADERS)/qmi-framework/inc

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jni/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../adsprpc/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sns_low_lat/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libsensor1/inc/

LOCAL_MODULE    := libsensor_low_lat
LOCAL_SRC_FILES += jni/src/sensor_low_lat.c
LOCAL_SRC_FILES += jni/src/rpcmem_android.c

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libsdsprpc
LOCAL_SHARED_LIBRARIES += libdl libc
LOCAL_SHARED_LIBRARIES += libsns_low_lat_stream_stub
LOCAL_HEADER_LIBRARIES := jni_headers

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE:=false

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
commonSources  :=
commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc
commonIncludes += $(TARGET_OUT_HEADERS)/diag/include
commonIncludes += $(TARGET_OUT_HEADERS)/qmi-framework/inc

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jni/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../adsprpc/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sns_low_lat/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libsensor1/inc/

LOCAL_MODULE    := libsensor_low_lat_system
LOCAL_SRC_FILES += jni/src/sensor_low_lat.c
LOCAL_SRC_FILES += jni/src/rpcmem_android.c

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdiag_system
LOCAL_SHARED_LIBRARIES += libsdsprpc_system
LOCAL_SHARED_LIBRARIES += libdl libc
LOCAL_SHARED_LIBRARIES += libsns_low_lat_stream_stub_system

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_PRELINK_MODULE:=false
LOCAL_INSTALLED_MODULE_STEM := libsensor_low_lat.so

include $(BUILD_SHARED_LIBRARY)
endif
