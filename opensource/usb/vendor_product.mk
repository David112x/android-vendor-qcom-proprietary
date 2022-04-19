PRODUCT_PACKAGES += android.hardware.usb@1.1-service-qti

#
# Since this module is part of a component that is shared with Android P
# the older Soong modules are unable to understand the 'vintf_fragments'
# property if added to the cc_binary rule in Android.bp. We can't use
# LOCAL_VINTF_FRAGMENTS here either because this is mk file is not defining
# the module (i.e. LOCAL_MODULE). We only intend to ship the USB HAL on
# Android Q and newer so we can just use a manual copy rule for now.
#
PRODUCT_COPY_FILES += vendor/qcom/opensource/usb/hal/android.hardware.usb@1.1-service.xml:$(TARGET_COPY_OUT_VENDOR)/etc/vintf/manifest/android.hardware.usb@1.1-service.xml

ifeq ($(TARGET_USES_USB_GADGET_HAL), true)
  PRODUCT_COPY_FILES += vendor/qcom/opensource/usb/hal/android.hardware.usb.gadget@1.0-service.xml:$(TARGET_COPY_OUT_VENDOR)/etc/vintf/manifest/android.hardware.usb.gadget@1.0-service.xml
  PRODUCT_PROPERTY_OVERRIDES += ro.vendor.usb.use_gadget_hal=true
endif
