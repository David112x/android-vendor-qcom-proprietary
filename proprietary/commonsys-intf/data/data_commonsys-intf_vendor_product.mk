

IMS_RCS_VND_HAL := com.qualcomm.qti.imscmservice@1.0.vendor
IMS_RCS_VND_HAL += com.qualcomm.qti.imscmservice@2.0.vendor
IMS_RCS_VND_HAL += com.qualcomm.qti.uceservice@2.0.vendor
IMS_RCS_VND_HAL += com.qualcomm.qti.imscmservice@2.1.vendor
IMS_RCS_VND_HAL += vendor.qti.ims.callinfo@1.0.vendor
IMS_RCS_VND_HAL += com.qualcomm.qti.imscmservice@2.2.vendor
IMS_RCS_VND_HAL += com.qualcomm.qti.uceservice@2.1.vendor


CNE_VND_HAL  := vendor.qti.hardware.data.dynamicdds@1.0.vendor
CNE_VND_HAL  += vendor.qti.ims.rcsconfig@1.0.vendor
CNE_VND_HAL  += vendor.qti.ims.rcsconfig@1.1.vendor
CNE_VND_HAL  += vendor.qti.data.slm@1.0.vendor
CNE_VND_HAL  += vendor.qti.latency@2.0.vendor
CNE_VND_HAL  += vendor.qti.hardware.data.latency@1.0.vendor

QCRIL_DATA_VND_HAL := vendor.qti.hardware.data.connection@1.0.vendor
QCRIL_DATA_VND_HAL := vendor.qti.hardware.data.connection@1.1.vendor


PRODUCT_PACKAGES += $(IMS_RCS_VND_HAL)
PRODUCT_PACKAGES += $(CNE_VND_HAL)
PRODUCT_PACKAGES += $(QCRIL_DATA_VND_HAL)
