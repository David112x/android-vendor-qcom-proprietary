ifneq ($(strip $(AUDIO_FEATURE_QSSI_COMPLIANCE)), true)
ifneq ($(AUDIO_USE_STUB_HAL), true)
include $(call all-subdir-makefiles)
endif
endif
