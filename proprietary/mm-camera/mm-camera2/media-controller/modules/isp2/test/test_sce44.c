/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "sce/sce40/sce40.h"
#include "sce_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_sce_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_sce_tv_validate(void *test_input,
  void *test_output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)test_output;
  ISP_Skin_enhan_ConfigCmdType *in, *out;

  CDBG("%s:\n", __func__);
  in = (ISP_Skin_enhan_ConfigCmdType *)(mod_in->reg_dump + (ISP_SCE40_OFF/4) );
  out = (ISP_Skin_enhan_ConfigCmdType *)(mod_op->reg_dump + (ISP_SCE40_OFF/4));
  //CR Coordinates
  VALIDATE_TST_VEC(in->crcoord.vertex00, out->crcoord.vertex00,
    0, "crcoord.vertex00");

  // Validate only if the vertext order is not swapped.
  if (!((in->crcoord.vertex01 == out->crcoord.vertex02) &&
      (in->crcoord.vertex02== out->crcoord.vertex01))) {
    VALIDATE_TST_VEC(in->crcoord.vertex01, out->crcoord.vertex01,
      0, "crcoord.vertex01");
    VALIDATE_TST_VEC(in->crcoord.vertex02, out->crcoord.vertex02,
      0, "crcoord.vertex02");
  }

  VALIDATE_TST_VEC(in->crcoord.vertex10, out->crcoord.vertex10,
    0, "crcoord.vertex10");

  // Validate only if the vertext order is not swapped.
  if (!((in->crcoord.vertex11 == out->crcoord.vertex12) &&
      (in->crcoord.vertex12== out->crcoord.vertex11))) {
    VALIDATE_TST_VEC(in->crcoord.vertex11, out->crcoord.vertex11,
      0, "crcoord.vertex11");
    VALIDATE_TST_VEC(in->crcoord.vertex12, out->crcoord.vertex12,
      0, "crcoord.vertex12");
  }
  VALIDATE_TST_VEC(in->crcoord.vertex20, out->crcoord.vertex20,
    0, "crcoord.vertex20");

    // Validate only if the vertext order is not swapped.
  if (!((in->crcoord.vertex21 == out->crcoord.vertex22) &&
      (in->crcoord.vertex22== out->crcoord.vertex21))) {
    VALIDATE_TST_VEC(in->crcoord.vertex21, out->crcoord.vertex21,
      0, "crcoord.vertex21");
    VALIDATE_TST_VEC(in->crcoord.vertex22, out->crcoord.vertex22,
      0, "crcoord.vertex22");
  }
  VALIDATE_TST_VEC(in->crcoord.vertex30, out->crcoord.vertex30,
    0, "crcoord.vertex30");

    // Validate only if the vertext order is not swapped.
  if (!((in->crcoord.vertex31 == out->crcoord.vertex32) &&
      (in->crcoord.vertex32== out->crcoord.vertex31))) {
    VALIDATE_TST_VEC(in->crcoord.vertex31, out->crcoord.vertex31,
      0, "crcoord.vertex31");
    VALIDATE_TST_VEC(in->crcoord.vertex32, out->crcoord.vertex32,
      0, "crcoord.vertex32");
  }
  VALIDATE_TST_VEC(in->crcoord.vertex40, out->crcoord.vertex40,
    0, "crcoord.vertex40");

      // Validate only if the vertext order is not swapped.
  if (!((in->crcoord.vertex41 == out->crcoord.vertex42) &&
      (in->crcoord.vertex42== out->crcoord.vertex41))) {
    VALIDATE_TST_VEC(in->crcoord.vertex41, out->crcoord.vertex41,
      0, "crcoord.vertex41");
    VALIDATE_TST_VEC(in->crcoord.vertex42, out->crcoord.vertex42,
      0, "crcoord.vertex42");
  }

  /* CB Coordinates */
  VALIDATE_TST_VEC(in->cbcoord.vertex00, out->cbcoord.vertex00, 0,
    "cbcoord.vertex00");

  // Validate only if the vertext order is not swapped.
  if (!((in->cbcoord.vertex01 == out->cbcoord.vertex02) &&
      (in->cbcoord.vertex02== out->cbcoord.vertex01))) {
    VALIDATE_TST_VEC(in->cbcoord.vertex01, out->cbcoord.vertex01, 0,
      "cbcoord.vertex01");
    VALIDATE_TST_VEC(in->cbcoord.vertex02, out->cbcoord.vertex02, 0,
      "cbcoord.vertex02");
  }
  VALIDATE_TST_VEC(in->cbcoord.vertex10, out->cbcoord.vertex10, 0,
    "cbcoord.vertex10");

  // Validate only if the vertext order is not swapped.
  if (!((in->cbcoord.vertex11 == out->cbcoord.vertex12) &&
      (in->cbcoord.vertex12== out->cbcoord.vertex11))) {
  VALIDATE_TST_VEC(in->cbcoord.vertex11, out->cbcoord.vertex11, 0,
    "cbcoord.vertex11");
  VALIDATE_TST_VEC(in->cbcoord.vertex12, out->cbcoord.vertex12, 0,
    "cbcoord.vertex12");
  }
  VALIDATE_TST_VEC(in->cbcoord.vertex20, out->cbcoord.vertex20, 0,
    "cbcoord.vertex20");
  // Validate only if the vertext order is not swapped.
  if (!((in->cbcoord.vertex21 == out->cbcoord.vertex22) &&
      (in->cbcoord.vertex22== out->cbcoord.vertex21))) {
  VALIDATE_TST_VEC(in->cbcoord.vertex21, out->cbcoord.vertex21, 0,
    "cbcoord.vertex21");
  VALIDATE_TST_VEC(in->cbcoord.vertex22, out->cbcoord.vertex22, 0,
    "cbcoord.vertex22");
  }
  VALIDATE_TST_VEC(in->cbcoord.vertex30, out->cbcoord.vertex30, 0,
    "cbcoord.vertex30");
    // Validate only if the vertext order is not swapped.
  if (!((in->cbcoord.vertex31 == out->cbcoord.vertex32) &&
      (in->cbcoord.vertex32== out->cbcoord.vertex31))) {
  VALIDATE_TST_VEC(in->cbcoord.vertex31, out->cbcoord.vertex31, 0,
    "cbcoord.vertex31");
  VALIDATE_TST_VEC(in->cbcoord.vertex32, out->cbcoord.vertex32, 0,
    "cbcoord.vertex32");
  }
  VALIDATE_TST_VEC(in->cbcoord.vertex40, out->cbcoord.vertex40, 0,
    "cbcoord.vertex40");
    // Validate only if the vertext order is not swapped.
  if (!((in->cbcoord.vertex41 == out->cbcoord.vertex42) &&
      (in->cbcoord.vertex42== out->cbcoord.vertex41))) {
  VALIDATE_TST_VEC(in->cbcoord.vertex41, out->cbcoord.vertex41, 0,
    "cbcoord.vertex41");
  VALIDATE_TST_VEC(in->cbcoord.vertex42, out->cbcoord.vertex42, 0,
    "cbcoord.vertex42");
  }

  /*CR co-efficients*/
  VALIDATE_TST_VEC(in->crcoeff.coef00, out->crcoeff.coef00, 0,
    "crcoeff.coef00");
  VALIDATE_TST_VEC(in->crcoeff.coef01, out->crcoeff.coef01, 0,
    "crcoeff.coef01");
  VALIDATE_TST_VEC(in->crcoeff.coef10, out->crcoeff.coef10, 0,
    "crcoeff.coef10");
  VALIDATE_TST_VEC(in->crcoeff.coef11, out->crcoeff.coef11, 0,
    "crcoeff.coef11");
  VALIDATE_TST_VEC(in->crcoeff.coef20, out->crcoeff.coef20, 0,
    "crcoeff.coef20");
  VALIDATE_TST_VEC(in->crcoeff.coef21, out->crcoeff.coef21, 0,
    "crcoeff.coef21");
  VALIDATE_TST_VEC(in->crcoeff.coef30, out->crcoeff.coef30, 0,
    "crcoeff.coef30");
  VALIDATE_TST_VEC(in->crcoeff.coef31, out->crcoeff.coef31, 0,
    "crcoeff.coef31");
  VALIDATE_TST_VEC(in->crcoeff.coef40, out->crcoeff.coef40, 0,
    "crcoeff.coef40");
  VALIDATE_TST_VEC(in->crcoeff.coef41, out->crcoeff.coef41, 0,
    "crcoeff.coef41");
  VALIDATE_TST_VEC(in->crcoeff.coef50, out->crcoeff.coef50, 0,
    "crcoeff.coef50");
  VALIDATE_TST_VEC(in->crcoeff.coef51, out->crcoeff.coef51, 0,
    "crcoeff.coef51");

  //CB co-efficients
  VALIDATE_TST_VEC(in->cbcoeff.coef00, out->cbcoeff.coef00,
    0, "cbcoeff.coef00");
  VALIDATE_TST_VEC(in->cbcoeff.coef01, out->cbcoeff.coef01,
    0, "cbcoeff.coef01");
  VALIDATE_TST_VEC(in->cbcoeff.coef10, out->cbcoeff.coef10,
    0, "cbcoeff.coef10");
  VALIDATE_TST_VEC(in->cbcoeff.coef11, out->cbcoeff.coef11,
    0, "cbcoeff.coef11");
  VALIDATE_TST_VEC(in->cbcoeff.coef20, out->cbcoeff.coef20,
    0, "cbcoeff.coef20");
  VALIDATE_TST_VEC(in->cbcoeff.coef21, out->cbcoeff.coef21,
    0, "cbcoeff.coef21");
  VALIDATE_TST_VEC(in->cbcoeff.coef30, out->cbcoeff.coef30,
    0, "cbcoeff.coef30");
  VALIDATE_TST_VEC(in->cbcoeff.coef31, out->cbcoeff.coef31,
    0, "cbcoeff.coef31");
  VALIDATE_TST_VEC(in->cbcoeff.coef40, out->cbcoeff.coef40,
    0, "cbcoeff.coef40");
  VALIDATE_TST_VEC(in->cbcoeff.coef41, out->cbcoeff.coef41,
    0, "cbcoeff.coef41");
  VALIDATE_TST_VEC(in->cbcoeff.coef50, out->cbcoeff.coef50,
    0, "cbcoeff.coef50");
  VALIDATE_TST_VEC(in->cbcoeff.coef51, out->cbcoeff.coef51,
    0, "cbcoeff.coef51");

  //Cr Offsets
  VALIDATE_TST_VEC(in->croffset.offset0, out->croffset.offset0,
    0, "croffset.offset0");
  VALIDATE_TST_VEC(in->croffset.offset1, out->croffset.offset1,
    0, "croffset.offset1");
  VALIDATE_TST_VEC(in->croffset.offset2, out->croffset.offset2,
    0, "croffset.offset2");
  VALIDATE_TST_VEC(in->croffset.offset3, out->croffset.offset3,
    0, "croffset.offset3");
  VALIDATE_TST_VEC(in->croffset.offset4, out->croffset.offset4,
    0, "croffset.offset4");
  VALIDATE_TST_VEC(in->croffset.offset5, out->croffset.offset5,
    0, "croffset.offset5");

  //Cr Shits
  VALIDATE_TST_VEC(in->croffset.shift0, out->croffset.shift0,
    0, "croffset.shift0");
  VALIDATE_TST_VEC(in->croffset.shift1, out->croffset.shift1,
    0, "croffset.shift1");
  VALIDATE_TST_VEC(in->croffset.shift2, out->croffset.shift2,
    0, "croffset.shift2");
  VALIDATE_TST_VEC(in->croffset.shift3, out->croffset.shift3,
    0, "croffset.shift3");
  VALIDATE_TST_VEC(in->croffset.shift4, out->croffset.shift4,
    0, "croffset.shift4");
  VALIDATE_TST_VEC(in->croffset.shift5, out->croffset.shift5,
    0, "croffset.shift5");

  //Cb Offsets
  VALIDATE_TST_VEC(in->cboffset.offset0, out->cboffset.offset0,
    0, "cboffset.offset0");
  VALIDATE_TST_VEC(in->cboffset.offset1, out->cboffset.offset1,
    0, "cboffset.offset1");
  VALIDATE_TST_VEC(in->cboffset.offset2, out->cboffset.offset2,
    0, "cboffset.offset2");
  VALIDATE_TST_VEC(in->cboffset.offset3, out->cboffset.offset3,
    0, "cboffset.offset3");
  VALIDATE_TST_VEC(in->cboffset.offset4, out->cboffset.offset4,
    0, "cboffset.offset4");
  VALIDATE_TST_VEC(in->cboffset.offset5, out->cboffset.offset5,
    0, "cboffset.offset5");

  //Cb Shits
  VALIDATE_TST_VEC(in->cboffset.shift0, out->cboffset.shift0,
    0, "cboffset.shift0");
  VALIDATE_TST_VEC(in->cboffset.shift1, out->cboffset.shift1,
    0, "cboffset.shift1");
  VALIDATE_TST_VEC(in->cboffset.shift2, out->cboffset.shift2,
    0, "cboffset.shift2");
  VALIDATE_TST_VEC(in->cboffset.shift3, out->cboffset.shift3,
    0, "cboffset.shift3");
  VALIDATE_TST_VEC(in->cboffset.shift4, out->cboffset.shift4,
    0, "cboffset.shift4");
  VALIDATE_TST_VEC(in->cboffset.shift5, out->cboffset.shift5,
    0, "cboffset.shift5");

  return 0;
}
