ifneq ($(TARGET_HAS_LOW_RAM),true)
ifeq ($(TARGET_FWK_SUPPORTS_FULL_VALUEADDS),true)
include $(call all-subdir-makefiles)
endif
endif
