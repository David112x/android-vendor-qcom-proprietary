ifeq ($(TARGET_HW_DISK_ENCRYPTION),true)
PRODUCT_PACKAGES += vendor.qti.hardware.cryptfshw@1.0
PRODUCT_PACKAGES += vendor.qti.hardware.cryptfshw@1.0.vendor
PRODUCT_PACKAGES += vendor.qti.hardware.cryptfshw@1.0-service-qti
PRODUCT_PACKAGES += vendor.qti.hardware.cryptfshw@1.0-service-qti.rc
PRODUCT_PACKAGES += libcryptfshwcommon
PRODUCT_PACKAGES += libcryptfshwhidl
PRODUCT_PACKAGES += libcryptfshw_hidl_headers
endif

