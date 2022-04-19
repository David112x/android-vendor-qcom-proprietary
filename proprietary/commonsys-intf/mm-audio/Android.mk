ifeq ($(strip $(AUDIO_FEATURE_QSSI_COMPLIANCE)), true)
include $(call all-subdir-makefiles)
endif
