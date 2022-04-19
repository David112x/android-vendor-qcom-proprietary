/* cpp_chromatix_int.h
*
* Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*/
#include "cpp_chromatix_int.h"

Chromatix_HDR_ASF_adj_type chromatix_hdr_adj_asf_params =
  {
      1, //  int                         enable;
      3, // tuning_control_type         control_hdr_asf;   // CONTROL_EXPOSURE_RATIO or CONTROL_EXP_TIME_RATIO
      // hdr_asf_adj_core_type       hdr_asf_adj_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
    {
       /* HDR ASF Adj Set1 */
      {
        /* exp ratio trigger data */
        {
          1.4325f,
          2.4324f,
        },

        /* exp time trigger data */
        {
          1.2325f,
          2.2324f,
        },

        1.0f,  // asf adj factor
      },
       /* HDR ASF Adj Set2 */
      {
        /* exp ratio trigger data */
        {
          1.4325f,
          2.4324f,
        },

        /* exp time trigger data */
        {
          1.2325f,
          2.2324f,
        },

        1.0f,  // asf adj factor
      },
       /* HDR ASF Adj Set3 */
      {
        /* exp ratio trigger data */
        {
          1.4325f,
          2.4324f,
        },

        /* exp time trigger data */
        {
          1.2325f,
          2.2324f,
        },

        1.0f,  // asf adj factor
      },
       /* HDR ASF Adj Set4 */
      {
        /* exp ratio trigger data */
        {
          1.4325f,
          2.4324f,
        },

        /* exp time trigger data */
        {
          1.2325f,
          2.2324f,
        },

        1.0f,  // asf adj factor
      },
       /* HDR ASF Adj Set5 */
      {
        /* exp ratio trigger data */
        {
          1.4325f,
          2.4324f,
        },

        /* exp time trigger data */
        {
          1.2325f,
          2.2324f,
        },

        1.0f,  // asf adj factor
      },
       /* HDR ASF Adj Set6 */
      {
        /* exp ratio trigger data */
        {
          1.4325f,
          2.4324f,
        },

        /* exp time trigger data */
        {
          1.2325f,
          2.2324f,
        },

        1.0f,  // asf adj factor
      },
    },
}
;

Chromatix_HDR_WNR_adj_type chromatix_hdr_wnr_adj_params =
{
  /* Chromatix_HDR_WNR_adj_type */

	1, // int                         enable;
    2, // tuning_control_type         control_hdr_wnr;                                    // CONTROL_EXPOSURE_RATIO or CONTROL_EXP_TIME_RATIO
    //hdr_wavelet_adj_core_type   hdr_wavelet_adj_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
    {
		/* HDR wavelet adj Set1 */
		{
			/* exp ratio trigger data */
			{
				1.4325f,
				2.4324f,
			},

			/* exp time trigger data */
			{
				1.2325f,
				2.2324f,
			},

			// wavelet_adj_type
			{
				//float   noise_profile_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
			},
		},
		/* HDR wavelet adj Set2 */
		{
			/* exp ratio trigger data */
			{
				1.4325f,
				2.4324f,
			},

			/* exp time trigger data */
			{
				1.2325f,
				2.2324f,
			},

			// wavelet_adj_type
			{
				//float   noise_profile_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
			},
		},
		/* HDR wavelet adj Set3 */
		{
			/* exp ratio trigger data */
			{
				1.4325f,
				2.4324f,
			},

			/* exp time trigger data */
			{
				1.2325f,
				2.2324f,
			},

			// wavelet_adj_type
			{
				//float   noise_profile_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
			},
		},
		/* HDR wavelet adj Set4 */
		{
			/* exp ratio trigger data */
			{
				1.4325f,
				2.4324f,
			},

			/* exp time trigger data */
			{
				1.2325f,
				2.2324f,
			},

			// wavelet_adj_type
			{
				//float   noise_profile_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
			},
		},
		/* HDR wavelet adj Set5 */
		{
			/* exp ratio trigger data */
			{
				1.4325f,
				2.4324f,
			},

			/* exp time trigger data */
			{
				1.2325f,
				2.2324f,
			},

			// wavelet_adj_type
			{
				//float   noise_profile_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
			},
		},
		/* HDR wavelet adj Set6 */
		{
			/* exp ratio trigger data */
			{
				1.4325f,
				2.4324f,
			},

			/* exp time trigger data */
			{
				1.2325f,
				2.2324f,
			},

			// wavelet_adj_type
			{
				//float   noise_profile_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
				//float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
				{1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
			},
		},
	},
};


Chromatix_ADRC_WNR_adj_type chromatix_adrc_wnr_adj_params =
{
  /* Chromatix_HDR_WNR_adj_type */

  0, // int                         enable;
    2, // tuning_control_type         control_hdr_wnr;                                    // CONTROL_EXPOSURE_RATIO or CONTROL_EXP_TIME_RATIO
    //hdr_wavelet_adj_core_type   hdr_wavelet_adj_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
    {
    /* HDR wavelet adj Set1 */
    {
      /* exp ratio trigger data */
      {
        1.4325f,
        2.4324f,
      },

      /* exp time trigger data */
      {
        1.2325f,
        2.2324f,
      },

      // wavelet_adj_type
      {
        //float   noise_profile_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
      },
    },
    /* HDR wavelet adj Set2 */
    {
      /* exp ratio trigger data */
      {
        1.4325f,
        2.4324f,
      },

      /* exp time trigger data */
      {
        1.2325f,
        2.2324f,
      },

      // wavelet_adj_type
      {
        //float   noise_profile_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
      },
    },
    /* HDR wavelet adj Set3 */
    {
      /* exp ratio trigger data */
      {
        1.4325f,
        2.4324f,
      },

      /* exp time trigger data */
      {
        1.2325f,
        2.2324f,
      },

      // wavelet_adj_type
      {
        //float   noise_profile_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
      },
    },
    /* HDR wavelet adj Set4 */
    {
      /* exp ratio trigger data */
      {
        1.4325f,
        2.4324f,
      },

      /* exp time trigger data */
      {
        1.2325f,
        2.2324f,
      },

      // wavelet_adj_type
      {
        //float   noise_profile_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
      },
    },
    /* HDR wavelet adj Set5 */
    {
      /* exp ratio trigger data */
      {
        1.4325f,
        2.4324f,
      },

      /* exp time trigger data */
      {
        1.2325f,
        2.2324f,
      },

      // wavelet_adj_type
      {
        //float   noise_profile_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
      },
    },
    /* HDR wavelet adj Set6 */
    {
      /* exp ratio trigger data */
      {
        1.4325f,
        2.4324f,
      },

      /* exp time trigger data */
      {
        1.2325f,
        2.2324f,
      },

      // wavelet_adj_type
      {
        //float   noise_profile_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
        //float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
        {1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f,},
      },
    },
  },
};

