#include common overlays
PRODUCT_PACKAGES += \
    FrameworksResCommon \
    BluetoothResCommon \
    CarrierConfigResCommon \
    CellBroadcastReceiverResCommon \
    SystemUIResCommon \
    TelecommResCommon \
    TelephonyResCommon \
    FrameworksResCommonQva

#include target specific overlays
OVERLAY_PATH := vendor/qcom/proprietary/resource-overlay
-include $(OVERLAY_PATH)/$(TARGET_BOARD_PLATFORM)/OverlayBoardConfig.mk
