ifneq ($(AUDIO_USE_STUB_HAL), true)
AUDIO_OMX := $(call my-dir)
include $(call all-subdir-makefiles)
endif
