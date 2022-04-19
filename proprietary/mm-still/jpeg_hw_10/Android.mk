ifeq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),arm arm64))
BUILD_TINY_ANDROID := false
include $(call all-subdir-makefiles)
endif
