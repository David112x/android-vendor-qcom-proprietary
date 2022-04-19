/* sml_model_parser.h
 *
 * This file contains the structures needed for the updated
 * sound model library version 3 sound model. These structures
 * are used to combine multiple sound models together into a
 * big sound model version 3. These are used in STHAL to parse
 * out the individual sound models' raw data.
 *
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef __SML_MODEL_PARSER_H__
#define __SML_MODEL_PARSER_H__

#if defined(__cplusplus)
extern "C" {
#endif

enum {
    SML_PARSER_SUCCESS = 0,
    SML_PARSER_ERROR = 1,
    SML_PARSER_NOT_SUPPORTED_VERSION,
    SML_PARSER_ID_NOT_EXIST,
    SML_PARSER_NOT_ALIGNED_SIZE,
    SML_PARSER_SIZE_MISSMATCH,
    SML_PARSER_VALUE_MISSMATCH,
    SML_PARSER_REF_UNINIT_VALUE,
    SML_PARSER_OUT_OF_RANGE,
    SML_PARSER_WRONG_MODEL,
    SML_PARSER_WRONG_VALUE,
    SML_PARSER_DELETING_LAST_KEYWORD,
    SML_PARSER_KEYWORD_NOT_FOUND,
    SML_PARSER_USER_NOT_FOUND,
    SML_PARSER_USER_ALREADY_EXIST,
    SML_PARSER_KEYWORD_USER_PAIR_EXIST,
    SML_PARSER_KEYWORD_USER_PAIR_NOT_EXIST,
    SML_PARSER_ACTIVE_USER_REMOVE_UDK,
    SML_PARSER_KEYWORD_ALREADY_EXIST,
    SML_PARSER_NOTHING_TO_CHANGE,
};

enum {
    SML_COMPARATOR_ERROR = -1,
    SML_COMPARATOR_SAME = 0,
    SML_COMPARATOR_DIFF = 1,
};

enum {
    SML_V3_MAX_PAYLOAD = 10000000,
    SML_V3_MIN_KEYWORDS = 1,
    SML_V3_MAX_KEYWORDS = 1,
    SML_V3_MIN_USERS    = 0,
    SML_V3_MAX_USERS    = 1,
};

enum {
    SML_GLOBAL_HEADER_MAGIC_NUMBER = 0x00180cc8,    // SML03
    SML_MAX_MODEL_NUM = 3,
    SML_MAX_STRING_LEN = 200,
};

typedef enum {
    ST_SM_ID_SVA_NONE     = 0x0000,
    ST_SM_ID_SVA_GMM      = 0x0001,
    ST_SM_ID_SVA_CNN      = 0x0002,
    ST_SM_ID_SVA_VOP      = 0x0004,
    ST_SM_ID_SVA_END      = 0x00F0,
    ST_SM_ID_CUSTOM_START = 0x0100,
    ST_SM_ID_CUSTOM_END   = 0xF000,
} listen_model_indicator_enum;

typedef struct _SML_GlobalHeaderType {
    uint32_t    magicNumber;                    // Magic number
    uint32_t    payloadBytes;                   // Payload bytes size
    uint32_t    modelVersion;                   // Model version
} SML_GlobalHeaderType;

typedef struct _SML_HeaderTypeV3 {
    uint32_t    numModels;                      // number of models
    uint32_t    keywordSpellLength;             // length of keyword spell ( include '\0' )
    uint32_t    userNameLength;                 // length of user name ( include '\0' )
    char    keywordSpell[SML_MAX_STRING_LEN];   // keyword spell
    char    userName[SML_MAX_STRING_LEN];       // user name
} SML_HeaderTypeV3;

typedef struct _SML_BigSoundModelTypeV3 {
    uint32_t version;                           // version of sound model ( always 3 for now )
    uint32_t offset;                            // start address of model data
    uint32_t size;                              // model size
    listen_model_indicator_enum type;           // type : Lower 1 byte : 1Stage KW model,
                                                //                       2Stage KW model,
                                                //                       2Stage User Model
                                                //        Upper 1 byte : 3rd Party - 1Stage KW model,
                                                //                       2Stage KW model,
                                                //                       2Stage User Model
}SML_BigSoundModelTypeV3;


typedef struct _SML_ModelTypeV3 {
    SML_HeaderTypeV3 header;                    // header for v3 model
    SML_BigSoundModelTypeV3 *modelInfo;         // model info, used for dynamic memory allocation.
    uint8_t* rawData;                           // actual model data
} SML_ModelTypeV3;

// all of model versions about SML
typedef enum _SML_ModelVersion {
    SML_MODEL_V2 = 0x0200,
    SML_MODEL_V3 = 0x0300,
} SML_ModelVersion;

// universial SML model structure
typedef struct _SML_ModelType {

    SML_GlobalHeaderType header;                // global header

    union _sml_model {
         SML_ModelTypeV3 SMLModelV3;             // sml3.0 -- kwihyuk
    } SMLModel;
} SML_ModelType;


/*
@brief decode to the SML model structure from serialized memory
*/
int sml_model_decoder(uint8_t *buffer, SML_ModelType *model);


#ifdef NON_VOICEWAKEUP_CSIM
/*
@brief create SML model through heap allocation
*/
SML_ModelType *sml_model_create(SML_ModelVersion smmlModelVersion);

/*
@brief used by encoder, merger, serializer, and migration to estimate serializing SML model size
*/
int sml_model_get_size(SML_ModelType *model);

/*
@brief serialize from encoded SML model structure
*/
int sml_model_serializer(SML_ModelType *model, uint8_t *buffer);


/*
@brief merge the base(merged) model and individual model such as GMMSVA, CNNSVA or GMMVoP.
*/
int sml_model_merger(uint8_t *binary_model, uint32_t model_size, SML_ModelType *merged_model, uint16_t indicator);


/*
@brief register user
*/
int sml_model_register_user(SML_ModelType *model, char *user_spell);


/*
@brief set keyword spell
*/
int sml_model_set_keyword_spell(SML_ModelType *model, char *keyword_spell);


/*
@brief delete specific keyword Model from combined(merged Model)
*/
int sml_model_deleter(SML_ModelType *merged_model, uint16_t indicator);


/*
@brief destroy SML model from heap allocation
*/
void sml_model_destroy(SML_ModelType *pModel);
#endif
#if defined(__cplusplus)
};
#endif

#endif

