All Telephony HALs residing in radio have been moved to a new git project:
        path:"vendor/qcom/proprietary/commonsys-intf/telephony"
        name:"platform/vendor/qcom-proprietary/telephony-commonsys-intf"

Please add new Telephony HAL changes to the above location.

Directions to move Telephony HALs
1) cd vendor/qcom/proprietary/interfaces/radio

2) Move all folders containing HAL files:
   mv vendor/qcom/proprietary/commonsys-intf/telephony/interfaces/hal/* .

3) Modify hidl-gen.sh in vendor/qcom/proprietary/interfaces/radio:
   QTI_RADIO_HAL_SRC_FOLDER=vendor/qcom/proprietary/commonsys-intf/telephony/interfaces/hal  --->
   QTI_RADIO_HAL_SRC_FOLDER=vendor/qcom/proprietary/interfaces/radio

4) Add/Modify Android.bp in vendor/qcom/proprietary/interfaces to include the following:
        Define the hidl_package_root to point to the HAL directories:
        subdirs = [
            "*"
        ]

        hidl_package_root {
            name: "vendor.qti.hardware.radio",
            path: "vendor/qcom/proprietary/interfaces/radio",
        }

5) Remove conflicting definitions:
   rm vendor/qcom/proprietary/commonsys-intf/telephony/interfaces/Android.bp
   rm -rf vendor/qcom/proprietary/commonsys-intf/telephony/interfaces/hal

6) Recompile the build

** (Optional) Run hidl-gen.sh in step 2) to create *.bp if not already present
