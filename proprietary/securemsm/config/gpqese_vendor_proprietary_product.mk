ifeq ($(strip $(TARGET_USES_GPQESE)),true)
#SECURE_ELEMENT Build Rules
SECUREMSM_VENDOR += android.hardware.secure_element@1.0-impl
SECUREMSM_VENDOR += vendor.qti.secure_element@1.0-service
SECUREMSM_VENDOR += vendor.qti.secure_element@1.0-service.rc
endif

#eSEPowerManager Build Rules
SECUREMSM_VENDOR += vendor.qti.esepowermanager@1.0-impl
SECUREMSM_VENDOR += vendor.qti.esepowermanager@1.0-service
SECUREMSM_VENDOR += vendor.qti.esepowermanager@1.0-service.rc

PRODUCT_PACKAGES += $(SECUREMSM_VENDOR)
