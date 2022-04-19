/**********************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

  /* enable */
  1,
  /*CONTRAST_CONTROL_Q4 */
  24,
  /*CHROMA DE SATURATION CONTROL WGT */
  /*0: Disable Chroma Saturation control,
  10: Max chroma desaturation control. Range 0 to 10*/
  5,
  /*CHROMA SATURATION CONTROL CLAMP BRIGHT AREA COLOR*/
  /*0: Disable Chroma Saturation control,
  40: Smaller the value, more the chroma de-saturation in Bright parts
  100: Bigger the value, lesser the chroma de-saturation in Bright parts*/
  80,
  /*CHROMA SATURATION CONTROL CLAMP DARK AREA COLOR*/
  /*0: Disable Chroma Saturation control,
  0: Smaller the value, lesser the chroma de-saturation in darker parts
  30: Bigger the value, more the chroma de-saturation in darker parts*/
  10,
  /* Feature points count */
  /* Minimum number feature points required to be detected
  in the input homography to apply HDR effect */
  10,
  /* Inliers count */
  /* Minimum number Inliers required to be detected
  in the input homography to apply HDR effect */
  15
  /*TO DO: Add more parameter need to be specified by customer*/

