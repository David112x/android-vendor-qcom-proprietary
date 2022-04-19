/*============================================================================
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ============================================================================*/

#include <stdbool.h>
#include <stdint.h>

/*Payload IDs enum used to set and get property for a corresponding parameter
 * value given in qap_dts_decoder_config_params_t structure,There is one to one
 * mapping between payload IDs and payload value.If any particular parameter
 * need to be set,it can be set individually.There is no need to set all the
 * parameters present in  qap_dts_decoder_config_params_t structure */

typedef enum {
    QAP_DTS_BITWIDTH,                  //Takes int32_t value as payload data
    QAP_DTS_CONFIGINIT,                //Takes bool value as payload data
    QAP_DTS_EXTENSIONSUBSTREAMID,      //Takes int32_t value as payload data
    QAP_DTS_AUDIOPRESENTATION,         //Takes int32_t value as payload data
    QAP_DTS_SPKROUT,                   //Takes int32_t value as payload data
    QAP_DTS_MULTDMIXOUTPUT,            //Takes int32_t value as payload data
    QAP_DTS_SNDFLDSPKOUTINDEX,         //Takes int32_t value as payload data
    QAP_DTS_UPSAMPLERATE,              //Takes int32_t value as payload data
    QAP_DTS_DRCPERCENT,                //Takes int32_t value as payload data
    QAP_DTS_INTERPOLATE,               //Takes bool value as payload data
    QAP_DTS_DECODEONLY96KFROM192,      //Takes bool value as payload data
    QAP_DTS_DECODEONLYCORE,            //Takes bool value as payload data
    QAP_DTS_DECODEONLYCORESUBSTREAM,   //Takes bool value as payload data
    QAP_DTS_NODECDEFAULTSPKRREMAP,     //Takes bool value as payload data
    QAP_DTS_NODIALNORM,                //Takes bool value as payload data
    QAP_DTS_NODOWNMIX,                 //Takes bool value as payload data
    QAP_DTS_ENABLEDOWNMIX,             //Takes bool value as payload data
    QAP_DTS_LOSSY,                     //Takes bool value as payload data
    QAP_DTS_LOSSLESS,                  //Takes bool value as payload data
    QAP_DTS_SETSECMIXOUT,              //Takes int32_t value as payload data
    QAP_DTS_ANALOGCOMP,                //Takes bool value as payload data
    QAP_DTS_SETANALOGCOMPMAXUSERGAIN,  //Takes int32_t value as payload data
    QAP_DTS_AUTOSTARTFADEIN,           //Takes bool value as payload data
    QAP_DTS_DISABLENAVFADEIN,          //Takes bool value as payload data
    QAP_DTS_ES61MATRIX,                //Takes bool value as payload data
    QAP_DTS_ACTIVEASSETMASK,           //Takes string value as payload data
    QAP_DTS_ACTIVESECONDARYASSETMASK,  //Takes string value as payload data
    QAP_DTS_CLEANAUDIOUSER,            //Takes int32_t value as payload data
    QAP_DTS_CLEANAUDIOFRONT,           //Takes int32_t value as payload data
    QAP_DTS_CLEANAUDIOCENTER,          //Takes int32_t value as payload data
    QAP_DTS_CLEANAUDIOSURROUND,        //Takes int32_t value as payload data
    QAP_DTS_AUDIODESCUSER,             //Takes int32_t value as payload data
    QAP_DTS_AUDIODESCFRONT,            //Takes int32_t value as payload data
    QAP_DTS_AUDIODESCCENTER,           //Takes int32_t value as payload data
    QAP_DTS_AUDIODESCSURROUND,         //Takes int32_t value as payload data
    QAP_DTS_AUDIODESCFADE,             //Takes int32_t value as payload data
    QAP_DTS_AUDIODESCPAN,              //Takes int32_t value as payload data
    QAP_DTS_ENABLEAUDIODESC,           //Takes bool value as payload data
    QAP_DTS_DISABLEAUDIODESC,          //Takes bool value as payload data
    QAP_DTS_RESETFADEIN,               //Takes int32_t value as payload data
    QAP_DTS_RESETSPKRCONFIG,           //Takes int32_t value as payload data
    QAP_DTS_RESETDOWNMIXCONFIG,        //Takes string value as payload data
    QAP_DTS_RESETDECODENATIVE,         //Takes bool value as payload data
    QAP_DTS_RESETDECODECORE,           //Takes bool value as payload data
    QAP_DTS_RESETDECODECORESUBSTREAM,  //Takes bool value as payload data
    QAP_DTS_RESETSOLOPLAYBACK,         //Takes bool value as payload data
    QAP_DTS_INITIALIZEPLAYER,          //Takes bool value as payload data
    QAP_DTS_RESETSTREAMS,              //Takes bool value as payload data
    QAP_DTS_IGNOREUHD,                 //Takes bool value as payload data
    QAP_DTS_CLEAROBJECTGAINS,          //Takes bool value as payload data
    QAP_DTS_SETDIALOGCONTROL,          //Takes int32_t value as payload data
    QAP_DTS_ENABLEDIRECTMODE,          //Takes bool value as payload data
    QAP_DTS_DISABLEDIRECTMODE,         //Takes bool value as payload data
    QAP_DTS_ENABLEBLINDPARMA,          //Takes bool value as payload data
    QAP_DTS_DISABLEBLINDPARMA,         //Takes bool value as payload data
    QAP_DTS_NOBLINDPARMAFORSTREAMWITHOBJ,  //Takes bool value as payload data
    QAP_DTS_UPMIX,                     //Takes bool value as payload data
    QAP_DTS_DISABLEPARMADEC,           //Takes bool value as payload data
    QAP_DTS_REPLACESET,                //Takes qap_dts_decoder_active_replacement_set_t as payload data
    QAP_DTS_SETOBJECTGAIN_ARRAY,       //Takes qap_dts_decoder_object_gain_t as payload data
    QAP_DTS_POSTPROCESSPCM,            //Takes bool value as payload data
    QAP_DTS_POSTPROCESSAFTERDECODE,    //Takes bool value as payload data
    QAP_DTS_TLA_TESTING,               //Takes bool value as payload data
} qap_dts_decoder_module_cmd_payload_id_t;

/*Active channel set and replacement channel structure*/

typedef struct {
    const char *replaceset;
    const char *nactchset;
} qap_dts_decoder_active_replacement_set_t;

/*Object gain structure */

typedef struct {
    const char* objectgain_id_array;
    const char* objectgain_array;
    int32_t setobjectgain_numobjects;
} qap_dts_decoder_object_gain_t;

/*dts configuration parameter,Any configuration parameter can be set for below
 * variables corresponding to above configuration IDS,Last 3 properties are
 * not set to DTS library.They are to support additional functionality  */

typedef struct {
    int32_t bitwidth;
    bool configinit;
    int32_t extensionsubstreamid;
    int32_t audiopresentation;
    int32_t spkrout;
    int32_t multdmixoutput;
    int32_t sndfldspkoutindex;
    int32_t upsamplerate;
    int32_t drcpercent;
    bool interpolate;
    bool decodeonly96kfrom192;
    bool decodeonlycore;
    bool decodeonlycoresubstream;
    bool nodecdefaultspkrremap;
    bool nodialnorm;
    bool nodownmix;
    bool enabledownmix;
    bool lossy;
    bool lossless;
    int32_t setsecmixout;
    bool analogcomp;
    int32_t setanalogcompmaxusergain;
    bool autostartfadein;
    bool disablenavfadein;
    bool es61matrix;
    const char *activeassetmask;
    const char *activesecondaryassetmask;
    int32_t cleanaudiouser;
    int32_t cleanaudiofront;
    int32_t cleanaudiocenter;
    int32_t cleanaudiosurround;
    int32_t audiodescuser;
    int32_t audiodescfront;
    int32_t audiodesccenter;
    int32_t audiodescsurround;
    int32_t audiodescfade;
    int32_t audiodescpan;
    bool enableaudiodesc;
    bool disableaudiodesc;
    int32_t resetfadein;
    int32_t resetspkrconfig;
    const char *resetdownmixconfig;
    bool resetdecodenative;
    bool resetdecodecore;
    bool resetdecodecoresubstream;
    bool resetsoloplayback;
    bool initializeplayer;
    bool resetstreams;
    bool ignoreuhd;
    bool clearobjectgains;
    int32_t setdialogcontrol;
    bool enabledirectmode;
    bool disabledirectmode;
    bool enableblindparma;
    bool disableblindparma;
    bool noblindparmaforstreamwithobj;
    bool upmix;
    bool disableparmadec;
    qap_dts_decoder_active_replacement_set_t active_set;
    qap_dts_decoder_object_gain_t object_gain;
    bool postprocesspcm;
    bool postprocessafterdecode;
    bool tla_testing;
} qap_dts_decoder_config_params_t;

/* Payload for qap_module_cmd get/set param :
 * @data - to be used to send param specific data to decoder module
 * payload IDs for which configuration parameter
 * value has been has been modified/set
 */

typedef struct {
    qap_dts_decoder_module_cmd_payload_id_t id;  //ID for corresponding data
    void *data;          //It is used send any data
} qap_dts_decoder_module_cmd_payload_t;



