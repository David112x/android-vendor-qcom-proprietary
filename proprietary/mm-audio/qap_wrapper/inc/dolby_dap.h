/*============================================================================
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

#define QAP_DAP_MAX_DAP_IO_CHANNELS 12
#define QAP_DAP_DEFAULT_BLOCK_SIZE 256
#define QAP_DAP_DEFAULT_MAX_BLOCKS	8

#define QAP_DAP_DIALOG_ENHANCER_CONFIG_PROPS 3
#define QAP_DAP_VOLUME_LEVELER_CONFIG_PROPS 4
#define QAP_DAP_VIRTUAL_BASS_PROPS 6
#define QAP_DAP_OUTPUT_MODE_CONFIG_PROPS 4

#define QAP_DAP_REGULATOR_TUNING_BANDS_MAX 20u
#define QAP_VIRTUAL_SUBGAIN_PARAMS 3

#define QAP_DAP_CPDP_IEQ_MAX_BANDS_NUM 20u

#define QAP_DAP_DEFAULT_SAMPLE_RATE 48000
#define QAP_DAP_DEFAULT_MI_PROCESS_DIS 0
#define QAP_DAP_DEFAULT_VIRT_BASS_EN 0
#define QAP_DAP_DEFAULT_MAX_OBJECT_COUNT 0
#define QAP_DAP_DEFAULT_DAP_SUPPORT_MODE 0
#define QAP_DAP_MAX_LICENSE_SIZE 25

#define QAP_DAP_DLB_BUFFER_SHORT_16 (4)
#define QAP_DAP_DLB_BUFFER_LONG_32  (6)
#define QAP_DAP_DLB_BUFFER_FLOAT    (7)
#define QAP_DAP_DLB_BUFFER_INT_LEFT (5)

#define QAP_DAP_CPDP_GRAPHIC_EQUALIZER_BANDS_MAX 20u

typedef enum {
	QAP_DAP_EXEC_CIDK = 0, QAP_DAP_EXEC_IIDK = 1
} dap_exec_mode_t;

/* channel forma */
typedef struct dap_channel_format {
	int ground_channel_num; /**< number of ground channels */
	int height_channel_num; /**< number of height channels */
	int lfe; /**< is lfe channel present    */

	int channel_num; /**< ground + height + lfe     */
} dap_channel_format_t;

/* Module params for DAP */
typedef enum {

	/* Dialog enhancer */
	QAP_DAP_PARAM_DIALOG_ENHANCER_SET,

	/* surround decoder enable */
	QAP_DAP_PARAM_SURROUND_DEC_ENABLE_SET,

	/* surround decoder centre spreading enable	*/
	QAP_DAP_PARAM_SURROUND_DEC_CENTRE_SPREADING_ENABLE_SET,

	/* volume leveler */
	QAP_DAP_PARAM_VOLUME_LEVELER_SET,

	/* virtual bass */
	QAP_DAP_PARAM_VIRTUAL_BASS_SET,

	/* amount of calibration for vol modeler */
	QAP_DAP_PARAM_VOL_MODELER_CALIBRATION_SET,

	/* set calibration boost */
	QAP_DAP_PARAM_CALIBRATION_BOOST_SET,

	/* enable volumemodeler */
	QAP_DAP_PARAM_VOL_MODELER_ENABLE_SET,

	/* output mode */
	QAP_DAP_PARAM_OUTPUT_MODE_SET,

	/* bass enhancer */
	QAP_DAP_PARAM_BASS_ENHANCER_SET,

	/* virtualizer */
	QAP_DAP_PARAM_VIRTUALIZER_SET,

	/* regulator tuning set */
	QAP_DAP_PARAM_REGULATOR_TUNING_SET,

	/* audio regulator set */
	QAP_DAP_PARAM_AUDIO_REGULATOR_SET,

	/* volume maximum boost */
	QAP_DAP_PARAM_VOL_MAX_BOOST_SET,

	/* volume maximum boost */
	QAP_DAP_PARAM_SYSTEM_GAIN_SET,

	/* set post gain */
	QAP_DAP_PARAM_POSTGAIN_SET,

	/* set pregain */
	QAP_DAP_PARAM_PREGAIN_SET,

	/* surround boost */
	QAP_DAP_PARAM_SURROUND_BOOST_SET,

	/* audio otpimizer enable */
	QAP_DAP_PARAM_AUDIO_OPTIMIZER_SET,

	/* process otpimizer enable */
	QAP_DAP_PARAM_PROCESS_OPTIMIZER_SET,

	/* set rear_height_filter_mode */
	QAP_DAP_PARAM_REAR_HEIGHT_FILTER_MODE,

	/* set rear_height_filter_mode */
	QAP_DAP_PARAM_HEIGHT_FILTER_MODE,

	/* media intelligence */
	QAP_DAP_PARAM_MEDIA_INTELLIGENCE_SET,

	/* intelligent equilizer */
	QAP_DAP_PARAM_INTELLIGEN_EQUILIZER_SET,

	/*output mode set */
	QAP_DAP_PARAM_OUTPUT_PRCOESSING_MODE_SET,

	/* set bass extraction enable */
	QAP_DAP_PARAM_BASS_EXTRACTOIN_EN,

	/* set bass extraction cutoff freq */
	QAP_DAP_PARAM_BASS_EXTRACTOIN_CUTOFF_FREQ,

	/* set bass extraction cutoff freq */
	QAP_DAP_PARAM_METAINFO_INPUT_FORMAT_SET,

	/* set graphic equilizer */
	QAP_DAP_PARAM_GRAPHIC_EQUILIZER_SET,

	/* Get process type */
	QAP_DAP_SESSION_PARAM_GET_PROCESSING_TYPE
} dolby_dap_param_t;

/* Structure to send parameter values from gstreamer to qap */
typedef struct {
	/* DAP param emums */
	dolby_dap_param_t id;
	void *data;
} qap_module_cmd_payload_t;

/* output rocessing mode qap enum list*/
typedef enum output_processing_mode {

	/* Enforces a mono (M) signal */
	QAP_DAP_CPDP_PROCESS_1 = 0,

	/* Enforces a stereo (L, R) signal */
	QAP_DAP_CPDP_PROCESS_2,

	/* Enforces a stereo (L, R) LtRt compatible signal */
	QAP_DAP_CPDP_PROCESS_2_LTRT,

	/* Enforces a 5.1 (L, R, C, LFE, Ls, Rs) signal */
	QAP_DAP_CPDP_PROCESS_5_1,

	/* Enforces a 5.1 (L, R, C, LFE, Ls, Rs) PLIIz compatible signal */
	QAP_DAP_CPDP_PROCESS_5_1_PLIIZ_COMPATIBLE,

	/* Enforces a 7.1 (L, R, C, LFE, Ls, Rs, Lrs, Rrs) signal */
	QAP_DAP_CPDP_PROCESS_7_1,

	/* Enforces a 5.1.2 (L, R, C, LFE, Ls, Rs, Ltm, Rtm) signal */
	QAP_DAP_CPDP_PROCESS_5_1_2,

	/* Enforces a 7.1.2 (L, R, C, LFE, Ls, Rs, Lrs, Rrs, Ltm, Rtm) */
	QAP_DAP_CPDP_PROCESS_7_1_2,

	/* Enforces a stereo (L, R) signal & not initialized supporting virtualizers */
	QAP_DAP_CPDP_PROCESS_2_HEADPHONE,

	/* Enforces a stereo (L, R) signal & not initialized supporting virtualizers */
	QAP_DAP_CPDP_PROCESS_2_HEADPHONE_HEIGHT,

	/* Enforces a 5.1 (L, R, C, LFE, Ls, Rs) signal */
	QAP_DAP_CPDP_PROCESS_5_1_SPEAKER,

	/* Enforces a 5.1.2 (L, R, C, LFE, Ls, Rs, Ltm, Rtm) signal */
	QAP_DAP_CPDP_PROCESS_5_1_2_SPEAKER,

	/* Enforces a 5.1.4 (L, R, C, LFE, Ls, Rs, Ltf, Rtf, Ltb, Rtb) signal */
	QAP_DAP_CPDP_PROCESS_5_1_4,

	/* Enforces a 7.1.4 (L, R, C, LFE, Ls, Rs, Lrs, Rrs, Ltf, Rtf, Ltb, Rtb) signal */
	QAP_DAP_CPDP_PROCESS_7_1_4,

	/* 5.1.4 (L, R, C, LFE, Ls, Rs, Ltf, Rtf, Ltb, Rtb) signal */
	QAP_DAP_CPDP_PROCESS_5_1_4_SPEAKER,

	/* Enforces a 7.1.4 (L, R, C, LFE, Ls, Rs, Lrs, Rrs, Ltf, Rtf, Ltb, Rtb) */
	QAP_DAP_CPDP_PROCESS_7_1_4_SPEAKER
} output_processing_mode_t;

typedef struct init_dap_configs {
	int32_t init_conf_mi_process_disable;
	int32_t init_conf_virtual_bass_process_enable;
	uint32_t init_conf_license_size;
	uint32_t init_conf_manufacturer_id;
	uint32_t init_conf_max_num_objects;
	int32_t init_conf_license_data[QAP_DAP_MAX_LICENSE_SIZE];
	int32_t init_conf_mode;
	int32_t execution_mode;

	dap_channel_format_t in_chan_fmt;
} init_dap_configs_t;

/* dialog enhancer props */
typedef struct dialog_enhancer {
	bool dialog_enhancer_enable;
	int32_t dialog_enhance_amt;
	int32_t dialog_enhance_ducking_amt;
	bool is_valid_de_en;
	bool is_valid_de_amt;
	bool is_valid_de_ducking;
} dialog_enhancer_t;

/* volumel leveler props */
typedef struct volume_leveler {
	bool volume_leveler_enable;
	int32_t volume_leveler_amount;
	int32_t volume_leveler_in_target;
	int32_t volume_leveler_out_target;
	bool is_valid_vol_lev_en;
	bool is_valid_vol_lev_amt;
	bool is_valid_vol_lev_in_target;
	bool is_valid_vol_lev_out_target;
} volume_leveler_t;

/* virtual bass props */
typedef struct virtual_bass {
	int32_t virtual_bass_low_mix_freq;
	int32_t virtual_bass_high_mix_freq;
	int32_t virtual_bass_subgains[QAP_VIRTUAL_SUBGAIN_PARAMS];
	int32_t virtual_bass_slope_gain;
	int32_t virtual_bass_overall_gain;
	int32_t virtual_bass_low_src_freq;
	int32_t virtual_bass_high_src_freq;
	int32_t virtual_bass_mode;

	bool is_valid_virtual_bass_mix_freq;
	bool is_valid_virtual_bass_subgains;
	bool is_valid_virtual_bass_slope_gain;
	bool is_valid_virtual_bass_overall_gain;
	bool is_valid_virtual_bass_src_freq;
	bool is_valid_virtual_bass_mode;
} virtual_bass_t;

/* virtulizer props */
typedef struct virt {
	uint32_t virtualizer_rear_height_speaker_angle;
	uint32_t virtualizer_height_speaker_angle;
	uint32_t virtualizer_rear_surround_speaker_angle;
	uint32_t virtualizer_surround_speaker_angle;
	uint32_t virtualizer_front_speaker_angle;

	bool is_valid_virtualizer_rear_height_speaker_angle;
	bool is_valid_virtualizer_height_speaker_angle;
	bool is_valid_virtualizer_rear_surround_speaker_angle;
	bool is_valid_virtualizer_surround_speaker_angle;
	bool is_valid_virtualizer_front_speaker_angle;

} virtualizer_t;

/* bass enhancer props */
typedef struct bass_enhan {
	bool bass_enhancer_enable;
	int32_t bass_enhancer_width_set;
	uint32_t bass_enhancer_cutoff_freq;
	int32_t bass_enhancer_boost_set;

	bool is_valid_bass_enhancer_enable;
	bool is_valid_bass_enhancer_width_set;
	bool is_valid_bass_enhancer_cutoff_freq;
	bool is_valid_bass_enhancer_boost_set;

} bass_enhancer_t;

/* audio regulator tuning */
typedef struct regulat {
	uint32_t nb_bands;
	uint32_t regulator_band_centers[QAP_DAP_REGULATOR_TUNING_BANDS_MAX];
	int32_t regulator_low_thresholds[QAP_DAP_REGULATOR_TUNING_BANDS_MAX];
	int32_t regulator_high_thresholds[QAP_DAP_REGULATOR_TUNING_BANDS_MAX];
	int32_t regulator_isolated_bands[QAP_DAP_REGULATOR_TUNING_BANDS_MAX];
} regulator_t;

/* output mode props */
typedef struct output {
	int32_t processing_mode;
	uint32_t nb_output_channels;
	int32_t *mix_matrix;
} output_mode_t;

/* audio regulator props */
typedef struct audio_reg {
	int32_t regulator_enable;
	int32_t regulator_speaker_dist_enable;
	int32_t regulator_relaxation_amount;
	int32_t regulator_overdrive;
	int32_t regulator_timbre_preservation;

	bool is_valid_regulator_enable;
	bool is_valid_regulator_speaker_dist_enable;
	bool is_valid_regulator_relaxation_amount;
	bool is_valid_regulator_overdrive;
	bool is_valid_regulator_timbre_preservation;
} audio_regulator_t;

/* media intelligence */
typedef struct media_int {
	int32_t mi_dialog_enhancer_en;
	int32_t mi_vol_leveler_en;
	int32_t mi_ieq_en;
	int32_t mi_surround_boost_en;

	bool is_valid_mi_dialog_enhancer;
	bool is_valid_mi_vol_leveler;
	bool is_valid_mi_ieq;
	bool is_valid_mi_surround_boost;
} media_intelligence_t;

typedef struct ieq_bands {
	uint32_t num_bands;
	uint32_t band_centers[QAP_DAP_CPDP_IEQ_MAX_BANDS_NUM];
	int32_t band_targets[QAP_DAP_CPDP_IEQ_MAX_BANDS_NUM];
} ieq_bands_t;

typedef struct intelligent_equ {
	int32_t ieq_en;
	int32_t ieq_set;
	ieq_bands_t ieq_bands_set;

	bool is_valid_ieq_en;
	bool is_valid_ieq_set;
	bool is_valid_ieq_bands_set;
} intelligent_equalizer_t;

typedef struct graphic_equ_bands_set {
	unsigned int nb_bands;
	unsigned int graphic_freq[QAP_DAP_CPDP_GRAPHIC_EQUALIZER_BANDS_MAX];
	int graphic_gains[QAP_DAP_CPDP_GRAPHIC_EQUALIZER_BANDS_MAX];
} graphic_bands_set_t;

typedef struct graphic_equ {
	int32_t graphic_en;
	graphic_bands_set_t graphic_bands_set;

	bool is_valid_graphic_en;
	bool is_valid_graphic_bands_set;
} graphic_equalizer_t;

typedef struct {
	/* Enable or disable the surround decoder. Depending
	 * on the input channels and the process mode component of the output mode,
	 * the surround decoder may not actually be active, even though it is enabled. */
	bool surrond_decoder_enable;

	/* Enables surround decoder spreading angle */
	bool surrond_decoder_cente_spreading_enable;

	/* dialog-enhancer	-	Array of Dialog Enhancer props which include
	 - Enable dialog enhancer
	 - Set amount for dialog enhancer, more the amount more aggresive amplification of dialog
	 - ducking other than dialog like music when dialog is detected. */
	dialog_enhancer_t dialog_enhance;

	/* enable/set Volume Leveler props */
	volume_leveler_t volume_level;

	/* enable/set Virtual Bass props */
	virtual_bass_t virt_bass;

	/* enable/set bass enhancer props */\

	bass_enhancer_t bass_enhancer;

	/* set virtualizer angles */
	virtualizer_t virtualizer;

	/* regulator tuning configuration */
	regulator_t regulator;

	/* set amount of Volume Modeler Calibration */
	int32_t volume_modeler_calibration;

	/* set amount of calibration boost */
	int32_t calibration_boost;

	/* volume-leveler	-  	Array of Volume Leveler which attenuates loud parts and boosts soft parts, which includes
	 - Enables volume-leveler
	 - input-target clip level
	 - output-target clip level
	 - amount specifies aggresiveness of volume leveler in reaching output target */
	bool volume_modeler_enable;

	/* audio otpimizer enable */
	bool audio_optimizer_enable;

	/* process otpimizer enable */
	bool process_optimizer_enable;

	/* set volume max boost */
	int32_t volmax_boost;

	/* set system gain */
	int32_t system_gain;

	/* set post gain */
	int32_t post_gain;

	/* set pre-gain */
	int32_t pregain;

	/* set surround boost */
	int32_t surround_boost;

	/* set rear_height_filter_mode */
	int32_t rear_height_filter_mode;

	/* set rear_height_filter_mode */
	int32_t height_filter_mode;

	/* audio regulator configuaration param set */
	audio_regulator_t audio_regulator;

	/* media intelligence */
	media_intelligence_t media_intelligence;

	/* out-mode can be configured by setting num of output channels(upto 12) and processing mode */
	output_mode_t output_mode;

	/* intelligent equilizer */
	intelligent_equalizer_t intelligent_equalizer;

	/* bass extraction enable */
	int32_t bass_extraction_en;

	/* bass extraction cutoff freq */
	int32_t bass_extraction_cutoff_frequency;

	/* outputmode for gst-launch */
	int32_t output_process_mode;

	/* outputmode for gst-launch */
	int32_t metainfo_input_format;

	/* graphic equilizer */
	graphic_equalizer_t graphic_equalizer;

} qap_dap_Config_t;

