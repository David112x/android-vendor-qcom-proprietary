32_BIT_FLAG := true

ifeq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),arm arm64))
  include $(call all-subdir-makefiles)
endif