/* sce40.c
 *
 * Copyright (c) 2014, 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "sce40.h"
#include "sce_algo.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SCE, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SCE, fmt, ##args)

#define SWAP(a,b) ({\
  a.x^=b.x;\
  b.x^=a.x;\
  a.x^=b.x;\
  a.y^=b.y;\
  b.y^=a.y;\
  a.y^=b.y;\
})

/* A, B, D, E is 12S in Hw, so the bits for abs of affine transform is 11 */
/* C, F is 17S in Hw, so the bits for abs of offset shift(Q2) is 16 */
#define MAX_BIT_Q2 16
#define MAX_BIT_Q1_Q2 11

/** sce_reorder_vertices
 *
 * updates the sce trigger enable flag
 *
 **/
static void sce_reorder_vertices(cr_cb_triangle_float in_tr, cr_cb_triangle *out_tr)
{
  sce_coor_t vertex[3];
  float slope_from_a0 = 0.0;
  float y2_at_slope = 0.0;
  boolean infinite_slope = FALSE;

  if((in_tr.point1.cr == in_tr.point2.cr) &&(in_tr.point2.cr == in_tr.point3.cr)) {
    ISP_ERR("Points are colinear");
  }

  if((in_tr.point1.cb == in_tr.point2.cb) && (in_tr.point2.cb == in_tr.point3.cb)) {
    ISP_ERR("Points are colinear");
  }

  vertex[0].x = (int)in_tr.point1.cr;
  vertex[0].y = (int)in_tr.point1.cb;
  vertex[1].x = (int)in_tr.point2.cr;
  vertex[1].y = (int)in_tr.point2.cb;
  vertex[2].x = (int)in_tr.point3.cr;
  vertex[2].y = (int)in_tr.point3.cb;

  if ((vertex[0].x - vertex[1].x) != 0) {
    slope_from_a0 = (vertex[0].y - vertex[1].y) / (vertex[0].x - vertex[1].x);
  } else {
    ISP_DBG("x coordinate verte0.x = %d, vertex1.x = %d vertex2.x = %d",
      vertex[0].x, vertex[1].x, vertex[2].x);
    infinite_slope = TRUE;
  }
  y2_at_slope = (vertex[2].x - vertex[0].x) * slope_from_a0  + vertex[0].y;

  if (infinite_slope == FALSE) {
    if (((vertex[1].x > vertex[0].x) && (vertex[2].y < y2_at_slope)) ||
       ((vertex[1].x <= vertex[0].x) && (vertex[2].y > y2_at_slope))) {
        ISP_DBG("clokwise, swap!");
        SWAP(vertex[1], vertex[2]);
    } else {
      ISP_DBG(" counter clockwise, no swap");
    }
  } else {
    ISP_DBG("infinite slope!!");
    if (((vertex[1].y > vertex[0].y) && (vertex[2].x > vertex[1].x)) ||
        ((vertex[1].y < vertex[0].y) && (vertex[2].x < vertex[1].x))) {
        ISP_DBG("clokwise, swap!");
        SWAP(vertex[1], vertex[2]);
    } else {
        ISP_DBG("counter clok wise, no swap");
    }
  }
  out_tr->point1.cr = vertex[0].x;
  out_tr->point1.cb = vertex[0].y;
  out_tr->point2.cr = vertex[1].x;
  out_tr->point2.cb = vertex[1].y;
  out_tr->point3.cr = vertex[2].x;
  out_tr->point3.cb = vertex[2].y;
}

/** calc_sce_newendpoint
 *
 * DESCRIPTION:  Calculate the new vertex based on the control
 *               factor
 *
 **/
static void calc_sce_newendpoint(double *rEnd_Cr, double *rEnd_Cb,
  const double Start_Cr, const double Start_Cb, double adj_fac)
{
  ISP_DBG("%s:",__func__);
  ISP_DBG("adj_fac : %lf",adj_fac);
  ISP_DBG("rEnd_cr : %lf, start_cr : %lf ",*rEnd_Cr,Start_Cr);
  ISP_DBG("rEnd_cb : %lf, start_cb : %lf ",*rEnd_Cb,Start_Cb);

  *rEnd_Cr = adj_fac * (*rEnd_Cr - Start_Cr) + Start_Cr;
  *rEnd_Cb = adj_fac * (*rEnd_Cb - Start_Cb) + Start_Cb;

  *rEnd_Cr = MAX(*rEnd_Cr,-128);
  *rEnd_Cr = MIN(*rEnd_Cr, 127);

  *rEnd_Cb = MAX(*rEnd_Cb,-128);
  *rEnd_Cb = MIN(*rEnd_Cb, 127);

  ISP_DBG("Final rEnd_cr : %lf, rEnd_cb : %lf",*rEnd_Cr,*rEnd_Cb);
}

/** sce40_ez_isp_update
 *
 *  @sceCfg: sce  module cfg
 *  @sceDiag: sce Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void sce40_ez_isp_update(skincolorenhancement_t *sceDiag,
  ISP_Skin_enhan_ConfigCmdType *sceCfg)
{
  sceDiag->crcoord.vertex00 = sceCfg->crcoord.vertex00;
  sceDiag->crcoord.vertex01 = sceCfg->crcoord.vertex01;
  sceDiag->crcoord.vertex02 = sceCfg->crcoord.vertex02;
  sceDiag->cbcoord.vertex00 = sceCfg->cbcoord.vertex00;
  sceDiag->cbcoord.vertex01 = sceCfg->cbcoord.vertex01;
  sceDiag->cbcoord.vertex02 = sceCfg->cbcoord.vertex02;
  sceDiag->crcoeff.coef00 = sceCfg->crcoeff.coef00;     /*coef A*/
  sceDiag->crcoeff.coef01 = sceCfg->crcoeff.coef01;     /*coef B*/
  sceDiag->croffset.offset0 = sceCfg->croffset.offset0; /*coef C*/
  sceDiag->croffset.shift0 = sceCfg->croffset.shift0;   /*matrix shift*/
  sceDiag->cbcoeff.coef00 = sceCfg->cbcoeff.coef00;     /*coef D*/
  sceDiag->cbcoeff.coef01 = sceCfg->cbcoeff.coef01;     /*coef E*/
  sceDiag->cboffset.offset0 = sceCfg->cboffset.offset0; /*coef F*/
  sceDiag->cboffset.shift0 = sceCfg->cboffset.shift0;   /*offset shift*/

  sceDiag->crcoord.vertex10 = sceCfg->crcoord.vertex10;
  sceDiag->crcoord.vertex11 = sceCfg->crcoord.vertex11;
  sceDiag->crcoord.vertex12 = sceCfg->crcoord.vertex12;
  sceDiag->cbcoord.vertex10 = sceCfg->cbcoord.vertex10;
  sceDiag->cbcoord.vertex11 = sceCfg->cbcoord.vertex11;
  sceDiag->cbcoord.vertex12 = sceCfg->cbcoord.vertex12;
  sceDiag->crcoeff.coef10 = sceCfg->crcoeff.coef10;     /*coef A*/
  sceDiag->crcoeff.coef11 = sceCfg->crcoeff.coef11;     /*coef B*/
  sceDiag->croffset.offset1 = sceCfg->croffset.offset1; /*coef C*/
  sceDiag->croffset.shift1 = sceCfg->croffset.shift1;   /*matrix shift*/
  sceDiag->cbcoeff.coef10 = sceCfg->cbcoeff.coef10;     /*coef D*/
  sceDiag->cbcoeff.coef11 = sceCfg->cbcoeff.coef11;     /*coef E*/
  sceDiag->cboffset.offset1 = sceCfg->cboffset.offset1; /*coef F*/
  sceDiag->cboffset.shift1 = sceCfg->cboffset.shift1;   /*offset shift*/

  sceDiag->crcoord.vertex20 = sceCfg->crcoord.vertex20;
  sceDiag->crcoord.vertex21 = sceCfg->crcoord.vertex21;
  sceDiag->crcoord.vertex22 = sceCfg->crcoord.vertex22;
  sceDiag->cbcoord.vertex20 = sceCfg->cbcoord.vertex20;
  sceDiag->cbcoord.vertex21 = sceCfg->cbcoord.vertex21;
  sceDiag->cbcoord.vertex22 = sceCfg->cbcoord.vertex22;
  sceDiag->crcoeff.coef20 = sceCfg->crcoeff.coef20;     /*coef A*/
  sceDiag->crcoeff.coef21 = sceCfg->crcoeff.coef21;     /*coef B*/
  sceDiag->croffset.offset2 = sceCfg->croffset.offset2; /*coef C*/
  sceDiag->croffset.shift2 = sceCfg->croffset.shift2;   /*matrix shift*/
  sceDiag->cbcoeff.coef20 = sceCfg->cbcoeff.coef20;     /*coef D*/
  sceDiag->cbcoeff.coef21 = sceCfg->cbcoeff.coef21;     /*coef E*/
  sceDiag->cboffset.offset2 = sceCfg->cboffset.offset2; /*coef F*/
  sceDiag->cboffset.shift2 = sceCfg->cboffset.shift2;   /*offset shift*/

  sceDiag->crcoord.vertex30 = sceCfg->crcoord.vertex30;
  sceDiag->crcoord.vertex31 = sceCfg->crcoord.vertex31;
  sceDiag->crcoord.vertex32 = sceCfg->crcoord.vertex32;
  sceDiag->cbcoord.vertex30 = sceCfg->cbcoord.vertex30;
  sceDiag->cbcoord.vertex31 = sceCfg->cbcoord.vertex31;
  sceDiag->cbcoord.vertex32 = sceCfg->cbcoord.vertex32;
  sceDiag->crcoeff.coef30 = sceCfg->crcoeff.coef30;     /*coef A*/
  sceDiag->crcoeff.coef31 = sceCfg->crcoeff.coef31;     /*coef B*/
  sceDiag->croffset.offset3 = sceCfg->croffset.offset3; /*coef C*/
  sceDiag->croffset.shift3 = sceCfg->croffset.shift3;   /*matrix shift*/
  sceDiag->cbcoeff.coef30 = sceCfg->cbcoeff.coef30;     /*coef D*/
  sceDiag->cbcoeff.coef31 = sceCfg->cbcoeff.coef31;     /*coef E*/
  sceDiag->cboffset.offset3 = sceCfg->cboffset.offset3; /*coef F*/
  sceDiag->cboffset.shift3 = sceCfg->cboffset.shift3;   /*offset shift*/

  sceDiag->crcoord.vertex40 = sceCfg->crcoord.vertex40;
  sceDiag->crcoord.vertex41 = sceCfg->crcoord.vertex41;
  sceDiag->crcoord.vertex42 = sceCfg->crcoord.vertex42;
  sceDiag->cbcoord.vertex40 = sceCfg->cbcoord.vertex40;
  sceDiag->cbcoord.vertex41 = sceCfg->cbcoord.vertex41;
  sceDiag->cbcoord.vertex42 = sceCfg->cbcoord.vertex42;
  sceDiag->crcoeff.coef40 = sceCfg->crcoeff.coef40;     /*coef A*/
  sceDiag->crcoeff.coef41 = sceCfg->crcoeff.coef41;     /*coef B*/
  sceDiag->croffset.offset4 = sceCfg->croffset.offset4; /*coef C*/
  sceDiag->croffset.shift4 = sceCfg->croffset.shift4;   /*matrix shift*/
  sceDiag->cbcoeff.coef40 = sceCfg->cbcoeff.coef40;     /*coef D*/
  sceDiag->cbcoeff.coef41 = sceCfg->cbcoeff.coef41;     /*coef E*/
  sceDiag->cboffset.offset4 = sceCfg->cboffset.offset4; /*coef F*/
  sceDiag->cboffset.shift4 = sceCfg->cboffset.shift4;   /*offset shift*/
}/* sce40_ez_isp_update */

/** sce40_fill_vfe_diag_data:
 *
 *  @sce: sce module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean sce40_fill_vfe_diag_data(sce40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean                 ret = TRUE;
  skincolorenhancement_t *skincolorenhan = NULL;
  vfe_diagnostics_t      *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    skincolorenhan = &vfe_diag->prev_skincolorenhan;

    sce40_ez_isp_update(skincolorenhan, &mod->sce_cmd);
  }
  return ret;
}/*sce40_fill_vfe_diag_data*/

/** sce40_print_config
 *
 *  @cmd: handle to ISP_SceCmdType
 *
 *  Print sce struct info for debug
 *
 *  Returns none
 **/
static void sce40_print_config(ISP_Skin_enhan_ConfigCmdType *cmd)
{
  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }

  ISP_DBG("==== Sce configurations ====");
  ISP_DBG("== Triangle 1 ==");
  ISP_DBG("vertex00:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex00, cmd->cbcoord.vertex00);
  ISP_DBG("vertex01:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex01, cmd->cbcoord.vertex01);
  ISP_DBG("vertex02:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex02, cmd->cbcoord.vertex02);
  ISP_DBG("crcoeff.coef00 (affine coeff A) : %d", cmd->crcoeff.coef00);
  ISP_DBG("crcoeff.coef01 (affine coeff B) : %d", cmd->crcoeff.coef01);
  ISP_DBG("cbcoeff.coef00 (affine coeff D) : %d", cmd->cbcoeff.coef00);
  ISP_DBG("cbcoeff.coef01 (affine coeff E) : %d", cmd->cbcoeff.coef01);
  ISP_DBG("croffset.offset0: (affine coeff C) %d", cmd->croffset.offset0);
  ISP_DBG("croffset.shift0 : (Matrix shift) %d", cmd->croffset.shift0);
  ISP_DBG("cboffset.offset0: (affine coeff F) %d", cmd->cboffset.offset0);
  ISP_DBG("cboffset.shift0 : (Offset shift) %d", cmd->cboffset.shift0);

  ISP_DBG("== Triangle 2 ==");
  ISP_DBG("vertex10:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex10, cmd->cbcoord.vertex10);
  ISP_DBG("vertex11:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex11, cmd->cbcoord.vertex11);
  ISP_DBG("vertex12:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex12, cmd->cbcoord.vertex12);
  ISP_DBG("crcoeff.coef10 (affine coeff A) : %d", cmd->crcoeff.coef10);
  ISP_DBG("crcoeff.coef11 (affine coeff B) : %d", cmd->crcoeff.coef11);
  ISP_DBG("cbcoeff.coef10 (affine coeff D) : %d", cmd->cbcoeff.coef10);
  ISP_DBG("cbcoeff.coef11 (affine coeff E) : %d", cmd->cbcoeff.coef11);
  ISP_DBG("croffset.offset1: (affine coeff C) %d", cmd->croffset.offset1);
  ISP_DBG("croffset.shift1 : (Matrix shift) %d", cmd->croffset.shift1);
  ISP_DBG("cboffset.offset1: (affine coeff F) %d", cmd->cboffset.offset1);
  ISP_DBG("cboffset.shift1 : (Offset shift) %d", cmd->cboffset.shift1);

  ISP_DBG("== Triangle 3 ==");
  ISP_DBG("vertex20:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex20, cmd->cbcoord.vertex20);
  ISP_DBG("vertex21:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex21, cmd->cbcoord.vertex21);
  ISP_DBG("vertex22:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex22, cmd->cbcoord.vertex22);
  ISP_DBG("crcoeff.coef20 (affine coeff A) : %d", cmd->crcoeff.coef20);
  ISP_DBG("crcoeff.coef21 (affine coeff B) : %d", cmd->crcoeff.coef21);
  ISP_DBG("cbcoeff.coef20 (affine coeff D) : %d", cmd->cbcoeff.coef20);
  ISP_DBG("cbcoeff.coef21 (affine coeff E) : %d", cmd->cbcoeff.coef21);
  ISP_DBG("croffset.offset2: (affine coeff C) %d", cmd->croffset.offset2);
  ISP_DBG("croffset.shift2 : (Matrix shift) %d", cmd->croffset.shift2);
  ISP_DBG("cboffset.offset2: (affine coeff F) %d", cmd->cboffset.offset2);
  ISP_DBG("cboffset.shift2 : (Offset shift) %d", cmd->cboffset.shift2);

  ISP_DBG("== Triangle 4 ==");
  ISP_DBG("vertex30:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex30, cmd->cbcoord.vertex30);
  ISP_DBG("vertex31:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex31, cmd->cbcoord.vertex31);
  ISP_DBG("vertex32:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex32, cmd->cbcoord.vertex32);
  ISP_DBG("crcoeff.coef30 (affine coeff A) : %d", cmd->crcoeff.coef30);
  ISP_DBG("crcoeff.coef31 (affine coeff B) : %d", cmd->crcoeff.coef31);
  ISP_DBG("cbcoeff.coef30 (affine coeff D) : %d", cmd->cbcoeff.coef30);
  ISP_DBG("cbcoeff.coef31 (affine coeff E) : %d", cmd->cbcoeff.coef31);
  ISP_DBG("croffset.offset3: (affine coeff C) %d", cmd->croffset.offset3);
  ISP_DBG("croffset.shift3 : (Matix shift) %d", cmd->croffset.shift3);
  ISP_DBG("cboffset.offset3: (affine coeff F) %d", cmd->cboffset.offset3);
  ISP_DBG("cboffset.shift3 : (Offset shift) %d", cmd->cboffset.shift3);

  ISP_DBG("== Triangle 5 ==");
  ISP_DBG("vertex40:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex40, cmd->cbcoord.vertex40);
  ISP_DBG("vertex41:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex41, cmd->cbcoord.vertex41);
  ISP_DBG("vertex42:(Cr, Cb) (%d, %d)",
    cmd->crcoord.vertex42, cmd->cbcoord.vertex42);
  ISP_DBG("crcoeff.coef40 (affine coeff A) : %d", cmd->crcoeff.coef40);
  ISP_DBG("crcoeff.coef41 (affine coeff B) : %d", cmd->crcoeff.coef41);
  ISP_DBG("cbcoeff.coef40 (affine coeff D) : %d", cmd->cbcoeff.coef40);
  ISP_DBG("cbcoeff.coef41 (affine coeff E) : %d", cmd->cbcoeff.coef41);
  ISP_DBG("croffset.offset4: (affine coeff C) %d", cmd->croffset.offset4);
  ISP_DBG("croffset.shift4 : (Matrix shift) %d", cmd->croffset.shift4);
  ISP_DBG("cboffset.offset4: (affine coeff F) %d", cmd->cboffset.offset4);
  ISP_DBG("cboffset.shift4 : (Offset shift) %d", cmd->cboffset.shift4);

  ISP_DBG("== outside area ==");
  ISP_DBG("crcoeff.coef50 (affine coeff A) : %d", cmd->crcoeff.coef50);
  ISP_DBG("crcoeff.coef51 (affine coeff B) : %d", cmd->crcoeff.coef51);
  ISP_DBG("cbcoeff.coef50 (affine coeff D) : %d", cmd->cbcoeff.coef50);
  ISP_DBG("cbcoeff.coef51 (affine coeff E) : %d", cmd->cbcoeff.coef51);
  ISP_DBG("croffset.offset5: (affine coeff C) %d", cmd->croffset.offset5);
  ISP_DBG("croffset.shift5 : (Matrix shift) %d", cmd->croffset.shift5);
  ISP_DBG("cboffset.offset5: (affine coeff F) %d", cmd->cboffset.offset5);
  ISP_DBG("cboffset.shift5 : (Offset shift) %d", cmd->cboffset.shift5);

} /* sce40_print_config */

/** sce_find_line_by_vector:
 *    @line: result line
 *    @vector: line direction vector
 *    @c_point: point belonging to line
 *
 *  This function finds a line parametric equation by vector and point.
 *
 *  Return:  None
 **/
static void sce_find_line_by_vector(ISP_Skin_enhan_line *line,
  sce_shift_vector *vector, cr_cb_point_float *c_point)
{
  line->point0  = *c_point;
  line->shift_cr = vector->cr;
  line->shift_cb = vector->cb;
} /* sce_find_line_by_vector */

/** sce_find_line_by_two_points:
 *    @line: result line
 *    @point1: point belonging to line
 *    @point2: point belonging to line
 *
 *  This function finds a line parametric equation by two points.
 *
 *  Return:  None
 **/
static void sce_find_line_by_two_points(ISP_Skin_enhan_line *line,
  cr_cb_point_float *point1, cr_cb_point_float *point2)
{
  line->point0  = *point1;
  line->shift_cr = (point2->cr - point1->cr);
  line->shift_cb = (point2->cb - point1->cb);
} /* sce_find_line_by_two_points*/

/** sce_find_intersection:
 *    @line1: first line
 *    @line2: second line
 *    @t: parameter for intersection point in first line
 *
 *  This function finds a intersection between two lines and return its position
 *  in first line by parameter.
 *
 *  Return:  TRUE  - lines are intersecting
 *           FALSE - lines are parallel
 **/
static boolean sce_find_intersection(ISP_Skin_enhan_line *line1,
  ISP_Skin_enhan_line *line2, double* t)
{
  double t2;
  /* check if one of the lines is either vertical or horizontal */
  /* Assumptions according SCE documentation:
     - lines are not matching:
        One of the line is crossing center of pentagon and other is one
        of its sides
     - none of the points belong to neither axis (cb and cr does not equal 0)
     - no line can have both shift_cr and shift_cb equal to 0
   */
  if ((F_EQUAL((line2->shift_cb * line1->shift_cr -
      line1->shift_cb * line2->shift_cr), 0)) ||
      ((F_EQUAL(line1->shift_cr, 0) && F_EQUAL(line2->shift_cr, 0)))){
    /* Lines are parallel */

    ISP_DBG("%s: parallel lines", __func__);
    return FALSE;
  }

  if(F_EQUAL(line1->shift_cr, 0)) {
    *t =  ((line1->point0.cr - line2->point0.cr) * line2->shift_cb +
          (line2->point0.cb - line1->point0.cb) * line2->shift_cr) /
          (line1->shift_cb * line2->shift_cr - line2->shift_cb * line1->shift_cr);
    t2  = (line1->point0.cr - line2->point0.cr + line1->shift_cr * *t) /
          line2->shift_cr;
  } else {
    t2 =  ((line2->point0.cr - line1->point0.cr) * line1->shift_cb +
          (line1->point0.cb - line2->point0.cb) * line1->shift_cr) /
          (line2->shift_cb * line1->shift_cr - line1->shift_cb * line2->shift_cr);
    *t =  (line2->point0.cr - line1->point0.cr + line2->shift_cr * t2) /
          line1->shift_cr;
  }

  if ((t2 < 0) || (t2 > 1)){
    /* Lines intersect outside poligon */
    ISP_DBG("%s: outside intersection", __func__);
    return FALSE;
  }

  return TRUE;
} /* sce_find_intersection */

/** sce_found_boundaries:
 *    @triangles: set of SCE triangles
 *    @line: line for SCE interpolation
 *    @t_pos boundary in positive direction of shift vector
 *    @t_neg boundary in negative direction of shift vector
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function finds boundaries in both directions from origin point between
 *  which point position can be interpolated according system team documentation
 *
 *  Return:   0 - Success
 *           -1 - Numbers from chromatix are inconsistent: triangles
 *                don't form proper poligon.
 **/
static int sce_found_boundaries(sce_cr_cb_triangle_set_float *triangles,
  ISP_Skin_enhan_line *line, double *t_pos, double *t_neg)
{
  cr_cb_triangle_float *array[5];
  double ints_t[2], tmp;
  int idx_t = 0;
  ISP_Skin_enhan_line temp_line;
  int i, j;
  boolean is_vertex;

  array[0] = &triangles->triangle1;
  array[1] = &triangles->triangle2;
  array[2] = &triangles->triangle3;
  array[3] = &triangles->triangle4;
  array[4] = &triangles->triangle5;

  for(i = 0; i < 5; i++) {
    sce_find_line_by_two_points(&temp_line, &array[i]->point2, &array[i]->point3);
    if(sce_find_intersection(line, &temp_line, &tmp)){
      is_vertex = FALSE;
      for(j = 0; j < idx_t; j++)
        if(j<2 && F_EQUAL(ints_t[j], tmp)) {
          /* intersection point is vertex */
          is_vertex = TRUE;
          break;
        }
      if(is_vertex)
        continue;
      if(idx_t > 1){
        /* line intersects pentagon in more than two points  - this should not
           happen unless chromatix is incorrect */
        ISP_ERR("%s: Error: too many intersections", __func__);
        return -1;
      }
      ints_t[idx_t] = tmp;
      idx_t++;
    }
  }

  if(idx_t < 2){
    /* line intersects pentagon in less than two points  - this should not
        happen unless chromatix is incorrect */
    ISP_ERR("%s: Error: less than two intersections %d", __func__, idx_t);
    return -1;
  }

  if(ints_t[0] > 0){
    *t_pos = ints_t[0];
    *t_neg = ints_t[1];
  } else {
    *t_pos = ints_t[1];
    *t_neg = ints_t[0];
  }

  if((*t_pos < 0) || (*t_neg > 0)) {
    /* intersections are on the same side of central point  - this should not
       happen unless chromatix is incorrect */
    ISP_ERR("%s: Error: intersections are on the same side of central point",
               __func__);
    return -1;
  }

  /* According documentation boundaries should be at two thirds of distance
     between central and intersection points. */
  *t_pos *= 2.0/3.0;
  *t_neg *= 2.0/3.0;

  ISP_DBG("%s: Boundaries %lf,%lf and %lf,%lf",__func__,
    line->point0.cr + line->shift_cr * *t_pos,
    line->point0.cb + line->shift_cb * *t_pos,
    line->point0.cr + line->shift_cr * *t_neg,
    line->point0.cb + line->shift_cb * *t_neg);

  return 0;
} /* sce_found_boundaries */

/** sce_copy_triangle_float
 *
 *  @sce: sce handle
 *  @p_sce: default configuration from SCE chromatix Header
 *
 *  reorder the triangles by default chromatix triangles
 *
 *  Return none
 **/
void sce_copy_triangle_float(sce_cr_cb_triangle_set *int_triangle,
  sce_cr_cb_triangle_set_float *float_triangle)
{
  float_triangle->triangle1.point1.cr = (float)int_triangle->triangle1.point1.cr;
  float_triangle->triangle1.point1.cb = (float)int_triangle->triangle1.point1.cb;
  float_triangle->triangle1.point2.cr = (float)int_triangle->triangle1.point2.cr;
  float_triangle->triangle1.point2.cb = (float)int_triangle->triangle1.point2.cb;
  float_triangle->triangle1.point3.cr = (float)int_triangle->triangle1.point3.cr;
  float_triangle->triangle1.point3.cb = (float)int_triangle->triangle1.point3.cb;
  float_triangle->triangle2.point1.cr = (float)int_triangle->triangle2.point1.cr;
  float_triangle->triangle2.point1.cb = (float)int_triangle->triangle2.point1.cb;
  float_triangle->triangle2.point2.cr = (float)int_triangle->triangle2.point2.cr;
  float_triangle->triangle2.point2.cb = (float)int_triangle->triangle2.point2.cb;
  float_triangle->triangle2.point3.cr = (float)int_triangle->triangle2.point3.cr;
  float_triangle->triangle2.point3.cb = (float)int_triangle->triangle2.point3.cb;
  float_triangle->triangle3.point1.cr = (float)int_triangle->triangle3.point1.cr;
  float_triangle->triangle3.point1.cb = (float)int_triangle->triangle3.point1.cb;
  float_triangle->triangle3.point2.cr = (float)int_triangle->triangle3.point2.cr;
  float_triangle->triangle3.point2.cb = (float)int_triangle->triangle3.point2.cb;
  float_triangle->triangle3.point3.cr = (float)int_triangle->triangle3.point3.cr;
  float_triangle->triangle3.point3.cb = (float)int_triangle->triangle3.point3.cb;
  float_triangle->triangle4.point1.cr = (float)int_triangle->triangle4.point1.cr;
  float_triangle->triangle4.point1.cb = (float)int_triangle->triangle4.point1.cb;
  float_triangle->triangle4.point2.cr = (float)int_triangle->triangle4.point2.cr;
  float_triangle->triangle4.point2.cb = (float)int_triangle->triangle4.point2.cb;
  float_triangle->triangle4.point3.cr = (float)int_triangle->triangle4.point3.cr;
  float_triangle->triangle4.point3.cb = (float)int_triangle->triangle4.point3.cb;
  float_triangle->triangle5.point1.cr = (float)int_triangle->triangle5.point1.cr;
  float_triangle->triangle5.point1.cb = (float)int_triangle->triangle5.point1.cb;
  float_triangle->triangle5.point2.cr = (float)int_triangle->triangle5.point2.cr;
  float_triangle->triangle5.point2.cb = (float)int_triangle->triangle5.point2.cb;
  float_triangle->triangle5.point3.cr = (float)int_triangle->triangle5.point3.cr;
  float_triangle->triangle5.point3.cb = (float)int_triangle->triangle5.point3.cb;
}

/** sce40_match_dest_orig
 *
 *  Return TRUE all match and FALSE on mismatch
 **/
static boolean sce40_match_dest_orig_chromatix(chromatix_SCE_type *p_sce)
{
  boolean is_mismatch = FALSE;

  if (0 != memcmp(&p_sce->origin_triangles_A,
    &p_sce->destination_triangles_A, sizeof (sce_cr_cb_triangle_set))) {
    is_mismatch = TRUE;
  }

  if (0 != memcmp(&p_sce->origin_triangles_TL84,
    &p_sce->destination_triangles_TL84, sizeof (sce_cr_cb_triangle_set))) {
    is_mismatch = TRUE;
  }
  if (0 != memcmp(&p_sce->origin_triangles_D65,
    &p_sce->destination_triangles_D65, sizeof (sce_cr_cb_triangle_set))) {
    is_mismatch = TRUE;
  }

  if (0 != memcmp(&p_sce->origin_triangles_H,
    &p_sce->destination_triangles_H, sizeof (sce_cr_cb_triangle_set))) {
    is_mismatch = TRUE;
  }

  if (is_mismatch == TRUE)
    return FALSE;

  return TRUE;
}

/** sce_copy_triangles_from_chromatix
 *
 *  @sce: sce handle
 *  @p_sce: default configuration from SCE chromatix Header
 *
 *  reorder the triangles by default chromatix triangles
 *
 *  Return none
 **/
void sce_copy_triangles_from_chromatix(sce40_t *sce,
  chromatix_SCE_type *p_sce)
{
  sce_copy_triangle_float(&p_sce->origin_triangles_A, &sce->origin_triangles_A);
  sce_copy_triangle_float(&p_sce->destination_triangles_A, &sce->destination_triangles_A);
  sce_copy_triangle_float(&p_sce->origin_triangles_D65, &sce->origin_triangles_D65);
  sce_copy_triangle_float(&p_sce->destination_triangles_D65, &sce->destination_triangles_D65);
  sce_copy_triangle_float(&p_sce->origin_triangles_TL84, &sce->origin_triangles_TL84);
  sce_copy_triangle_float(&p_sce->destination_triangles_TL84, &sce->destination_triangles_TL84);
  sce_copy_triangle_float(&p_sce->origin_triangles_H, &sce->origin_triangles_H);
  sce_copy_triangle_float(&p_sce->destination_triangles_H, &sce->destination_triangles_H);

  sce40_copy_triangles_from_chromatix(sce, p_sce);

  if (TRUE == sce40_match_dest_orig_chromatix(p_sce)) {
    ISP_DBG("all destination traingle match orignal");
    sce->is_dest_match_orig = TRUE;
  }

  /* Initializing with default tables */
  sce->orig = sce->origin_triangles_TL84;
  sce->dest= sce->destination_triangles_TL84;

}

/** sce40_calc_bit_number
 *
 *  @coeff: transformation matrix
 *  @matrix_shift: output matrix shift
 *  @offset_shift: output affset shift
 *
 *  Calculate the right shift bits for the transformation matrix and
 *              offset operation for triangle n
 *
 *  Return none
 **/
static int sce40_calc_bit_number(double data)
{
  double Q = 0.0;
  int Q_int = 0;

  /* tolerance is to resolve precision diff between simulator and target.
     the tolerance is set to match simulator*/
  float neg_tolerance = -1e-7f;
  float pos_tolerance = 1e-5f;

  if (!F_EQUAL(0.0, data)) {
    Q = log(data) / log(2.0);
  } else {
    Q = 0.0;
  }

  /*synced this implementation with simulator,
    log Q tolerate a small precision diff,
    to avoid the huge diff on shift*/
  if (Q < 0 && Q >= neg_tolerance)
    Q = 0.0;

  if (Q == (int)Q) {
    /*if dMax exactly 2's power of Q, need Q+1 bit  */
    if (Q >= 0)
      Q++;

    Q_int = (int)Q;
  } else {
    /* float: ceiling*/
    Q_int = (int)(++Q);
  }

  return Q_int;
}

/** sce40_calc_shiftbits
 *
 *  @coeff: transformation matrix
 *  @matrix_shift: output matrix shift
 *  @offset_shift: output affset shift
 *
 *  Calculate the right shift bits for the transformation matrix and
 *              offset operation for triangle n
 *
 *  Return none
 **/
static void sce40_calc_shiftbits(sce_affine_transform_2d_double *coeff,
  uint32_t *matrix_shift, uint32_t *offset_shift)
{
  double dMax = 0.0;
  double A,B,C,D,E,F;
  uint32_t bit_shift_Q2 = 0, bit_shift_Q1_Q2 = 0;
  int bit_of_coeff_Q2 = 0, bit_of_coeff_Q1_Q2 = 0;

  A = fabs(coeff->a);
  B = fabs(coeff->b);
  C = fabs(coeff->c);
  D = fabs(coeff->d);
  E = fabs(coeff->e);
  F = fabs(coeff->f);

  /* number of bits:
     number of bit can cover the max determine of A,B,C,D,E */
  dMax = MAX(E, MAX(D, MAX(A,B)));
  bit_of_coeff_Q1_Q2 = sce40_calc_bit_number(dMax);
  bit_shift_Q1_Q2 = MAX_BIT_Q1_Q2 - bit_of_coeff_Q1_Q2;

  /* number of bits:
     number of bit can cover the max determine of A,B,C,D,E */
  dMax = MAX(C, F);
  bit_of_coeff_Q2 = sce40_calc_bit_number(dMax);
  bit_shift_Q2 = MAX_BIT_Q2 - bit_of_coeff_Q2;

  /* matrixShift[i] is Q1 */
  /* offsetShift[i] is Q2, the value of Q2 is restricted by
     the overall shift (Q1+Q2) */
  if (bit_shift_Q1_Q2 <= bit_shift_Q2) {
    ISP_DBG("affine_dbg use offset shift Q1 + Q2");
    *offset_shift = bit_shift_Q1_Q2;
    *matrix_shift = 0;
  } else {
    ISP_DBG("affine_dbg use shift Q2");
    *offset_shift = bit_shift_Q2;
    *matrix_shift = bit_shift_Q1_Q2 - bit_shift_Q2;
  }

  ISP_DBG("affine_dbg Q1_Q2 coeff(ABDE) bit_shift = %d bit_of_coeff %d",
    bit_shift_Q1_Q2, bit_of_coeff_Q1_Q2);
  ISP_DBG("affine_dbg Q2 coeff(CF) bit_shift = %d bit_of_coeff %d",
    bit_shift_Q2, bit_of_coeff_Q2);

  return;
}

/** sce40_calc_sce_affine_transform
 *
 *  @interp_range: range in which the common triangles vertex can be shifted
 *  @pOrigVert: original vertex
 *  @pTransform: transformation matrix
 *  @val: sce adjustment factor
 *
 *  Calculate the new co-efficents and the offset bits
 *  for the new triangle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean sce40_calc_sce_affine_transform(
  ISP_Skin_enhan_range *interp_range,
  cr_cb_triangle_float *OrigVert, cr_cb_triangle_float *DestVert,
  sce_affine_transform_2d_double *Affine_Tx, double adjust_factor,
  boolean shift_from_orig)
{
  double M1[9], M2[9], InvM2[9], Tx[9];
  double dest_cr, dest_cb, int_step;
  int32_t i;
  double determine_orig = 0.0;
  boolean rc = TRUE;

  if (OrigVert == NULL || Affine_Tx == NULL) {
    ISP_ERR("failed! Null pointer! in vfe_util_sce_transform");
    return FALSE;
  }

  /*prepare Matrix 1,
    1. regular: filled destination triangle
    2. if destination match original, shift from original
  */
  if (shift_from_orig) {
    /* Calculate the new vertex */
    ISP_DBG("shift from orig triangle!! dest mathc orig triangle!");
    if (adjust_factor < 0)
      int_step = interp_range->neg_step * (-adjust_factor);
    else
      int_step = interp_range->pos_step * adjust_factor;

    dest_cr = interp_range->interpolation_line.point0.cr +
              interp_range->interpolation_line.shift_cr * int_step;
    dest_cb = interp_range->interpolation_line.point0.cb +
              interp_range->interpolation_line.shift_cb * int_step;

    /* fill in M1, from shift vector and orig triangle */
    M1[0] = dest_cr;
    M1[1] = OrigVert->point2.cr;
    M1[2] = OrigVert->point3.cr;
    M1[3] = dest_cb;
    M1[4] = OrigVert->point2.cb;
    M1[5] = OrigVert->point3.cb;
    M1[6] = M1[7] = M1[8] = 1.0;
  } else {
    /* fill in M1, the dest triangle */
    M1[0] = DestVert->point1.cr;
    M1[1] = DestVert->point2.cr;
    M1[2] = DestVert->point3.cr;
    M1[3] = DestVert->point1.cb;
    M1[4] = DestVert->point2.cb;
    M1[5] = DestVert->point3.cb;
    M1[6] = M1[7] = M1[8] = 1.0;
  }

  /* fill in M2 the original three vertex */
  M2[0] = OrigVert->point1.cr;
  M2[1] = OrigVert->point2.cr;
  M2[2] = OrigVert->point3.cr;
  M2[3] = OrigVert->point1.cb;
  M2[4] = OrigVert->point2.cb;
  M2[5] = OrigVert->point3.cb;
  M2[6] = M2[7] = M2[8] = 1.0;

  /* Calculate the new vertex */
  if (shift_from_orig) {
    calc_sce_newendpoint(&M1[0], &M1[3], M2[0], M2[3], adjust_factor);
  }

  MATRIX_INVERSE_3x3(M2, InvM2);
  Tx[8] = M1[6]*InvM2[2] + M1[7]*InvM2[5] + M1[8]*InvM2[8];

  if (Tx[8]) {
     Tx[0] = (M1[0]*InvM2[0] + M1[1]*InvM2[3] + M1[2]*InvM2[6]) / Tx[8];
     Tx[1] = (M1[0]*InvM2[1] + M1[1]*InvM2[4] + M1[2]*InvM2[7]) / Tx[8];
     Tx[2] = (M1[0]*InvM2[2] + M1[1]*InvM2[5] + M1[2]*InvM2[8]) / Tx[8];
     Tx[3] = (M1[3]*InvM2[0] + M1[4]*InvM2[3] + M1[5]*InvM2[6]) / Tx[8];
     Tx[4] = (M1[3]*InvM2[1] + M1[4]*InvM2[4] + M1[5]*InvM2[7]) / Tx[8];
     Tx[5] = (M1[3]*InvM2[2] + M1[4]*InvM2[5] + M1[5]*InvM2[8]) / Tx[8];
     Tx[6] = (M1[6]*InvM2[0] + M1[7]*InvM2[3] + M1[8]*InvM2[6]) / Tx[8];
     Tx[7] = (M1[6]*InvM2[1] + M1[7]*InvM2[4] + M1[8]*InvM2[7]) / Tx[8];
     Tx[8] = 1;

    for (i = 0; i < 6; i++)
      *((double *)Affine_Tx + i) = (Tx[i]);
  } else {
      ISP_DBG("failed! Tx[8] = -1");
  }

  ISP_DBG("affine_dbg === dest matrix ====");
  ISP_DBG("affine_dbg dest: %lf, %lf, %lf", M1[0], M1[1], M1[2]);
  ISP_DBG("affine_dbg dest: %lf, %lf, %lf", M1[3], M1[4], M1[5]);
  ISP_DBG("affine_dbg dest: %lf, %lf, %lf", M1[6], M1[7], M1[8]);

  ISP_DBG("affine_dbg === orig matrix ====");
  ISP_DBG("affine_dbg orig: %lf, %lf, %lf", M2[0], M2[1], M2[2]);
  ISP_DBG("affine_dbg orig: %lf, %lf, %lf", M2[3], M2[4], M2[5]);
  ISP_DBG("affine_dbg orig: %lf, %lf, %lf", M2[6], M2[7], M2[8]);

  ISP_DBG("affine_dbg === tx matrix ====");
  ISP_DBG("affine_dbg affine: A %.9lf, B %.9f, C %.9f", Affine_Tx->a, Affine_Tx->b, Affine_Tx->c);
  ISP_DBG("affine_dbg affine: D %.9lf, E %.9lf, F %.9lf", Affine_Tx->d, Affine_Tx->e, Affine_Tx->f);

  return TRUE;
}

/** calc_sce_shift_affine_coeff
 *
 *  @Tx: transformation matrix
 *  @coeff: output coefficients
 *  @matrix_shift: output matrix shift
 *  @offset_shift: output affset shift
 *
 *  calculate the coefficient updated the config reg struct
 *  with the coeff calculated here
 *
 *  Return: none
 **/
static void calc_sce_shift_affine_coeff(sce_affine_transform_2d_double *Tx,
  int32_t *coeff, uint32_t *matrix_shift, uint32_t *offset_shift)
{
  int i;
  double *TxElem = (double *)Tx;

  /* calculate the co-efficients */
  sce40_calc_shiftbits(Tx, matrix_shift, offset_shift);
  ISP_DBG("affine_dbg  matrix shift %d, offset shift %d", *matrix_shift, *offset_shift);

  for (i = 0; i < 6; i++) {
    if ((i % 3) == 2) {
      if (i == 5)
        ISP_DBG("affine_dbg3 coeff 6 = %.9lf", *TxElem);
      coeff[i] = (int32_t) round((*TxElem) * (double)(1 << (*offset_shift)));
      if (coeff[i] <= -65536 || coeff[i] > 65536) {
        coeff[i] = MAX(coeff[i] , -65536);
        coeff[i] = MIN(coeff[i] , 65536);
      }
    } else {
      coeff[i] = (int32_t)round((*TxElem) *
        (double)(1<<( (*matrix_shift) + (*offset_shift))));
      if (coeff[i] <= -2047 || coeff[i] > 2047) {
        coeff[i] = MAX(coeff[i] , -2047);
        coeff[i] = MIN(coeff[i] , 2047);
      }
    }
    TxElem++;
  }
  return;
} /* calc_sce_shift_affine_coeff */

/** calc_sce_affine_params
 *
 *  @interp_range: range in which the common triangles vertex can be shifted
 *  @pOrigVert: original vertex
 *  @val: sce adjustment factor
 *  @coeff: output coefficients
 *  @matrix_shift: output matrix shift
 *  @offset_shift: output affset shift
 *
 *  Get the new coeff and Tx
 *
 *  Return: none
 **/
static boolean calc_sce_affine_params(ISP_Skin_enhan_range *interp_range,
  cr_cb_triangle_float *orig_triangle, cr_cb_triangle_float *dest_triangle,
  double adjust_factor, int32_t *coeff, uint32_t *matrix_shift,
  uint32_t *offset_shift, boolean is_dest_match_orig)
{
  sce_affine_transform_2d affine_Tx_final;
  sce_affine_transform_2d_double affine_Tx_double;
  boolean ret = TRUE;
  boolean shift_from_orig = FALSE;
  int i = 0;

  if (is_dest_match_orig == TRUE)
    shift_from_orig = TRUE;

  /*affine transfomr operate on double precision to match simulator*/
  ret = sce40_calc_sce_affine_transform(interp_range,
    orig_triangle, dest_triangle, &affine_Tx_double, adjust_factor, shift_from_orig);
  if (ret == FALSE) {
    ISP_ERR("failed,  sce40_calc_sce_affine_transform!");
    return FALSE;
  }
  /*fill in the output affine parameters with Float*/
  affine_Tx_final.a = (float)affine_Tx_double.a;
  affine_Tx_final.b = (float)affine_Tx_double.b;
  affine_Tx_final.c = (float)affine_Tx_double.c;
  affine_Tx_final.d = (float)affine_Tx_double.d;
  affine_Tx_final.e = (float)affine_Tx_double.e;
  affine_Tx_final.f = (float)affine_Tx_double.f;

  /*calculate SCE shit bits and shift the coeffcient A, B, C, D, E, F*/
  calc_sce_shift_affine_coeff(&affine_Tx_double,
    coeff, matrix_shift, offset_shift);

  return ret;
} /* vfe_calc_sce */

/** sce40_set_sce_factor
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: set param event data
 *
 *  Set the value of sce adjust factor using HAL param
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean sce40_set_sce_factor(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                        ret = TRUE;
  sce40_t                       *sce = NULL;
  int32_t                        sce_factor =0;
  mct_event_control_parm_t      *set_param = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sce = (sce40_t *)isp_sub_module->private_data;
  if (!sce) {
    ISP_ERR("failed: sce %p", sce);
    return FALSE;
  }

  set_param = (mct_event_control_parm_t *)data;
  sce_factor = *((int32_t *)set_param->parm_data);
  ISP_DBG(" sce_factor %d ", sce_factor);

  if (!isp_sub_module->chromatix_module_enable) {
    if ( sce_factor == 0 ) {
        isp_sub_module->submod_enable = FALSE;
        isp_sub_module->update_module_bit= TRUE;
    }
    else if( (sce_factor >= 100) || (sce_factor <= -100) ) {
        isp_sub_module->submod_enable = TRUE;
        isp_sub_module->update_module_bit= TRUE;
    }
  }

  sce->sce_adjust_factor = ((double)sce_factor)/100;

  if (!isp_sub_module->submod_enable) {
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
}

/** trigger_interpolate_sce_triangles
 *
 *  @ip1: input triangle 1
 *  @ip2: input triangle 2
 *  @op: interpolate triangle
 *  @ratio: interpolate ratio
 *
 *  interpolate the sce triangles based on ratio
 *
 *  Return: none
 **/
void trigger_interpolate_sce_triangles(sce_cr_cb_triangle_set_float* ip1,
  sce_cr_cb_triangle_set_float* ip2, sce_cr_cb_triangle_set_float* op, float ratio)
{
  float *ptrIp1 = (float *)ip1;
  float *ptrIp2 = (float *)ip2;
  float *ptrOp = (float *)op;
  int i, size = sizeof(sce_cr_cb_triangle_set_float)/ sizeof(float);

  TBL_INTERPOLATE(ptrIp1, ptrIp2, ptrOp, ratio, size, i);
} /* trigger_interpolate_sce_triangles */


/** trigger_interpolate_sce_triangles_int
 *
 *  @ip1: input triangle 1
 *  @ip2: input triangle 2
 *  @op: interpolate triangle
 *  @ratio: interpolate ratio
 *
 *  interpolate the sce triangles based on ratio
 *
 *  Return: none
 **/
void trigger_interpolate_sce_triangles_int(sce_cr_cb_triangle_set* ip1,
  sce_cr_cb_triangle_set* ip2, sce_cr_cb_triangle_set_float* op, float ratio)
{
  int *ptrIp1 = (int *)ip1;
  int *ptrIp2 = (int *)ip2;
  float *ptrOp = (float *)op;
  int i, size = sizeof(sce_cr_cb_triangle_set)/ sizeof(int);

  for (i=0; i<size; i++) {
     ptrOp[i] = LINEAR_INTERPOLATION((float)ptrIp1[i], (float)ptrIp2[i], ratio);
  }
} /* trigger_interpolate_sce_triangles */

/** trigger_interpolate_sce_vectors
 *
 *  @ip1: input vector 1
 *  @ip2: input vector 2
 *  @op: interpolate vector
 *  @ratio: interpolate ratio
 *
 *  Linear interpolation of sce vectors base on ratio
 *
 *  Return: none
 **/
void trigger_interpolate_sce_vectors(sce_shift_vector* ip1,
  sce_shift_vector* ip2, sce_shift_vector* op, float ratio)
{
  op->cr = LINEAR_INTERPOLATION(ip1->cr, ip2->cr, ratio);
  op->cb = LINEAR_INTERPOLATION(ip1->cb, ip2->cb, ratio);
} /* trigger_interpolate_sce_vectors */

/** trigger_sce_get_triangles
 *
 *  @sce: sce handle
 *  @chroma_ptr: default configuration from Chromatix Header
 *  @cct_type:  cct type
 *
 *  Get triangles based on AWB/AEC decision
 *
 *  Return: none
 **/
boolean trigger_sce_get_triangles(sce40_t *sce,
  chromatix_SCE_type *p_sce, awb_cct_type cct_type)
{
  float awb_ratio = 0.0;

  switch (cct_type) {
    case AWB_CCT_TYPE_A:
      sce->orig = sce->origin_triangles_A;
      sce->interp_vector = p_sce->shift_vector_A;
      sce->dest = sce->destination_triangles_A;
    break;

    case AWB_CCT_TYPE_D65:
      sce->orig = sce->origin_triangles_D65;
      sce->interp_vector = p_sce->shift_vector_D65;
      sce->dest = sce->destination_triangles_D65;
    break;

    case AWB_CCT_TYPE_TL84_A:
      awb_ratio = GET_INTERPOLATION_RATIO(
        sce->trigger_info.mired_color_temp,
        sce->trigger_info.trigger_A.mired_start,
        sce->trigger_info.trigger_A.mired_end);

      trigger_interpolate_sce_triangles(
        &(sce->origin_triangles_TL84),
        &(sce->origin_triangles_A),
        &sce->orig,
        awb_ratio);

      trigger_interpolate_sce_triangles(
        &(sce->destination_triangles_TL84),
        &(sce->destination_triangles_A),
        &sce->dest,
        awb_ratio);

      trigger_interpolate_sce_vectors(&p_sce->shift_vector_TL84,
          &p_sce->shift_vector_A, &sce->interp_vector, awb_ratio);
      break;

    case AWB_CCT_TYPE_D65_TL84: {
      awb_ratio = GET_INTERPOLATION_RATIO(
        sce->trigger_info.mired_color_temp,
        sce->trigger_info.trigger_d65.mired_end,
        sce->trigger_info.trigger_d65.mired_start);

      trigger_interpolate_sce_triangles(&(sce->origin_triangles_D65),
        &(sce->origin_triangles_TL84),
        &sce->orig,
        awb_ratio);

      trigger_interpolate_sce_triangles(&(sce->destination_triangles_D65),
        &(sce->destination_triangles_TL84),
        &sce->dest,
        awb_ratio);

      trigger_interpolate_sce_vectors( &p_sce->shift_vector_D65,
        &p_sce->shift_vector_TL84,
        &sce->interp_vector, awb_ratio);
  }
    break;

  case AWB_CCT_TYPE_A_H: {
    awb_ratio = GET_INTERPOLATION_RATIO(
      sce->trigger_info.mired_color_temp,
      sce->trigger_info.trigger_H.mired_start,
      sce->trigger_info.trigger_H.mired_end);

    trigger_interpolate_sce_triangles(&(sce->origin_triangles_A),
      &(sce->origin_triangles_H),
      &sce->orig,
      awb_ratio);

    trigger_interpolate_sce_triangles(&(sce->destination_triangles_A),
      &(sce->destination_triangles_H),
      &sce->dest,
      awb_ratio);

    trigger_interpolate_sce_vectors(&p_sce->shift_vector_A,
      &p_sce->shift_vector_H, &sce->interp_vector, awb_ratio);
  }
    break;

  case AWB_CCT_TYPE_H:
    sce->orig = sce->origin_triangles_H;
    sce->interp_vector = p_sce->shift_vector_H;
    sce->dest = sce->destination_triangles_H;
  break;

  case AWB_CCT_TYPE_TL84:
  default:
    sce->orig = sce->origin_triangles_TL84;
    sce->interp_vector = p_sce->shift_vector_TL84;
    sce->dest = sce->destination_triangles_TL84;
    break;
  }

  ISP_DBG("%s: AWB cct type: %d awb ratio %f\n",
    __func__, cct_type, awb_ratio);

  sce_find_line_by_vector(&sce->interp_range.interpolation_line,
    &sce->interp_vector, &sce->dest.triangle1.point1);
  sce_found_boundaries(&sce->origin_triangles_A,
    &sce->interp_range.interpolation_line, &sce->interp_range.pos_step,
    &sce->interp_range.neg_step);
  ISP_DBG("%s: Interpolation parameter range -  %6.4lf to %6.4lf",
      __func__, sce->interp_range.neg_step, sce->interp_range.pos_step);

  return TRUE;
} /* trigger_sce_get_triangles */

/** sce40_config_cmd
 *
 *  @sce: sce handle
 *  @chroma_ptr: default configuration from Chromatix Header
 *
 *  Update SCE HW configuration based on orig triangles
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean sce40_config_cmd(sce40_t *sce,
  chromatix_parms_type *pchromatix)
{
  int32_t i;
  int32_t coeff[6];
  uint32_t matrix_shift, offset_shift;
  chromatix_SCE_type *chromatix_SCE = &pchromatix->chromatix_VFE.chromatix_SCE;
  double sce_adj_factor;
  float portrait_sce = sce->skin_color_boost_factor;
  boolean shift_from_orig = FALSE;
  cr_cb_triangle reorder_triangle;

  memset(&reorder_triangle, 0, sizeof(cr_cb_triangle));
  if (sce->portrait_severity != 0) {
    sce_adj_factor = sce->sce_adjust_factor *
      (1.0 - sce->portrait_severity / 255.0) +
      portrait_sce * (sce->portrait_severity / 255.0);
  } else {
    sce_adj_factor = sce->sce_adjust_factor;
  }

  ISP_DBG("%s: Portrait severity :%f",__func__, sce->portrait_severity);
  ISP_DBG("%s: Portrait adj factor :%f",__func__, portrait_sce);
  ISP_DBG("%s: sce_adj_factor :%f",__func__, sce_adj_factor);

  /* Step: 1 do affine transform, find coeff A B C D E F, shift
           2 re order vertex to counter clock wise */
  /*triangle 1 */
  ISP_DBG("affine_dbg triangle1");
  calc_sce_affine_params(&sce->interp_range,
    &sce->orig.triangle1, &sce->dest.triangle1,
    sce_adj_factor, coeff, &matrix_shift, &offset_shift, shift_from_orig);

  /*Cr: coef0 = affine_A coef1 = affine_B */
  sce->sce_cmd.crcoeff.coef00 = coeff[0];
  sce->sce_cmd.crcoeff.coef01 = coeff[1];
  /*Cb: coef0 = affine_D coef1 = affine_E */
  sce->sce_cmd.cbcoeff.coef00 = coeff[3];
  sce->sce_cmd.cbcoeff.coef01 = coeff[4];
  /*Cr_offset = affine_C,  Cb_offset = affine_F */
  sce->sce_cmd.croffset.offset0 = coeff[2];
  sce->sce_cmd.cboffset.offset0 = coeff[5];
  /*Cr_offset = affine_C,  Cb_offset = affine_F */
  sce->sce_cmd.croffset.shift0 = matrix_shift;
  sce->sce_cmd.cboffset.shift0 = offset_shift;

  /*triangle 2 */
  ISP_DBG("affine_dbg triangle2");
  calc_sce_affine_params(&sce->interp_range,
    &sce->orig.triangle2, &sce->dest.triangle2,
    sce_adj_factor, coeff, &matrix_shift, &offset_shift, shift_from_orig);

  /*Cr: coef0 = affine_A coef1 = affine_B */
  sce->sce_cmd.crcoeff.coef10 = coeff[0];
  sce->sce_cmd.crcoeff.coef11 = coeff[1];
  /*Cb: coef0 = affine_D coef1 = affine_E */
  sce->sce_cmd.cbcoeff.coef10 = coeff[3];
  sce->sce_cmd.cbcoeff.coef11 = coeff[4];
  /*Cr_offset = affine_C,  Cb_offset = affine_F */
  sce->sce_cmd.croffset.offset1 = coeff[2];
  sce->sce_cmd.cboffset.offset1 = coeff[5];
  /*Cr_offset = affine_C,  Cb_offset = affine_F */
  sce->sce_cmd.croffset.shift1 = matrix_shift;
  sce->sce_cmd.cboffset.shift1 = offset_shift;

  /* triangle 3 */
  ISP_DBG("affine_dbg triangle3");
  calc_sce_affine_params(&sce->interp_range,
    &sce->orig.triangle3, &sce->dest.triangle3,
    sce_adj_factor, coeff, &matrix_shift, &offset_shift, shift_from_orig);

  /*Cr: coef0 = affine_A coef1 = affine_B */
  sce->sce_cmd.crcoeff.coef20 = coeff[0];
  sce->sce_cmd.crcoeff.coef21 = coeff[1];
  /*Cb: coef0 = affine_D coef1 = affine_E */
  sce->sce_cmd.cbcoeff.coef20 = coeff[3];
  sce->sce_cmd.cbcoeff.coef21 = coeff[4];
  /*Cr_offset = affine_C,  Cb_offset = affine_F */
  sce->sce_cmd.croffset.offset2 = coeff[2];
  sce->sce_cmd.cboffset.offset2 = coeff[5];
  /*Cr_shift = matrix_shift,  Cb_shift = offset_shift */
  sce->sce_cmd.croffset.shift2 = matrix_shift;
  sce->sce_cmd.cboffset.shift2 = offset_shift;

  /*triangle 4 */
  ISP_DBG("affine_dbg triangle4");
  calc_sce_affine_params(&sce->interp_range,
    &sce->orig.triangle4, &sce->dest.triangle4,
    sce_adj_factor, coeff, &matrix_shift, &offset_shift, shift_from_orig);

  /*Cr: coef0 = affine_A coef1 = affine_B */
  sce->sce_cmd.crcoeff.coef30 = coeff[0];
  sce->sce_cmd.crcoeff.coef31 = coeff[1];
  /*Cb: coef0 = affine_D coef1 = affine_E */
  sce->sce_cmd.cbcoeff.coef30 = coeff[3];
  sce->sce_cmd.cbcoeff.coef31 = coeff[4];
  /*Cr_offset = affine_C,  Cb_offset = affine_F */
  sce->sce_cmd.croffset.offset3 = coeff[2];
  sce->sce_cmd.cboffset.offset3 = coeff[5];
  /*Cr_shift = matrix_shift,  Cb_shift = offset_shift */
  sce->sce_cmd.croffset.shift3 = matrix_shift;
  sce->sce_cmd.cboffset.shift3 = offset_shift;

  /*triangle 5 */
  ISP_DBG("affine_dbg2 triangle5");
  calc_sce_affine_params(&sce->interp_range,
    &sce->orig.triangle5, &sce->dest.triangle5,
    sce_adj_factor, coeff, &matrix_shift, &offset_shift, shift_from_orig);

  /*Cr: coef0 = affine_A coef1 = affine_B */
  sce->sce_cmd.crcoeff.coef40 = coeff[0];
  sce->sce_cmd.crcoeff.coef41 = coeff[1];
  /*Cb: coef0 = affine_D coef1 = affine_E */
  sce->sce_cmd.cbcoeff.coef40 = coeff[3];
  sce->sce_cmd.cbcoeff.coef41 = coeff[4];
  /*Cr_offset = affine_C,  Cb_offset = affine_F */
  sce->sce_cmd.croffset.offset4 = coeff[2];
  sce->sce_cmd.cboffset.offset4 = coeff[5];
  /*Cr_shift = matrix_shift,  Cb_shift = offset_shift */
  sce->sce_cmd.croffset.shift4 = matrix_shift;
  sce->sce_cmd.cboffset.shift4 = offset_shift;

  /* Update VFE with new co-efficients,
     for outside region mapping, no need to calculate inversion */
  sce_affine_transform_2d_double affine_Tx_double;
  affine_Tx_double.a = (double)chromatix_SCE->outside_region_mapping.a;
  affine_Tx_double.b = (double)chromatix_SCE->outside_region_mapping.b;
  affine_Tx_double.c = (double)chromatix_SCE->outside_region_mapping.c;
  affine_Tx_double.d = (double)chromatix_SCE->outside_region_mapping.d;
  affine_Tx_double.e = (double)chromatix_SCE->outside_region_mapping.e;
  affine_Tx_double.f = (double)chromatix_SCE->outside_region_mapping.f;
  calc_sce_shift_affine_coeff(&affine_Tx_double,
    coeff, &matrix_shift, &offset_shift);

  /*Cr: coef0 = affine_A coef1 = affine_B */
  sce->sce_cmd.crcoeff.coef50 = coeff[0];
  sce->sce_cmd.crcoeff.coef51 = coeff[1];
  /*Cb: coef0 = affine_D coef1 = affine_E */
  sce->sce_cmd.cbcoeff.coef50 = coeff[3];
  sce->sce_cmd.cbcoeff.coef51 = coeff[4];
  /*Cr_offset0 = affine_C,  Cb_offset1 = affine_F */
  sce->sce_cmd.croffset.offset5 = coeff[2];
  sce->sce_cmd.cboffset.offset5 = coeff[5];
  /*Cr_shift = matrix_shift,  Cb_shift = offset_shift */
  sce->sce_cmd.croffset.shift5 = matrix_shift;
  sce->sce_cmd.cboffset.shift5 = offset_shift;


  /* Cordintates (Cr, Cb) for Triangle 0-4 */
  /*  Triangle 1 */
  /*reorder triangel vertex to be counter clockwise*/
  ISP_DBG("order_dbg triangle1");
  sce_reorder_vertices(sce->orig.triangle1, &reorder_triangle);
  sce->sce_cmd.crcoord.vertex00 = reorder_triangle.point1.cr;
  sce->sce_cmd.cbcoord.vertex00 = reorder_triangle.point1.cb;

  sce->sce_cmd.crcoord.vertex01 = reorder_triangle.point2.cr;
  sce->sce_cmd.cbcoord.vertex01 = reorder_triangle.point2.cb;

  sce->sce_cmd.crcoord.vertex02 = reorder_triangle.point3.cr;
  sce->sce_cmd.cbcoord.vertex02 = reorder_triangle.point3.cb;

  /*  Triangle 2 */
  ISP_DBG("order_dbg triangle2");
  sce_reorder_vertices(sce->orig.triangle2, &reorder_triangle);
  sce->sce_cmd.crcoord.vertex10 = reorder_triangle.point1.cr;
  sce->sce_cmd.cbcoord.vertex10 = reorder_triangle.point1.cb;

  sce->sce_cmd.crcoord.vertex11 = reorder_triangle.point2.cr;
  sce->sce_cmd.cbcoord.vertex11 = reorder_triangle.point2.cb;

  sce->sce_cmd.crcoord.vertex12 = reorder_triangle.point3.cr;
  sce->sce_cmd.cbcoord.vertex12 = reorder_triangle.point3.cb;

  /*  Triangle 3 */
  ISP_DBG("order_dbg triangle3");
  sce_reorder_vertices(sce->orig.triangle3, &reorder_triangle);
  sce->sce_cmd.crcoord.vertex20 = reorder_triangle.point1.cr;
  sce->sce_cmd.cbcoord.vertex20 = reorder_triangle.point1.cb;

  sce->sce_cmd.crcoord.vertex21 = reorder_triangle.point2.cr;
  sce->sce_cmd.cbcoord.vertex21 = reorder_triangle.point2.cb;

  sce->sce_cmd.crcoord.vertex22 = reorder_triangle.point3.cr;
  sce->sce_cmd.cbcoord.vertex22 = reorder_triangle.point3.cb;


  /*  Triangle 4 */
  ISP_DBG("order_dbg triangle4");
  sce_reorder_vertices(sce->orig.triangle4, &reorder_triangle);
  sce->sce_cmd.crcoord.vertex30 = reorder_triangle.point1.cr;
  sce->sce_cmd.cbcoord.vertex30 = reorder_triangle.point1.cb;

  sce->sce_cmd.crcoord.vertex31 = reorder_triangle.point2.cr;
  sce->sce_cmd.cbcoord.vertex31 = reorder_triangle.point2.cb;

  sce->sce_cmd.crcoord.vertex32 = reorder_triangle.point3.cr;
  sce->sce_cmd.cbcoord.vertex32 = reorder_triangle.point3.cb;

  /*  Triangle 5 */
  ISP_DBG("order_dbg triangle5");
  sce_reorder_vertices(sce->orig.triangle5, &reorder_triangle);
  sce->sce_cmd.crcoord.vertex40 = reorder_triangle.point1.cr;
  sce->sce_cmd.cbcoord.vertex40 = reorder_triangle.point1.cb;

  sce->sce_cmd.crcoord.vertex41 = reorder_triangle.point2.cr;
  sce->sce_cmd.cbcoord.vertex41 = reorder_triangle.point2.cb;

  sce->sce_cmd.crcoord.vertex42 = reorder_triangle.point3.cr;
  sce->sce_cmd.cbcoord.vertex42 = reorder_triangle.point3.cb;

  return TRUE;
} /* sce40_config_cmd */

/** sce40_store_hw_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @lineariztion: sce handle
 *
 *  Create hw update list and store in isp sub module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean sce40_store_hw_update(isp_sub_module_t *isp_sub_module,
  sce40_t *sce)
{
  boolean                        ret = TRUE;
  int                            i = 0;
  struct msm_vfe_cfg_cmd_list   *hw_update = NULL;
  struct msm_vfe_cfg_cmd2       *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd    *reg_cfg_cmd = NULL;
  ISP_Skin_enhan_ConfigCmdType  *sce_cmd = NULL;

  if (!isp_sub_module || !sce) {
    ISP_ERR("failed: %p %p", isp_sub_module, sce);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  sce_cmd = (ISP_Skin_enhan_ConfigCmdType *)CAM_MALLOC(sizeof(*sce_cmd));
  if (!sce_cmd) {
    ISP_ERR("failed: sce_cmd %p", sce_cmd);
    goto ERROR_SCE_CMD;
  }
  memset(sce_cmd, 0, sizeof(*sce_cmd));

  /* prepare config cmd for hw update list */
  *sce_cmd = sce->sce_cmd;

  cfg_cmd->cfg_data = (void *)sce_cmd;
  cfg_cmd->cmd_len = sizeof(*sce_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  /* set dmi to proper sce bank */
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_SCE40_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_SCE40_LEN * sizeof(uint32_t);

  sce40_print_config(sce_cmd);

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(sce_cmd);
ERROR_SCE_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* sce40_store_hw_update */

/** sce40_trigger_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean sce40_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                        ret = TRUE;
  sce40_t                       *sce = NULL;
  isp_sub_module_output_t       *output = NULL;
  isp_private_event_t           *private_event = NULL;
  int8_t                         module_enable;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (!isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  sce = (sce40_t *)isp_sub_module->private_data;
  if (!sce) {
    ISP_ERR("failed: sce %p", sce);
    goto ERROR;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
    isp_sub_module->chromatix_module_enable) {
    module_enable = (isp_sub_module->manual_ctrls.tonemap_mode ==
                     CAM_TONEMAP_MODE_CONTRAST_CURVE) ? FALSE : TRUE;
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      output->stats_params->
        module_enable_info.reconfig_needed = TRUE;
      output->stats_params->module_enable_info.
        submod_enable[isp_sub_module->hw_module_id] = module_enable;
      output->stats_params->module_enable_info.
        submod_mask[isp_sub_module->hw_module_id] = 1;

      if (!isp_sub_module->submod_enable) {
        PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
        return ret;
      }
    }
  }

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG("no trigger update for SCE, enabled %d, trig_enable %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto ERROR;
  }

  if (isp_sub_module->trigger_update_pending == TRUE) {
    ret = sce40_select_table(isp_sub_module, sce);
    if (ret == FALSE) {
      ISP_ERR("failed: sce40_select_table");
      goto ERROR;
    }
    ret = sce40_config_cmd(sce, isp_sub_module->chromatix_ptrs.chromatixPtr);
    if (ret == FALSE) {
      ISP_ERR("failed: sce40_config_cmd");
      goto ERROR;
    }
    ret = sce40_store_hw_update(isp_sub_module, sce);
    if (ret == FALSE) {
      ISP_ERR("failed: sce40_store_hw_update");
      goto ERROR;
    }
    isp_sub_module->trigger_update_pending = FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable) {
    ret =   sce40_fill_vfe_diag_data(sce, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: sce40_fill_vfe_diag_data");
    }
  }

  if (output &&
      output->meta_dump_params &&
      output->metadata_dump_enable == 1) {
    output->meta_dump_params->frame_meta.adrc_info.color_drc_gain =
      sce->aec_update.color_drc_gain;
    output->meta_dump_params->frame_meta.adrc_info.exposure_time_ratio =
      sce->aec_update.hdr_exp_time_ratio;
    output->meta_dump_params->frame_meta.adrc_info.exposure_ratio =
      sce->aec_update.hdr_sensitivity_ratio;
    }

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* sce40_trigger_update */

/** sce_config
 *
 *  @isp_sub_module: isp sub module handle
 *  @sce: sce handle
 *
 *  config default params
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean sce40_config(isp_sub_module_t *isp_sub_module, sce40_t *sce)
{
  chromatix_parms_type *pchromatix = NULL;
  int32_t               coeff[6];
  uint32_t              matrix_shift;
  uint32_t              offset_shift;
  boolean               ret;

  if (!isp_sub_module || !sce) {
    ISP_ERR("failed: %p %p", isp_sub_module, sce);
    return FALSE;
  }

  pchromatix = (chromatix_parms_type *)
      isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!pchromatix) {
    ISP_ERR("failed: cpchromatix %p", pchromatix);
    return FALSE;
  }
  chromatix_3a_parms_type  *chromatix_3a_ptr = NULL;
  AAA_ASD_struct_type      *ASD_algo_data = NULL;

  chromatix_3a_ptr  = isp_sub_module->chromatix_ptrs.chromatix3APtr;
  if (!chromatix_3a_ptr) {
     ISP_ERR("failed: chromatix_3a_ptr %p", chromatix_3a_ptr);
     return FALSE;
  }
  ASD_algo_data = &chromatix_3a_ptr->ASD_3A_algo_data;

  sce->skin_color_boost_factor =
      ASD_algo_data->portrait_scene_detect.skin_color_boost_factor;
  sce_copy_triangles_from_chromatix(sce, &pchromatix->chromatix_VFE.chromatix_SCE);

  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
} /* sce40_config */

/** sce40_stats_aec_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean sce40_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t            *stats_update = NULL;
  sce40_t                   *sce = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sce = (sce40_t *)isp_sub_module->private_data;
  if (!sce) {
    ISP_ERR("failed: sce %p", sce);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if ((!F_EQUAL(sce->aec_update.real_gain, stats_update->aec_update.real_gain) ||
    !F_EQUAL(sce->aec_update.lux_idx, stats_update->aec_update.lux_idx))) {
    isp_sub_module->trigger_update_pending = TRUE;
  }
  sce->aec_update = stats_update->aec_update;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
} /* sce40_stats_aec_update */

/** sce40_stats_awb_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AWB update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean sce40_stats_awb_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t            *stats_update = NULL;
  sce40_t                   *sce = NULL;
  chromatix_parms_type      *pchromatix = NULL;
  chromatix_SCE_type        *chromatix_SCE;
  awb_cct_type               new_cct_type;
  cct_trigger_info          *trigger_info = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sce = (sce40_t *)isp_sub_module->private_data;
  if (!sce) {
    ISP_ERR("failed: sce %p", sce);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  pchromatix =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!pchromatix) {
    ISP_ERR("failed: chromatix_ptr %p", pchromatix);
    /* Put hw update flag to TRUE */
    isp_sub_module->trigger_update_pending = TRUE;

    return TRUE;
  }

  chromatix_SCE = &pchromatix->chromatix_VFE.chromatix_SCE;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  trigger_info = &sce->trigger_info;

  MIRED(stats_update->awb_update.color_temp, trigger_info->mired_color_temp);

  CALC_CCT_TRIGGER_MIRED(trigger_info->trigger_A,
    chromatix_SCE->SCE_A_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info->trigger_d65,
    chromatix_SCE->SCE_D65_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info->trigger_H,
    chromatix_SCE->SCE_H_trigger);

  /* get the cct type */
  new_cct_type = isp_sub_module_util_get_awb_cct_with_H_type(trigger_info);

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
      (sce->cur_cct_type != new_cct_type)) {
    ISP_DBG("AWB type prev %d new %d",
      sce->cur_cct_type, new_cct_type);
    isp_sub_module->trigger_update_pending = TRUE;

  }

  sce->cur_cct_type = new_cct_type;
  ISP_DBG("cur_cct_type %d", sce->cur_cct_type);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
} /* sce40_stats_awb_update */

/** sce40_stats_asd_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle ASD update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean sce40_stats_asd_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t      *stats_update = NULL;
  sce40_t             *sce = NULL;
  float               cur_portrait_severity;
  float               new_portrait_severity;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sce = (sce40_t *)isp_sub_module->private_data;
  if (!sce) {
    ISP_ERR("failed: sce %p", sce);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cur_portrait_severity = sce->portrait_severity;
  new_portrait_severity = stats_update->asd_update.portrait_severity;

  if (cur_portrait_severity != new_portrait_severity) {
    isp_sub_module->trigger_update_pending = TRUE;
  }
  sce->portrait_severity = new_portrait_severity;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
} /* sce40_stats_asd_update */

/** sce40_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Store chromatix ptr and make initial configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean sce40_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                ret = TRUE;
  sce40_t               *sce = NULL;
  modulesChromatix_t    *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  chromatix_ptrs = (modulesChromatix_t *)data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  sce = (sce40_t *)isp_sub_module->private_data;
  if (!sce) {
    ISP_ERR("failed: sce %p", sce);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  if (!isp_sub_module->chromatix_module_enable) {
     if( (sce->sce_adjust_factor >= 1.0) ||
         (sce->sce_adjust_factor <= -1.0) ) {
        isp_sub_module->submod_enable = TRUE;
        isp_sub_module->update_module_bit= TRUE;
    }
  }


  ret = sce40_config(isp_sub_module, sce);
  if (ret == FALSE) {
    ISP_ERR("failed: sce40_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* sce40_set_chromatix_ptr */

/** sce40_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean sce40_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  sce40_t *sce = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sce = (sce40_t *)isp_sub_module->private_data;
  if (!sce) {
    ISP_ERR("failed: sce %p", sce);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(sce, 0, sizeof(*sce));
  isp_sub_module->trigger_update_pending = FALSE;
  sce->prev_cct_type = AWB_CCT_TYPE_TL84;
  sce->cur_cct_type = AWB_CCT_TYPE_TL84;
  sce->sce_adjust_factor = 0.0;
  sce->prev_sce_adj = 0.0;
  sce->prev_aec_ratio = 0.0;
  sce->prev_lighting = TRIGGER_NORMAL;
  sce->portrait_severity = 0.0;
  sce->prev_color_drc_gain = 0.0;
  sce->orig = sce->origin_triangles_TL84;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* sce40_streamoff */

/** sce_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the sce module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean sce40_init(isp_sub_module_t *isp_sub_module)
{
  sce40_t *sce = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  sce = (sce40_t *)CAM_MALLOC(sizeof(sce40_t));
  if (!sce) {
    ISP_ERR("failed: sce %p", sce);
    return FALSE;
  }

  memset(sce, 0, sizeof(*sce));

  isp_sub_module->private_data = (void *)sce;
  isp_sub_module->manual_ctrls.tonemap_mode = CAM_TONEMAP_MODE_FAST;
  sce->prev_cct_type = AWB_CCT_TYPE_TL84;
  sce->cur_cct_type = AWB_CCT_TYPE_TL84;
  sce->sce_adjust_factor = 0.0;
  sce->prev_sce_adj = 0.0;
  sce->prev_aec_ratio = 0.0;
  sce->prev_lighting = TRIGGER_NORMAL;
  sce->portrait_severity = 0.0;
  sce->prev_color_drc_gain= 0.0;
  sce->orig = sce->origin_triangles_TL84;
  sce->is_dest_match_orig = FALSE;
  sce->trigger_info.mired_color_temp = DEFAULT_COLOR_TEMP;

  return TRUE;
}/* sce40_init */

/** sce40_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void sce40_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module || !isp_sub_module->private_data) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }
  free(isp_sub_module->private_data);
  isp_sub_module->private_data = NULL;

  return;
} /* sce40_destroy */
