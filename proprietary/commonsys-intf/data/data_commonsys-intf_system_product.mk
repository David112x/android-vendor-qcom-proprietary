



IMS_RCS_HAL := com.qualcomm.qti.imscmservice@1.0
IMS_RCS_HAL += com.qualcomm.qti.imscmservice@1.0_vendor
IMS_RCS_HAL += com.qualcomm.qti.imscmservice@1.0-java
IMS_RCS_HAL += com.qualcomm.qti.imscmservice.xml
IMS_RCS_HAL += com.qualcomm.qti.imscmservice@2.0
IMS_RCS_HAL += com.qualcomm.qti.imscmservice-V2.0-java
IMS_RCS_HAL += com.qualcomm.qti.imscmservice-V2.0-java.xml
IMS_RCS_HAL += com.qualcomm.qti.uceservice@2.0
IMS_RCS_HAL += com.qualcomm.qti.uceservice-V2.0-java.xml
IMS_RCS_HAL += com.qualcomm.qti.uceservice-V2.0-java
IMS_RCS_HAL += com.qualcomm.qti.imscmservice@2.1
IMS_RCS_HAL += com.qualcomm.qti.imscmservice-V2.1-java
IMS_RCS_HAL += com.qualcomm.qti.imscmservice-V2.1-java.xml
IMS_RCS_HAL += vendor.qti.ims.callinfo@1.0
IMS_RCS_HAL += vendor.qti.ims.callinfo-V1.0-java
IMS_RCS_HAL += com.qualcomm.qti.imscmservice@2.2
IMS_RCS_HAL += com.qualcomm.qti.imscmservice-V2.2-java
IMS_RCS_HAL += com.qualcomm.qti.imscmservice-V2.2-java.xml
IMS_RCS_HAL += com.qualcomm.qti.uceservice@2.1
IMS_RCS_HAL += com.qualcomm.qti.uceservice-V2.1-java.xml
IMS_RCS_HAL += com.qualcomm.qti.uceservice-V2.1-java

#CNE server is required as dependency on IFactory
CNE_HAL := vendor.qti.hardware.data.dynamicdds@1.0
CNE_HAL += vendor.qti.ims.rcsconfig@1.0
CNE_HAL += vendor.qti.ims.rcsconfig@1.1
CNE_HAL += vendor.qti.latency@2.0
CNE_HAL += vendor.qti.data.slm@1.0
CNE_HAL += vendor.qti.hardware.data.dynamicdds-V1.0-java
CNE_HAL += vendor.qti.ims.rcsconfig-V1.0-java
CNE_HAL += vendor.qti.ims.rcsconfig-V1.1-java
CNE_HAL += vendor.qti.latency-V2.0-java
CNE_HAL += vendor.qti.data.slm-V1.0-java

QCRIL_DATA_HAL := vendor.qti.hardware.data.connection@1.0
QCRIL_DATA_HAL += vendor.qti.hardware.data.connection-V1.0-java
QCRIL_DATA_HAL += vendor.qti.hardware.data.connection-V1.0-java.xml
QCRIL_DATA_HAL := vendor.qti.hardware.data.connection@1.1
QCRIL_DATA_HAL += vendor.qti.hardware.data.connection-V1.1-java
QCRIL_DATA_HAL += vendor.qti.hardware.data.connection-V1.1-java.xml
QCRIL_DATA_HAL += vendor.qti.hardware.data.iwlan@1.0
QCRIL_DATA_HAL += vendor.qti.hardware.data.iwlan-V1.0-java



PRODUCT_PACKAGES += $(IMS_RCS_HAL)
PRODUCT_PACKAGES += $(CNE_HAL)
PRODUCT_PACKAGES += $(QCRIL_DATA_HAL)
