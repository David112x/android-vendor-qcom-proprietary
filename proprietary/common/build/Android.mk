
LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_META_PATH),)

#TARGET_META_PATH check
ifeq (,$(wildcard $(TARGET_META_PATH)/contents.xml))
$(error "TARGET_META_PATH value is invalid. Please check")
endif

#meta_cli check
ifeq (,$(wildcard $(TARGET_META_PATH)/common/build/app/meta_cli.py))
$(error "meta_cli not present. Please check")
endif


# TARGET_STORAGE_TYPE
ifeq ($(TARGET_STORAGE_TYPE),)

ifneq ($(filter sdm845,$(TARGET_BOARD_PLATFORM)),)
TARGET_STORAGE_TYPE := ufs
endif

ifneq ($(filter msmnile,$(TARGET_BOARD_PLATFORM)),)
TARGET_STORAGE_TYPE := ufs
endif


ifneq ($(filter sdm660 msm8937 sdm710,$(TARGET_BOARD_PLATFORM)),)
TARGET_STORAGE_TYPE := emmc
endif

endif #stoage type

# make variables
MAKE_IMAGE_TOOL := $(HOST_OUT_EXECUTABLES)/meta_image
META_TARGET := $(LOCAL_PATH)/metainfo.txt
BOOTLOADER_IMAGE := $(PRODUCT_OUT)/bootloader.img
RADIO_IMAGE := $(PRODUCT_OUT)/radio.img

#function to generate meta image
define build-meta-image
  @echo "target meta image generation"
  $(hide) python $(QC_PROP_ROOT)/common/build/make_meta_image.py --meta_path="$(TARGET_META_PATH)" --dest_path="$(PRODUCT_OUT)" --meta_tool="$(MAKE_IMAGE_TOOL)" --storage_type="$(TARGET_STORAGE_TYPE)" --img_version="$(TARGET_IMAGE_VERSION)"
endef


#meta image dependencies
META_DEPS := $(MAKE_IMAGE_TOOL) gensecimage_target $(INSTALLED_PERSISTIMAGE_TARGET)

ifneq ($(filter msm8937,$(TARGET_BOARD_PLATFORM)),)
META_DEPS += $(PRODUCT_OUT)/mdtp.img
endif

#radio image
$(RADIO_IMAGE): $(META_DEPS)
	$(hide) $(call build-meta-image)

#booloader image
$(BOOTLOADER_IMAGE) : $(RADIO_IMAGE)

#goal for image generation
$(META_TARGET): $(BOOTLOADER_IMAGE)

ALL_DEFAULT_INSTALLED_MODULES += $(META_TARGET)
ALL_MODULES.$(LOCAL_MODULE).INSTALLED += $(META_TARGET)


#phony way
.PHONY:metacore
metacore: $(BOOTLOADER_IMAGE)

endif #TARGET_META_PATH


#add bootloader.img, radio.img to RADIO_TARGET to dist
ifneq (, $(wildcard $(PRODUCT_OUT)/bootloader.img))
INSTALLED_RADIOIMAGE_TARGET += $(PRODUCT_OUT)/bootloader.img
endif

ifneq (, $(wildcard $(PRODUCT_OUT)/radio.img))
INSTALLED_RADIOIMAGE_TARGET += $(PRODUCT_OUT)/radio.img
endif

include $(call all-subdir-makefiles)
