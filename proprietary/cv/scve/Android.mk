TARGET_SCVE_DISABLED := true

ifneq ($(TARGET_SCVE_DISABLED),true)
   include $(call all-subdir-makefiles)
endif
