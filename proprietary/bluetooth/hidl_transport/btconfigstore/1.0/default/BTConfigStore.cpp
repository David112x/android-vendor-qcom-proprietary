/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "BTConfigStore.h"
#include <cutils/properties.h>
#include <string.h>
#define LOG_TAG "vendor.qti.hardware.btconfigstore@1.0-BTConfigStore"

namespace vendor {
namespace qti {
namespace hardware {
namespace btconfigstore {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::qti::hardware::btconfigstore::V1_0::IBTConfigStore follow.
Return<void> BTConfigStore::getVendorProperties(const uint32_t vPropType,
                                                    getVendorProperties_cb _hidl_cb) {
  int ret = 0;
  char soc_name[PROPERTY_VALUE_MAX];
  char splita2dp[PROPERTY_VALUE_MAX];
  char a2dp_offload_cap[PROPERTY_VALUE_MAX];
  char wipower_support[PROPERTY_VALUE_MAX];
  char aac_frame_ctl[PROPERTY_VALUE_MAX];
  char swb_enabled[PROPERTY_VALUE_MAX];
  char swbpm_enabled[PROPERTY_VALUE_MAX];
  char a2dp_multicast[PROPERTY_VALUE_MAX];
  char twsp_state_support[PROPERTY_VALUE_MAX];
  Result result = Result::SUCCESS;
  hidl_vec<VendorProperty> vProp;

  switch (vPropType) {
    case BT_PROP_ALL :
      ret = property_get("vendor.qcom.bluetooth.soc", soc_name, "pronto");
      ALOGI("%s:: vendor.qcom.bluetooth.soc prop: %s, ret: %d",__func__,
          soc_name, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      ret = property_get("persist.vendor.qcom.bluetooth.enable.splita2dp",
          splita2dp, "false");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.enable.splita2dp: %s, ret: %d",
          __func__, splita2dp, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      ret = property_get("persist.vendor.qcom.bluetooth.a2dp_offload_cap",
          a2dp_offload_cap, "");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.a2dp_offload_cap: %s, ret: %d",
          __func__, a2dp_offload_cap, ret);

      if (ret == 0) {
        ALOGI("%s: a2dp_offload_cap string is empty", __func__);
      }

      ret = property_get("ro.vendor.bluetooth.wipower", wipower_support,
          "false");
      ALOGI("%s:: ro.vendor.bluetooth.wipower: %s, ret: %d", __func__,
          wipower_support, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      ret = property_get("persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled",
          aac_frame_ctl, "false");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled: %s, ret: %d",
          __func__, aac_frame_ctl, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      ret = property_get("persist.vendor.qcom.bluetooth.enable.swb",
          swb_enabled, "true");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.enable.swb: %s, ret: %d",
          __func__, swb_enabled, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      ret = property_get("persist.vendor.qcom.bluetooth.enable.swbpm",
          swbpm_enabled, "true");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.enable.swbpm: %s, ret: %d",
          __func__, swbpm_enabled, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      ret = property_get("persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled",
          a2dp_multicast, "false");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled: %s, ret: %d",
          __func__, a2dp_multicast, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      ret = property_get("persist.vendor.qcom.bluetooth.twsp_state.enabled",
          twsp_state_support, "false");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.twsp_state.enabled: %s, ret: %d",
          __func__, twsp_state_support, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_SOC_TYPE, soc_name},
        {BT_PROP_SPILT_A2DP, splita2dp},
        {BT_PROP_A2DP_OFFLOAD_CAP, a2dp_offload_cap},
        {BT_PROP_WIPOWER, wipower_support},
        {BT_PROP_AAC_FRAME_CTL, aac_frame_ctl},
        {BT_PROP_SWB_ENABLE, swb_enabled},
        {BT_PROP_SWBPM_ENABLE, swbpm_enabled},
        {BT_PROP_A2DP_MCAST_TEST, a2dp_multicast},
        {BT_PROP_TWSP_STATE, twsp_state_support},
      };

      break;

    case BT_PROP_SWB_ENABLE :
      ret = property_get("persist.vendor.qcom.bluetooth.enable.swb",
          swb_enabled, "true");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.enable.swb: %s, ret: %d",
          __func__, swb_enabled, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_SWB_ENABLE, swb_enabled},
      };

      break;

    case BT_PROP_SWBPM_ENABLE :
      ret = property_get("persist.vendor.qcom.bluetooth.enable.swbpm",
          swbpm_enabled, "true");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.enable.swbpm: %s, ret: %d",
          __func__, swbpm_enabled, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_SWBPM_ENABLE, swbpm_enabled},
      };

      break;

    case BT_PROP_SOC_TYPE :
      ret = property_get("vendor.qcom.bluetooth.soc", soc_name, "pronto");
      ALOGI("%s:: vendor.qcom.bluetooth.soc prop: %s, ret: %d",
          __func__, soc_name, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_SOC_TYPE, soc_name},
      };

      break;

    case BT_PROP_SPILT_A2DP :
      ret = property_get("persist.vendor.qcom.bluetooth.enable.splita2dp",
          splita2dp, "false");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.enable.splita2dp: %s, ret: %d",
          __func__, splita2dp, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_SPILT_A2DP, splita2dp},
      };

      break;

    case BT_PROP_A2DP_OFFLOAD_CAP :
      ret = property_get("persist.vendor.qcom.bluetooth.a2dp_offload_cap",
          a2dp_offload_cap, "");
      ALOGI("%s:persist.vendor.qcom.bluetooth.a2dp_offload_cap: %s, ret: %d",
          __func__, a2dp_offload_cap, ret);

      if (ret == 0) {
        ALOGI("%s: a2dp_offload_cap string is empty", __func__);
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_A2DP_OFFLOAD_CAP, a2dp_offload_cap},
      };

      break;

    case BT_PROP_WIPOWER:
      ret = property_get("ro.vendor.bluetooth.wipower", wipower_support,
          "false");
      ALOGI("%s:: ro.vendor.bluetooth.wipower: %s, ret: %d",
          __func__, wipower_support, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_WIPOWER, wipower_support},
      };

      break;

    case BT_PROP_AAC_FRAME_CTL:
      ret = property_get("persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled",
          aac_frame_ctl, "false");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.aac_frm_ctl.enabled: %s, ret: %d",
          __func__, aac_frame_ctl, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_AAC_FRAME_CTL, aac_frame_ctl},
      };
      break;

    case BT_PROP_A2DP_MCAST_TEST:
      ret = property_get("persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled",
          a2dp_multicast, "false");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.a2dp_mcast_test.enabled: %s, ret: %d",
          __func__, a2dp_multicast, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_A2DP_MCAST_TEST, a2dp_multicast},
      };

      break;

    case BT_PROP_TWSP_STATE:
      ret = property_get("persist.vendor.qcom.bluetooth.twsp_state.enabled",
          twsp_state_support, "false");
      ALOGI("%s:: persist.vendor.qcom.bluetooth.twsp_state.enabled: %s, ret: %d",
          __func__, twsp_state_support, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {BT_PROP_TWSP_STATE, twsp_state_support},
      };

      break;

    case FM_PROP_NOTCH_VALUE:
      char notch_value[PROPERTY_VALUE_MAX];
      ret = property_get("vendor.notch.value", notch_value, "");
      ALOGI("%s:: vendor.notch.value: %s, ret: %d",
          __func__, notch_value, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {FM_PROP_NOTCH_VALUE, notch_value},
      };

      break;

    case FM_PROP_HW_INIT:
      char fm_hw_init[PROPERTY_VALUE_MAX];
      ret = property_get("vendor.hw.fm.init", fm_hw_init, "0");
      ALOGI("%s:: vendor.hw.fm.init: %s, ret: %d",
          __func__, fm_hw_init, ret);

      if (ret == 0) {
        result = Result::FAILED;
      }

      vProp =  hidl_vec<VendorProperty>{
        {FM_PROP_HW_INIT, fm_hw_init},
      };

      break;

    default:
      ALOGI("%s:: request not handled vPropType: %d", __func__, vPropType);
      result = Result::FAILED;
      break;
  }

  _hidl_cb(result, vProp);
  return Void();
}

Return<Result> BTConfigStore::setVendorProperty(const VendorProperty& vendorProp) {

    const char *vPropValue = vendorProp.value.c_str();

    ALOGI("%s:: type: 0x%04x, value: %s", __func__, vendorProp.type, vPropValue);
    switch (vendorProp.type) {
      case FM_PROP_CTL_START:
        property_set("ctl.start", vPropValue);
        break;

      case FM_PROP_CTL_STOP:
         property_set("ctl.stop", vPropValue);
        break;

      case FM_PROP_HW_INIT:
         property_set("vendor.hw.fm.init", vPropValue);
        break;

      case FM_PROP_NOTCH_VALUE:
        property_set("vendor.notch.value", vPropValue);
        break;

      case FM_PROP_HW_MODE:
        property_set("vendor.hw.fm.mode", vPropValue);
        break;

      default:
        break;

    }
    return Result::SUCCESS;
}

Return<void> BTConfigStore::getAddOnFeatures(getAddOnFeatures_cb _hidl_cb) {
  DataHandler *data_handler = DataHandler::Get();
  features_list* flist = NULL;
  AddOnFeaturesList add_on_features_list;
  Result result = Result::SUCCESS;

  if (data_handler != nullptr) {
    flist = data_handler->GetAddOnFeatureList();
    ALOGI("%s:getBTAddOnFeatures: %p", __func__, flist);

    if (flist != NULL) {
       add_on_features_list.product_id = flist->product_id;
       add_on_features_list.rsp_version = flist->rsp_version;
       add_on_features_list.feat_mask_len = flist->feat_mask_len;
       memcpy(&add_on_features_list.features, flist->features,
          sizeof(add_on_features_list.features));
    } else {
      result = Result::FAILED;
    }
  } else {
    result = Result::FAILED;
  }
  _hidl_cb(result, add_on_features_list);

  return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IBTConfigStore* HIDL_FETCH_IBTConfigStore(const char* /*name*/) {
    return new BTConfigStore();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace btconfigstore
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
