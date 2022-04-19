/******************************************************************************
  @file    perfHal.go
  @brief   Android performance HAL module

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

package perfHal

import (
"fmt"
"android/soong/android"
"android/soong/cc"
)

func init() {
    fmt.Sprintf("perfHal init:%s", "Init Called");
    android.RegisterModuleType("perfHaldefaults", perfHalDefaultsFactory)
}

func perfHalDefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, perfHaldefaults)
    return module
}

func perfHaldefaults(ctx android.LoadHookContext) {
    type props struct {
        Cflags []string
    }
    p := &props{}
    p.Cflags = globalDefaults(ctx)
    ctx.AppendProperties(p)
}

func envDefault(ctx android.BaseContext, key string) string {
    ret := ctx.AConfig().Getenv(key)
    return ret
}

func globalDefaults(ctx android.BaseContext) ([]string) {
    var cflags []string
    var temp string = "\""
    variant := envDefault(ctx,"TARGET_PRODUCT")
    temp = temp + variant + temp
    cflags = append(cflags, "-DTARGET_NAME="+temp)
    return cflags
}

