ifneq ($(TARGET_HAS_LOW_RAM),true)
#CapabilityConfigStore HIDL


PREBUILD_CONFIGSTORE_DATA := $(LOCAL_PATH)/prebuild_configstore.mk

CCS_HIDL = vendor.qti.hardware.capabilityconfigstore@1.0
CCS_HIDL += vendor.qti.hardware.capabilityconfigstore-V1.0-java
CCS_HIDL += vendor.qti.hardware.capabilityconfigstore@1.0-impl
CCS_HIDL += vendor.qti.hardware.capabilityconfigstore@1.0-service
CCS_HIDL += capabilityconfigstoretest
CCS_HOST += configstore_xmlparser

PRODUCT_PACKAGES += $(CCS_HIDL)
PRODUCT_HOST_PACKAGES += $(CCS_HOST)
endif
