#########################################################################
# Create Folder Structure
#########################################################################
TARGET_OUT_FIRMWARE="/vendor/firmware_mnt"
TARGET_OUT_PERSIST="/mnt/vendor/persist"

$(shell rm -rf $(TARGET_OUT_VENDOR)/rfs/)

#To be enabled when prepopulation support is needed for the read_write folder
# $(shell rm -rf  $(TARGET_OUT_DATA)/rfs/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/msm/mpss/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/msm/adsp/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/msm/slpi/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/msm/cdsp/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/mdm/mpss/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/mdm/adsp/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/mdm/slpi/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/mdm/cdsp/)

#########################################################################
# MSM Folders
#########################################################################
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/msm/mpss/readonly/vendor)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/msm/adsp/readonly/vendor)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/msm/slpi/readonly/vendor)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/msm/cdsp/readonly/vendor)

$(shell ln -s /data/vendor/tombstones/rfs/modem $(TARGET_OUT_VENDOR)/rfs/msm/mpss/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/msm/mpss $(TARGET_OUT_VENDOR)/rfs/msm/mpss/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/msm/mpss/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/msm/mpss/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/msm/mpss/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/msm/mpss/readonly/vendor/firmware)

$(shell ln -s /data/vendor/tombstones/rfs/lpass $(TARGET_OUT_VENDOR)/rfs/msm/adsp/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/msm/adsp $(TARGET_OUT_VENDOR)/rfs/msm/adsp/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/msm/adsp/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/msm/adsp/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/msm/adsp/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/msm/adsp/readonly/vendor/firmware)

$(shell ln -s /data/vendor/tombstones/rfs/slpi $(TARGET_OUT_VENDOR)/rfs/msm/slpi/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/msm/slpi $(TARGET_OUT_VENDOR)/rfs/msm/slpi/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/msm/slpi/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/msm/slpi/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/msm/slpi/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/msm/slpi/readonly/vendor/firmware)

$(shell ln -s /data/vendor/tombstones/rfs/cdsp $(TARGET_OUT_VENDOR)/rfs/msm/cdsp/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/msm/cdsp $(TARGET_OUT_VENDOR)/rfs/msm/cdsp/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/msm/cdsp/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/msm/cdsp/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/msm/cdsp/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/msm/cdsp/readonly/vendor/firmware)
#########################################################################
# MDM Folders
#########################################################################
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/mdm/mpss/readonly/vendor)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/mdm/adsp/readonly/vendor)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/mdm/tn/readonly)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/mdm/slpi/readonly)
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/mdm/cdsp/readonly)


$(shell ln -s /data/vendor/tombstones/rfs/modem $(TARGET_OUT_VENDOR)/rfs/mdm/mpss/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/mdm/mpss $(TARGET_OUT_VENDOR)/rfs/mdm/mpss/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/mpss/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/mpss/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/mdm/mpss/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/mdm/mpss/readonly/vendor/firmware)

$(shell ln -s /data/vendor/tombstones/rfs/lpass $(TARGET_OUT_VENDOR)/rfs/mdm/adsp/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/mdm/adsp $(TARGET_OUT_VENDOR)/rfs/mdm/adsp/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/adsp/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/adsp/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/mdm/adsp/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/mdm/adsp/readonly/vendor/firmware)

$(shell ln -s /data/vendor/tombstones/rfs/slpi $(TARGET_OUT_VENDOR)/rfs/mdm/slpi/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/mdm/slpi $(TARGET_OUT_VENDOR)/rfs/mdm/slpi/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/slpi/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/slpi/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/mdm/slpi/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/mdm/slpi/readonly/vendor/firmware)

$(shell ln -s /data/vendor/tombstones/rfs/cdsp $(TARGET_OUT_VENDOR)/rfs/mdm/cdsp/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/mdm/cdsp $(TARGET_OUT_VENDOR)/rfs/mdm/cdsp/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/cdsp/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/cdsp/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/mdm/cdsp/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/mdm/cdsp/readonly/vendor/firmware)

$(shell ln -s /data/vendor/tombstones/rfs/tn $(TARGET_OUT_VENDOR)/rfs/mdm/tn/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/mdm/tn $(TARGET_OUT_VENDOR)/rfs/mdm/tn/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/tn/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/mdm/tn/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/mdm/tn/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/mdm/tn/readonly/vendor/firmware)

#########################################################################
# APQ Folders
#########################################################################
$(shell mkdir -p $(TARGET_OUT_VENDOR)/rfs/apq/gnss/readonly/vendor)

$(shell ln -s /data/vendor/tombstones/rfs/modem $(TARGET_OUT_VENDOR)/rfs/apq/gnss/ramdumps)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/apq/gnss $(TARGET_OUT_VENDOR)/rfs/apq/gnss/readwrite)
$(shell ln -s $(TARGET_OUT_PERSIST)/rfs/shared $(TARGET_OUT_VENDOR)/rfs/apq/gnss/shared)
$(shell ln -s $(TARGET_OUT_PERSIST)/hlos_rfs/shared $(TARGET_OUT_VENDOR)/rfs/apq/gnss/hlos)
$(shell ln -s $(TARGET_OUT_FIRMWARE) $(TARGET_OUT_VENDOR)/rfs/apq/gnss/readonly/firmware)
$(shell ln -s /vendor/firmware $(TARGET_OUT_VENDOR)/rfs/apq/gnss/readonly/vendor/firmware)

