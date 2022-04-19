/*===========================================================================

                     C H R O M A T I X Extension.

Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

{

     /******************************************************************************/
  // Feature name: HD Global Tone Map
  // Applicable chipset(s): MSM8998 and later with header revision 0x310 and later
  // Applicable operation mode:
  //     snapshot processing, and video modes.
  // GTM V2 is 2D control for GTM tuning, One of the dimension is based on exposure ratio ( aec sensitivity ratio ) and
  // 2nd dimension is either gain or lux index triggered. Total sets for GTM v2 are [MAX_SETS_FOR_GTM_EXP_ADJ][MAX_SETS_FOR_TONE_NOISE_ADJ]
  //
  // Variable name: int gtm_v2_enable
  // Description: Enables the feature. This flag has highest priority over GTM v1 structure.
  // Data range: 0 or 1.
  //
  // Variable name: tuning_control_type control_gtm_v2
  // Description: Gain , Luxindex to control GTM parameters in MAX_SETS_FOR_GTM_EXP_ADJ sets.
  // Data range: 0, 1
  //
  // Variable name: trigger_point2_type aec_sensitivity_ratio_v2
  // Description: Exposure ratio ( aec sensitivity ratio ) trigger , this is the inner(2nd) dimension for GTM V2 2D Control
  /******************************************************************************/

  /* GTM V2 */
  {
      1, /* GTM v2 Enable */
     1, /* Control Enable */
      2, /* Control Method - 1 = Gain, 0 = Lux Index */
     /* Data */
     {
        /* Data 1 */
        {
           /* Trigger */
           {
               1.000000f, /* Gain Start */
               1.990000f, /* Gain End */
              199, /* Lux Index Start */
              221, /* Lux Index End */
           },
           /* Exp Data */
           {
              /* Exp Data 1 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                    1.000000f, /* Start */
                     1.990000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.500000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     4.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 2 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     2.000000f, /* Start */
                     3.990000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.500000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     4.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 3 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     4.000000f, /* Start */
                     7.990000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.500000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     4.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 4 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     8.000000f, /* Start */
                     11.99000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.500000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     4.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 5 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     12.00000f, /* Start */
                     14.99000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.500000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     4.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 6 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     16.00000f, /* Start */
                     17.00000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.500000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     4.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
           },
        },
        /* Data 2 */
        {
           /* Trigger */
           {
               4.000000f, /* Gain Start */
               7.990000f, /* Gain End */
              100, /* Lux Index Start */
              111, /* Lux Index End */
           },
           /* Exp Data */
           {
              /* Exp Data 1 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                    1.000000f, /* Start */
                     1.990000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.150000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 2 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     2.000000f, /* Start */
                     3.990000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.150000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 3 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     4.000000f, /* Start */
                     7.990000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.150000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 4 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     8.000000f, /* Start */
                     11.99000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.150000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 5 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     12.00000f, /* Start */
                     14.99000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.150000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 6 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     16.00000f, /* Start */
                     17.00000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.150000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
           },
        },
        /* Data 3 */
        {
           /* Trigger */
           {
               12.00000f, /* Gain Start */
               14.99000f, /* Gain End */
               100, /* Lux Index Start */
               111, /* Lux Index End */
           },
           /* Exp Data */
           {
              /* Exp Data 1 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                    1.000000f, /* Start */
                     1.990000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.100000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 2 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     2.000000f, /* Start */
                     3.990000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.100000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 3 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     4.000000f, /* Start */
                     7.990000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.100000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                 },
              },
              /* Exp Data 4 */
              {
                 /* AEC Sensitivity Ratio Data */
                 {
                     8.000000f, /* Start */
                     11.99000f, /* End */
                 },
                 /* Core Light */
                 {
                     0.100000f, /* A MiddleTone */
                    0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                    0.999990f, /* Max Percentile */
                    0.000010f, /* Min Percentile */
                 },
                 /* Reserved Light */
                 {
                    1024, /* MaxVal Th */
                    64, /* Key Min Th */
                    4096, /* Key Max Th */
                    0.500000f, /* Key Hist Bin Weight */
                    16383, /* YOut MaxVal */
                    128, /* MinVal Threshold */
                    0.000000f, /* Reserved 1 */
                    0.000000f, /* Reserved 2 */
                  },
               },
               /* Exp Data 5 */
               {
                  /* AEC Sensitivity Ratio Data */
                  {
                     12.00000f, /* Start */
                     14.99000f, /* End */
                  },
                  /* Core Light */
                  {
                     0.100000f, /* A MiddleTone */
                     0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                     0.999990f, /* Max Percentile */
                     0.000010f, /* Min Percentile */
                  },
                  /* Reserved Light */
                  {
                     1024, /* MaxVal Th */
                     64, /* Key Min Th */
                     4096, /* Key Max Th */
                     0.500000f, /* Key Hist Bin Weight */
                     16383, /* YOut MaxVal */
                     128, /* MinVal Threshold */
                     0.000000f, /* Reserved 1 */
                     0.000000f, /* Reserved 2 */
                  },
               },
               /* Exp Data 6 */
               {
                  /* AEC Sensitivity Ratio Data */
                  {
                     16.00000f, /* Start */
                     17.00000f, /* End */
                  },
                  /* Core Light */
                  {
                     0.100000f, /* A MiddleTone */
                     0.920000f, /* Temporal W */
                     8.000000f, /* MiddleTone W */
                     0.999990f, /* Max Percentile */
                     0.000010f, /* Min Percentile */
                  },
                  /* Reserved Light */
                  {
                     1024, /* MaxVal Th */
                     64, /* Key Min Th */
                     4096, /* Key Max Th */
                     0.500000f, /* Key Hist Bin Weight */
                     16383, /* YOut MaxVal */
                     128, /* MinVal Threshold */
                     0.000000f, /* Reserved 1 */
                     0.000000f, /* Reserved 2 */
                  },
               },
            },
         },
      },
   },
}
