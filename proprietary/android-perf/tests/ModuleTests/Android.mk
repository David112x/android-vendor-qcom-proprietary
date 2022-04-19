LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
COMMONSYS_INTF:=vendor/qcom/proprietary/commonsys-intf/android-perf/
COMMONSYS:=vendor/qcom/proprietary/commonsys/android-perf/
VENDOR:=vendor/qcom/proprietary/android-perf/
LOCAL_SRC_FILES := \
	mp-ctl/perflock_native_test.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libdl \
	libutils \
	liblog \
	libqti-util \
	libperfgluelayer \
	libqti-perfd

LOCAL_C_INCLUDES := \
	$(VENDOR)/perf-hal \
	$(COMMONSYS_INTF)/perf-util \
	$(COMMONSYS_INTF)/mp-ctl \
	$(VENDOR)/mp-ctl \
	$(VENDOR)/tests

LOCAL_HEADER_LIBRARIES := libcutils_headers

LOCAL_CFLAGS := \
	-Wall \
	-DQTI_DEBUG=0

LOCAL_MODULE := perflock_native_test
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	mp-ctl/perflock_native_stress_test.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libdl \
	libutils \
	liblog \
	libqti-util \
	libperfgluelayer \
	libqti-perfd

LOCAL_C_INCLUDES := \
	$(VENDOR)/perf-hal \
	$(COMMONSYS_INTF)/perf-util \
	$(COMMONSYS_INTF)/mp-ctl \
	$(VENDOR)/mp-ctl \
	$(VENDOR)/tests

LOCAL_HEADER_LIBRARIES := libcutils_headers

LOCAL_CFLAGS := \
	-Wall \
	-DQTI_DEBUG=0

LOCAL_MODULE := perflock_native_stress_test
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
        mp-ctl/Boostconfig-parser.c

LOCAL_SHARED_LIBRARIES := \
        libxml2 \
        libicuuc \
        libbase \

LOCAL_C_INCLUDES := \
        external/libxml2/include \
        external/icu/icu4c/source/common/unicode/ \
        $(VENDOR)/tests

LOCAL_HEADER_LIBRARIES := libcutils_headers

LOCAL_CFLAGS := \
        -Wall \
        -DQTI_DEBUG=0

LOCAL_MODULE := boostConfigParser
LOCAL_PROPRIETARY_MODULE := true
#FIXME::: to enable boostConfigParser executable include $(BUILD_EXECUTABLE)

#Perf-lock test script
include $(CLEAR_VARS)
LOCAL_MODULE := perflock-test.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := mp-ctl/perflock-test.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/nativetest/perflock_native_test/
include $(BUILD_PREBUILT)
