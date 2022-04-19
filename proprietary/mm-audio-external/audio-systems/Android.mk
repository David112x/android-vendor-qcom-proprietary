ifneq ($(AUDIO_USE_STUB_HAL), true)
AUDIO_EXT_SYS_ROOT := $(call my-dir)
include $(call all-subdir-makefiles)
endif
