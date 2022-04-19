LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
COMMONSYS_INTF:=vendor/qcom/proprietary/commonsys-intf/android-perf/
COMMONSYS:=vendor/qcom/proprietary/commonsys/android-perf/
VENDOR:=vendor/qcom/proprietary/android-perf/
LOCAL_SRC_FILES := \
	ttf.cpp \
	perf-util/EventDataPoolTest.cpp \
	perf-util/EventQueueTest.cpp \
	perf-hal/PerfGlueLayerTest.cpp \
	activity-trigger/ActivityTriggerTest.cpp \
	mp-ctl/ActiveRequestListTest.cpp

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

LOCAL_CFLAGS := \
	-Wall \
	-DQTI_DEBUG=0

LOCAL_MODULE := perfunittests

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)
