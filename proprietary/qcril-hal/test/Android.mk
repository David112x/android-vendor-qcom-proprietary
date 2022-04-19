LOCAL_DIR := $(call my-dir)
qcril_cflags := -O0 -g
qcril_cppflags := -O0 -g
include $(call all-makefiles-under,$(LOCAL_DIR))
