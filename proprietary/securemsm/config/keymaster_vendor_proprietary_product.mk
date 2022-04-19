# Enable TDES support when KM4 is enabled
ifeq ($(ENABLE_KM_4_0), true)
PRODUCT_PROPERTY_OVERRIDES += ro.hardware.keystore_desede=true
endif

ifeq ($(ADOPTABLE_STORAGE_SUPPORTED), true)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.crypto.volume.filenames_mode = "aes-256-cts" \
    ro.crypto.allow_encrypt_override = true
endif

#PRODUCT_PACKAGES
ifeq ($(KMGK_USE_QTI_SERVICE),true)
PRODUCT_PACKAGES += android.hardware.gatekeeper@1.0-impl-qti
PRODUCT_PACKAGES += android.hardware.gatekeeper@1.0-service-qti
PRODUCT_PACKAGES += android.hardware.gatekeeper@1.0-service-qti.rc
PRODUCT_PACKAGES += android.hardware.keymaster@4.0-service-qti
PRODUCT_PACKAGES += android.hardware.keymaster@3.0-service-qti
PRODUCT_PACKAGES += android.hardware.keymaster@3.0-impl-qti

ifeq ($(ENABLE_KM_4_0), true)
PRODUCT_PACKAGES += android.hardware.keymaster@4.0-service-qti.rc

ifeq ($(ENABLE_STRONGBOX_KM), true)
PRODUCT_PACKAGES += android.hardware.keymaster@4.0-strongbox-service-qti
PRODUCT_PACKAGES += android.hardware.keymaster@4.0-strongbox-service-qti.rc
PRODUCT_COPY_FILES += frameworks/native/data/etc/android.hardware.strongbox_keystore.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.strongbox_keystore.xml
endif

else
PRODUCT_PACKAGES += android.hardware.keymaster@3.0-service-qti.rc
endif #ENABLE_KM_4_0

else
PRODUCT_PACKAGES += android.hardware.gatekeeper@1.0-impl
PRODUCT_PACKAGES += android.hardware.gatekeeper@1.0-service
PRODUCT_PACKAGES += android.hardware.keymaster@3.0-impl
PRODUCT_PACKAGES += android.hardware.keymaster@3.0-service
endif # KMGK_USE_QTI_SERVICE

ifeq ($(ENABLE_AUTH_SECRET), true)
PRODUCT_PACKAGES += android.hardware.authsecret@1.0-service-qti
PRODUCT_PACKAGES += android.hardware.authsecret@1.0-service-qti.rc
endif

PRODUCT_PACKAGES += gatekeeper.$(TARGET_BOARD_PLATFORM)
PRODUCT_PACKAGES += keystore.$(TARGET_BOARD_PLATFORM)
PRODUCT_PACKAGES += KmInstallKeybox
PRODUCT_PACKAGES += libkeymasterdeviceutils
PRODUCT_PACKAGES += libkeymasterprovision
PRODUCT_PACKAGES += libkeymasterutils
PRODUCT_PACKAGES += libqtikeymaster4
PRODUCT_PACKAGES += qti_gatekeeper_tests
PRODUCT_PACKAGES += qti_keymaster_tests
