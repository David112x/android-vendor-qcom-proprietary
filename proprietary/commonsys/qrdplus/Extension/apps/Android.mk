# Disable include all subdir makefiles, If any one module can normal compile, please include module makefile in here.
#include $(call all-subdir-makefiles)

BASE_PATH := $(call my-dir)

include $(BASE_PATH)/PowerOffAlarm/Android.mk
include $(BASE_PATH)/SimContacts/Android.mk
include $(BASE_PATH)/PerformanceMode/Android.mk
include $(BASE_PATH)/QColor/Android.mk
