/**********************************************************************
* Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __FACEPROC_H__
#define __FACEPROC_H__

#include "img_common.h"
#include "fd_chromatix.h"

/**
 * CONSTANTS and MACROS
 **/

#define FACE_CONTOUR_DETECT 1
#define FACE_RECOGNITION 1
#define FACE_RECOGNIZE_TEST_REGISTER 0
#define FACE_PART_DETECT 1
#define FACE_BGS_DETECT 1
#define FACE_SMILE_DETECT 1

#define MAX_FACE_ROI 10
#define MAX_FACE_NUM 5
/*Max num of frames handled by FD at a time*/
#define MAX_NUM_FD_FRAMES 3
#define MAX_NUM_OFFLINE_FD_FRAMES 20
#define MIN_TRY_NUM_OFFLINE_FD_FRAMES 20

#define MAX_FACE_PART 12
#define FACE_EYE_CLOSED_THRESHOLD 600

/* Histogram size */
#define FD_PIXEL_BIT_WIDTH      8
#define FD_HIST_SIZE            (1<<FD_PIXEL_BIT_WIDTH)

#define FD_ROT_RANGE_MAX_SUPPORTED    0xffffffff

/* Configurations to FD - potentially stored as chromatix later */
#define MAX_FACES_TO_DETECT   MAX_FACE_ROI
#define MIN_FACE_SIZE         50
#define MAX_FACE_SIZE         500
#define DEFAULT_FACE_ANGLE    45
#define FACE_ANGLE_TO_DETECT  FD_ROT_RANGE_MAX_SUPPORTED

#define DEFAULT_DT_MODE  3
#define REGISTER_DT_MODE 1

/* Debug Face detection feature mask */
/* Face detection enable */
#define FACE_PROP_ENABLE_FD     (1 << 1)
/* Face detection facial parts on (smile, eyes */
#define FACE_PROP_PARTS_ON      (1 << 2)
/* Face angle on/off */
#define FACE_PROP_ANGLE_ON      (1 << 3)
/* Downscaling enable  */
#define FACE_PROP_DOWNSCALE_ON  (1 << 4)
/* Number of faces if set it will be 10 otherwise 5 */
#define FACE_PROP_MAX_FACES     (1 << 5)
/* Min face size in pixels 1- 25, 2 - 50, 3 - 100*/
#define FACE_PROP_MIN_FACE_SIZE_SHIFT 7
#define FACE_PROP_MIN_FACE_SIZE (3 << FACE_PROP_MIN_FACE_SIZE_SHIFT)

#define FACE_PROP_DEFAULT \
  (FACE_PROP_ENABLE_FD | FACE_PROP_ANGLE_ON \
  | (0<<FACE_PROP_MIN_FACE_SIZE_SHIFT) \
  | FACE_PROP_PARTS_ON)

#define FD_NUMBER_OF_ADDITIONAL_BUFFERS 1

/** Face Detection Metadata featurized mask:
 *
 * Mask to enable/disable sending split and featurized
 *   face info metadata related to face detection only
 *   facial parts only, or correspondint to other FD features.
 **/
#define FACE_INFO_MASK_FACE_DETECT        (1 << 1)
#define FACE_INFO_MASK_FACE_LANDMARKS     (1 << 2)
#define FACE_INFO_MASK_BLINK              (1 << 3)
#define FACE_INFO_MASK_SMILE              (1 << 4)
#define FACE_INFO_MASK_GAZE               (1 << 5)
#define FACE_INFO_MASK_CONTOUR            (1 << 6)
#define FACE_INFO_MASK_RECOG              (1 << 7)
#define FACE_INFO_MASK_ALL                0xffffffff

/* Debug directory path */
#define FACE_DEBUG_PATH "fd_test"

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_FACEPROC_SHIFT
/** FD_DLSYM_ERROR_RET:
 *
 * @p_lib: ponter to the faceproc lib
 * @fptr_name: name of the function pointer
 * @str_name: name of the symbol to be loaded
 *
 *   Helper macro to set the symbol and assign the pointer
 */
#define FD_DLSYM_ERROR_RET(p_lib, fptr_name, sym_name) ({\
  *(void **)&(p_lib->fns.fptr_name) = dlsym(p_lib->ptr_stub, sym_name); \
  if (p_lib->fns.fptr_name == NULL) { \
    IDBG_ERROR("%s:%d] Loading %s error %s", \
      __func__, __LINE__, sym_name, dlerror()); \
    return IMG_ERR_NOT_FOUND; \
  } \
})

/* Detected face in front pose angle in degrees */
#define FD_POSE_FRONT_ANGLE 0
/* Detected face in right diagonal pose angle in degrees */
#define FD_POSE_RIGHT_DIAGONAL_ANGLE 45
/* Detected face in right pose angle in degrees */
#define FD_POSE_RIGHT_ANGLE 90
/* Detected face in left diagonal pose angle in degrees */
#define FD_POSE_LEFT_DIAGONAL_ANGLE -45
/* Detected face in left pose angle in degrees */
#define FD_POSE_LEFT_ANGLE -90

/** faceproc_engine_t
 *   @FACEPROC_COMP_HW: HW faceproc component used
 *   @FACEPROC_ENGINE_DSP: DSP faceproc component used
 *   @FACEPROC_COMP_SW: SW faceproc component used
 *
 *   Faceproc engine selection
 **/
typedef enum {
  FACEPROC_ENGINE_HW,
  FACEPROC_ENGINE_DSP,
  FACEPROC_ENGINE_SW,
  FACEPROC_ENGINE_MAX
} faceproc_engine_t;

/** faceproc_frame_cfg_t
 *   @max_width: Maximum width allowed.
 *   @max_height: Maximum height allowed.
 *   @min_stride: Minimum stride allowed.
 *   @min_scanline:  Minimum scanline allowed.
 *
 *   faceproc frame configuration
 **/
typedef struct {
  uint32_t max_width;
  uint32_t max_height;
  uint32_t min_stride;
  uint32_t min_scanline;
} faceproc_frame_cfg_t;

/** fd_rect_t
 *   @x: left cordinate
 *   @y: top cordinate
 *   @dx: width of window
 *   @dy: height of the window
 *
 *   faceproc rect information
 **/
typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t dx;
  uint32_t dy;
} fd_rect_t;

/** faceproc_orientation_t
 *   FD_FACE_ORIENTATION_0: 0 degree orientation
 *   FD_FACE_ORIENTATION_90: 90 degree orientation
 *   FD_FACE_ORIENTATION_180: 180 degree orientation
 *   FD_FACE_ORIENTATION_270: 270 degree orientation
 *
 *   face orientation
 **/
typedef enum {
  FD_FACE_ORIENTATION_0 = 0,
  FD_FACE_ORIENTATION_90,
  FD_FACE_ORIENTATION_180,
  FD_FACE_ORIENTATION_270,
  FD_FACE_ORIENTATION_UNKNOWN,
} faceproc_orientation_t;

/** faceproc_region_info_t
 *   region_boundary: region boundary
 *   min_face_size: minimum face of the region
 *   weight: region priority read from chromatix
 *
 *   region infomation
 **/
typedef struct {
  fd_rect_t region_boundary;
  uint32_t min_face_size;
  float weight;
} faceproc_region_info_t;

/** faceproc_frame_cfg_t
 *   @min_face_size: minimum face size in pixels
 *   @max_face_size: maximum face size in pixels
 *   @max_num_face_to_detect: maximum number of faces to detect
 *   @face_orientation_hint: Hint on orientation of face w.r.t.
 *                         the frames. It's useful optional
 *                         information to the FD algorithms
 *                         that's usually derived from the
 *                         camera's sensor orientation together
 *                         with the data from orientation
 *                         sensor. If no hint is available, set
 *                         it to FD_FACE_ORIENTATION_UNKNOWN.
 *                         Algorithms not supporting this hint
 *                         will return error otherwise.
 *   @rotation_range: Faces inclined at this angle (front-facing
 *                  plane) on either side will be detected. In
 *                  units of degree. Use
 *                  FD_ROT_RANGE_MAX_SUPPORTED to request the
 *                  maximum supported range. Algorithms not
 *                  supporting specific ranges will return an
 *                  error during configuration if a specific
 *                  range other than FD_ROT_RANGE_MAX_SUPPORTED
 *                  is requested.
 *   @detection_mode: Face detection mode used only for SW FD. Available modes:
 *     1 - FULL Face detection makes full scan on every frame.
 *     2 - MOTION PARTITION: Full scan is done after every N frames,
 *       otherwise face tracking is used.
 *     3 - MOTION PROGRESS: On every scan only one partition is searched for
 *       new faces, for already detected faces tracking is used.
 *    @detection_threshold: Face detection threshold 0..9.
 *    @speed: Face detection speed parameter. If set to 0 speed will not be set.
 *      Range: Starting from 1 in incremental order (slower->faster).
 *    @use_sw_assisted_fd: Flag indicating to use sw assisted fd.
 *    @pending_buffs_number: Number of allowed pending buffers.
 *
 *   faceproc face configuration
 **/
typedef struct {
  uint32_t min_face_size;
  uint32_t max_face_size;
  uint32_t max_num_face_to_detect;
  faceproc_orientation_t face_orientation_hint;
  uint32_t rotation_range;
  uint32_t detection_mode;
  uint32_t detection_threshold;
  uint32_t speed;
  uint32_t use_sw_assisted_fd;
  uint32_t pending_buffs_number;
} faceproc_face_cfg_t;

/** faceproc_config_t
 *   @frame_cfg: frame configuration
 *   @face_cfg: face configuration
 *   @histogram_enable: flag to indicate if the histogram is
 *                    enabled
 *   @region_info: region definition for dymanic face detection
 *   @fd_feature_mask: need for debug bitmask with different
 *   combinations of enabled features.
 *   @fd_face_info_mask: mask determining what parts of
 *                     featurized FD Face Info metadata to be
 *                     sent to HAL.
 *
 *   faceproc configuration
 **/
typedef struct {
  faceproc_frame_cfg_t frame_cfg;
  faceproc_face_cfg_t face_cfg;
  faceproc_region_info_t region_info[FD_CHROMATIX_MAX_REGION_NUM];
  uint32_t histogram_enable;
  uint32_t fd_feature_mask;
  uint32_t fd_face_info_mask;
} faceproc_config_t;

/** fd_hist_t
 *   @bin: histogram bin
 *   @num_samples: number of samples
 *
 *   faceproc histogram
 **/
typedef struct {
  uint32_t bin[FD_HIST_SIZE];
  uint32_t num_samples;
} fd_hist_t;

/** fd_pixel_t
 *   @x: x cordinate
 *   @y: y cordinate
 *
 *   faceproc pixel information
 **/
typedef struct {
  int32_t x;
  int32_t y;
} fd_pixel_t;


/** face_part_detect
 *
 *   faceproc part type.
 *   Note:- this enumeration matches the Engine enumeration
 *   value. Any update in Engine APIs needs to be matched here.
 *
 **/
typedef enum {
  FACE_PART_LEFT_EYE = 0,  /* Center of left eye        */
  FACE_PART_RIGHT_EYE,     /* Center of right eye       */
  FACE_PART_MOUTH,         /* Mouth Center              */
  FACE_PART_LEFT_EYE_IN,   /* Inner corner of left eye  */
  FACE_PART_LEFT_EYE_OUT,  /* Outer corner of left eye  */
  FACE_PART_RIGHT_EYE_IN,  /* Inner corner of right eye */
  FACE_PART_RIGHT_EYE_OUT, /* Outer corner of right eye */
  FACE_PART_MOUTH_LEFT,    /* Left corner of mouth      */
  FACE_PART_MOUTH_RIGHT,   /* Right corner of mouth     */
  FACE_PART_NOSE_LEFT,     /* Left Nostril              */
  FACE_PART_NOSE_RIGHT,    /* Right Nostril             */
  FACE_PART_MOUTH_UP,      /* Mouth top                 */
} face_part_type_t;

/** face_part_detect
 *   @face_pt: cordinates for face part
 *   @face_pt_valid: Whether corresponding face point is valid or not
 *   @confidence: confidence value for face parts
 *   @direction_up_down: up down degree
 *   @direction_left_right: left right degree
 *   @direction_roll: roll degree
 *
 *   faceproc part information
 **/
typedef struct {
  fd_pixel_t face_pt[MAX_FACE_PART];
  uint8_t face_pt_valid[MAX_FACE_PART];
  int confidence[MAX_FACE_PART];
  int direction_up_down; // -180 to 179
  int direction_left_right;// -180 to 179
  int direction_roll; //-180 to 179
} face_part_detect;

/** fd_smile_detect
 *   @smile_degree: smile degree
 *   @confidence: confidence of the smile
 *
 *   faceproc smile information
 **/
typedef struct {
 int smile_degree;  //0 - 100
 int confidence;  // 0 -1000
} fd_smile_detect;

/** face_contour_point_eye_t
 *
 *   faceproc contour type ear.
 **/
typedef enum {
  FACE_CONTOUR_POINT_EYE_L_PUPIL,
  FACE_CONTOUR_POINT_EYE_L_IN,
  FACE_CONTOUR_POINT_EYE_L_OUT,
  FACE_CONTOUR_POINT_EYE_L_UP,
  FACE_CONTOUR_POINT_EYE_L_DOWN,
  FACE_CONTOUR_POINT_EYE_R_PUPIL,
  FACE_CONTOUR_POINT_EYE_R_IN,
  FACE_CONTOUR_POINT_EYE_R_OUT,
  FACE_CONTOUR_POINT_EYE_R_UP,
  FACE_CONTOUR_POINT_EYE_R_DOWN,
  FACE_CONTOUR_POINT_EYE_MAX
} face_contour_point_eye_t;

/** face_contour_point_forh_t
 *
 *   faceproc contour type forehead.
 **/
typedef enum {
  FACE_CONTOUR_POINT_FOREHEAD,
  FACE_CONTOUR_POINT_FOREHEAD_MAX
} face_contour_point_forh_t;

/** face_contour_point_nose_t
 *
 *   faceproc contour type nose.
 **/
typedef enum {
  FACE_CONTOUR_POINT_NOSE,
  FACE_CONTOUR_POINT_NOSE_TIP,
  FACE_CONTOUR_POINT_NOSE_L,
  FACE_CONTOUR_POINT_NOSE_R,
  FACE_CONTOUR_POINT_NOSE_L_0,
  FACE_CONTOUR_POINT_NOSE_R_0,
  FACE_CONTOUR_POINT_NOSE_L_1,
  FACE_CONTOUR_POINT_NOSE_R_1,
  FACE_CONTOUR_POINT_NOSE_MAX
} face_contour_point_nose_t;

/** face_contour_point_mouth_t
 *
 *   faceproc contour type mouth.
 **/
typedef enum {
  FACE_CONTOUR_POINT_MOUTH_L,
  FACE_CONTOUR_POINT_MOUTH_R,
  FACE_CONTOUR_POINT_MOUTH_UP,
  FACE_CONTOUR_POINT_MOUTH_DOWN,
  FACE_CONTOUR_POINT_MOUTH_MAX
} face_contour_point_mouth_t;

/** face_contour_point_lip_t
 *
 *   faceproc contour type lip.
 **/
typedef enum {
  FACE_CONTOUR_POINT_LIP_UP,
  FACE_CONTOUR_POINT_LIP_DOWN,
  FACE_CONTOUR_POINT_LIP_MAX
} face_contour_point_lip_t;

/** face_contour_point_brow_t
 *
 *   faceproc contour type brow.
 **/
typedef enum {
  FACE_CONTOUR_POINT_BROW_L_UP,
  FACE_CONTOUR_POINT_BROW_L_DOWN,
  FACE_CONTOUR_POINT_BROW_L_IN,
  FACE_CONTOUR_POINT_BROW_L_OUT,
  FACE_CONTOUR_POINT_BROW_R_UP,
  FACE_CONTOUR_POINT_BROW_R_DOWN,
  FACE_CONTOUR_POINT_BROW_R_IN,
  FACE_CONTOUR_POINT_BROW_R_OUT,
  FACE_CONTOUR_POINT_BROW_MAX
} face_contour_point_brow_t;

/** face_contour_point_chin_t
 *
 *   faceproc contour type chin.
 **/
typedef enum {
  FACE_CONTOUR_POINT_CHIN,
  FACE_CONTOUR_POINT_CHIN_L,
  FACE_CONTOUR_POINT_CHIN_R,
  FACE_CONTOUR_POINT_CHIN_MAX
} face_contour_point_chin_t;

/** face_contour_point_ear_t
 *
 *   faceproc contour type ear.
 **/
typedef enum {
  FACE_CONTOUR_POINT_EAR_L_DOWN,
  FACE_CONTOUR_POINT_EAR_R_DOWN,
  FACE_CONTOUR_POINT_EAR_L_UP,
  FACE_CONTOUR_POINT_EAR_R_UP,
  FACE_CONTOUR_POINT_EAR_MAX
} face_contour_point_ear_t;

/** contour_detect
 *   @is_eye_valid: Set if eye's information is available.
 *   @eye_pt: Eye's contour points.
 *   @is_forehead_valid: Set if forehead information is available.
 *   @forh_pt: Forehead contour points.
 *   @is_nose_valid: Set if nose information is available.
 *   @nose_pt: Nose contour points.
 *   @is_mouth_valid: Set if mouth information is available.
 *   @mouth_pt: Mouth contour points.
 *   @is_lip_valid: Set if lip information is available.
 *   @lip_pt: Lip contour points.
 *   @is_brow_valid: Set if brow information is available.
 *   @brow_pt: Brow contour points.
 *   @is_chin_valid: Set if chin information is available.
 *   @chin_pt: Chin contour points.
 *   @is_ear_valid: Set if sear information is available.
 *   @ear_pt: Ear contour points.
 *
 *   contour information
 **/
typedef struct {
  uint8_t is_eye_valid;
  fd_pixel_t eye_pt[FACE_CONTOUR_POINT_EYE_MAX];
  uint8_t is_forehead_valid;
  fd_pixel_t forh_pt[FACE_CONTOUR_POINT_FOREHEAD_MAX];
  uint8_t is_nose_valid;
  fd_pixel_t nose_pt[FACE_CONTOUR_POINT_NOSE_MAX];
  uint8_t is_mouth_valid;
  fd_pixel_t mouth_pt[FACE_CONTOUR_POINT_MOUTH_MAX];
  uint8_t is_lip_valid;
  fd_pixel_t lip_pt[FACE_CONTOUR_POINT_LIP_MAX];
  uint8_t is_brow_valid;
  fd_pixel_t brow_pt[FACE_CONTOUR_POINT_BROW_MAX];
  uint8_t is_chin_valid;
  fd_pixel_t chin_pt[FACE_CONTOUR_POINT_CHIN_MAX];
  uint8_t is_ear_valid;
  fd_pixel_t ear_pt[FACE_CONTOUR_POINT_EAR_MAX];
} contour_detect;

/** faceproc_info_t
 *   @face_boundary: face boundary
 *   @fp: face part information
 *   @ct: contour cordinates
 *   @sm: smile detec information
 *   @is_face_recognised: flag to indicate whether the face is
 *                      recognised
 *   @blink_detected: flag to indicate whether the blink is
 *                  detected
 *   @left_blink: value for the left blink
 *   @right_blink: value of the right blink
 *   @left_right_gaze: value of left-right gaze
 *   @top_bottom_gaze: value of top bottom gaze
 *   @unique_id: unique id for the faceproc output
 *   @gaze_angle: angle of gaze
 *   @face_angle_roll: Face angle (roll) 0-359 degrees.
 *   @fd_confidence: confidence of face detection
 *   @score: 0: discard this face, 1: keep this face in region filter
 *   @weight: priority of the face in the region, used to sort face in region filter
 *   @tracking: Whether this face is being tracked or a new face
 *
 *   faceproc information
 **/
typedef struct {
  fd_rect_t              face_boundary;
  face_part_detect       fp;
  contour_detect         ct;
  fd_smile_detect        sm;
  int                    is_face_recognised;  /* 1 or 0 */
  int                    blink_detected; /* 1 or 0 */
  int                    left_blink; // 0 - 1000
  int                    right_blink; // 0 - 1000

  /* left gaze gives +ve value; right gaze gives -ve value */
  int                    left_right_gaze;
  /*top gaze gives -ve, bottom gaze give +ve value */
  int                    top_bottom_gaze;
  int                    unique_id;
  int                    gaze_angle;// -90, -45, 0 , 45 ,90
  int                    face_angle_roll;
  int                    fd_confidence;
  uint32_t               score; /*if 1 or 0 */
  float                  weight; /* weight is used to sort face in region filter */
  int                    sw_fd_result;
  int                    tracking;
} faceproc_info_t;

/** faceproc_result_t
 *   @roi: result for each region of interest
 *   @num_faces_detected: number of faces detected
 *   @client_id: id provided by the client
 *   @frame_id: frame id of the frame
 *   @updated_result: fd algo result updated flag
 *
 *   faceproc information
 **/
typedef struct {
  faceproc_info_t roi[MAX_FACE_ROI];
  uint32_t num_faces_detected;
  int client_id;
  uint32_t frame_id;
  uint8_t updated_result;
} faceproc_result_t;

/** faceproc_mode_t
 *   FACE_DETECT_OFF: face detection is off
 *   FACE_DETECT: face detection mode
 *   FACE_RECOGNIZE: face recognition mode
 *   FACE_REGISTER: face registration mode
 *   CLEAR_ALBUM: album clear mode
 *   FACE_DETECT_LITE: face detection light mode
 *   FACE_DETECT_BSGC: Blink, Smile, Gaze detection
 *
 *   faceproc information
 **/
typedef enum {
  FACE_DETECT_OFF,
  FACE_DETECT,
  FACE_RECOGNIZE,
  FACE_REGISTER,
  CLEAR_ALBUM,
  FACE_DETECT_LITE,
  FACE_DETECT_BSGC,
} faceproc_mode_t;

/** faceproc_dump_mode_t
 *   FACE_FRAME_DUMP_OFF: Dump disabled.
 *   FACE_FRAME_DUMP_NON_TRACKED: Dump frames with non-tracked faces.
 *   FACE_FRAME_DUMP_DETECTED: Dump frames with tracked faces.
 *   FACE_FRAME_DUMP_NOT_DETECTED: Dump frames without detected faces.
 *   FACE_FRAME_DUMP_ALL: Dump all frames.
 *   FACE_FRAME_DUMP_NUMBERED_SET: Dump a set number of frames.
 *   FACE_FRAME_DUMP_ALL_DRAW_FACE_RECT: Dump all frames with
 *     face rects of the faces detected being drawn.
 *
 *   faceproc information
 **/
typedef enum {
  FACE_FRAME_DUMP_OFF = 0,
  FACE_FRAME_DUMP_NON_TRACKED,
  FACE_FRAME_DUMP_TRACKED,
  FACE_FRAME_DUMP_NOT_DETECTED,
  FACE_FRAME_DUMP_ALL,
  FACE_FRAME_DUMP_NUMBERED_SET,
  FACE_FRAME_DUMP_ALL_DRAW_FACE_RECT
} faceproc_dump_mode_t;

/** faceproc_profile_t
 *   FD_HANDLE_CREATE_TIME: Time taken for faceproc handles creation.
 *   FD_HANDLE_DELETE_TIME: Time taken for faceproc handles deletion.
 *   FD_FRAME_HW_DT_TIME: HW detection processing time.
 *   FD_FRAME_SW_DT_TIME: SW detection processing time.
 *   FD_FRAME_SW_FP_TIME: SW facial parts processing time.
 *   FD_FRAME_PROCESSING_TIME: Frame full processing time.
 *
 *   Index values while profiling FD
 **/
typedef enum {
  FD_HANDLE_CREATE_TIME = 0,
  FD_HANDLE_DELETE_TIME,
  FD_FRAME_HW_DT_TIME,
  FD_FRAME_SW_DT_TIME,
  FD_FRAME_SW_FP_TIME,
  FD_FRAME_PROCESSING_TIME,
} faceproc_profile_t;

/** faceproc_profile_t
 *   FD_HANDLE_CREATE_TIME: Time taken for faceproc handles creation.
 *   FD_HANDLE_DELETE_TIME: Time taken for faceproc handles deletion.
 *   FD_FRAME_HW_DT_TIME: HW detection processing time.
 *   FD_FRAME_SW_DT_TIME: SW detection processing time.
 *   FD_FRAME_SW_FP_TIME: SW facial parts processing time.
 *   FD_FRAME_PROCESSING_TIME: Frame full processing time.
 *
 *   faceproc information
 **/
typedef enum {
  FD_PROFILE_LEVEL_NONE = 0,
  FD_PROFILE_LEVEL_STREAM,
  FD_PROFILE_LEVEL_FRAME,
  FD_PROFILE_LEVEL_FACE,
  FD_PROFILE_LEVEL_ALL,
} faceproc_profile_level_t;

#define FRAME_BATCH_SIZE_MAX 2

/** faceproc_batch_mode_t
 *   FACE_FRAME_BATCH_MODE_OFF: batch mode off - default.
 *   FACE_FRAME_BATCH_MODE_ON: batch mode, comp processes
 *   multiple frames.
 *
 *   faceproc frame batch mode
 **/
typedef enum {
  FACE_FRAME_BATCH_MODE_OFF = 0,
  FACE_FRAME_BATCH_MODE_ON
} faceproc_batch_mode_t;

/** faceproc_face_criteria_t
 *   FACE_BIGGEST: Biggest face
 *   FACE_SMALLEST: Smallest face
 *   FACE_OLDEST: Oldest face
 *   FACE_NEWEST: Newest face
 *
 *   faceproc face criteria
 **/
typedef enum {
  FACE_BIGGEST = 0,
  FACE_SMALLEST,
  FACE_OLDEST,
  FACE_NEWEST
} faceproc_face_criteria_t;

/** faceproc_batch_mode_info_t
 *   @batch_mode: batch mode.
 *   @frame_batch_size: how many frames are processed together
 *
 *   faceproc frame batch mode information
 **/
typedef struct {
  faceproc_batch_mode_t batch_mode;
  uint32_t frame_batch_size;
} faceproc_batch_mode_info_t;

/** faceproc_debug_settings_t
*    @profile_fd_level: profile fd
*    @skip_fdui_updates: whether to skip fd ui updates
*    @fps_print_interval: fps print interval in seconds
*    @overwrite_d_o: whether to overwrite device_orientation information.
*        -1 : if no overwrite
*        device_orientation value if to overwrite
*
*    Data structure for fd specific debug settings
*
 **/
typedef struct {
  uint8_t profile_fd_level;
  bool skip_fdui_updates;
  uint8_t fps_print_interval;
  int32_t overwrite_d_o;
} faceproc_debug_settings_t;

/** faceproc_debug_settings_t
 *    @lux_index: Dynamically configurable lux index
 *
 **/
typedef struct {
  int32_t lux_index;
} faceproc_dynamic_cfg_params_t;

/** QWD_FACEPROC_CFG
 *
 *   datastructure passed needs to be of type faceproc_config_t
 **/
#define QWD_FACEPROC_CFG     (QIMG_FACEPROC_PARAM_OFF +  1)

/** QWD_FACEPROC_RESULT
 *
 *   datastructure passed needs to be of type faceproc_result_t
 **/
#define QWD_FACEPROC_RESULT  (QIMG_FACEPROC_PARAM_OFF +  2)

/** QWD_FACEPROC_MODE
 *
 *   datastructure passed needs to be of type faceproc_mode_t
 **/
#define QWD_FACEPROC_MODE    (QIMG_FACEPROC_PARAM_OFF +  3)

/** QWD_FACEPROC_CHROMATIX
 *
 *   datastructure passed needs to be of type fd_chromatix_t
 **/
#define QWD_FACEPROC_CHROMATIX   (QIMG_FACEPROC_PARAM_OFF +  4)

/** QWD_FACEPROC_BUFF_TYPE
 *
 *   datastructure passed needs to be of type img_buf_type_t
 **/
#define QWD_FACEPROC_BUFF_TYPE (QIMG_FACEPROC_PARAM_OFF + 5)

/** QWD_FACEPROC_TRY_SIZE
 *
 *   datastructure passed needs to be of type faceproc_frame_cfg_t
 **/
#define QWD_FACEPROC_TRY_SIZE (QIMG_FACEPROC_PARAM_OFF + 6)

/** QWD_FACEPROC_DUMP_DATA
 *
 *   data structure passed needs to be of type faceproc_dump_mode_t
 **/
#define QWD_FACEPROC_DUMP_DATA (QIMG_FACEPROC_PARAM_OFF + 7)

/** QWD_FACEPROC_USE_INT_BUFF
 *
 *   data structure passed needs to be of type uint32_t
 **/
#define QWD_FACEPROC_USE_INT_BUFF (QIMG_FACEPROC_PARAM_OFF + 8)

/** QWD_FACEPROC_BATCH_MODE_DATA
 *
 *   data structure passed needs to be of type
 *   faceproc_batch_mode_infor_t
 **/
#define QWD_FACEPROC_BATCH_MODE_DATA (QIMG_FACEPROC_PARAM_OFF + 9)

/** QWD_FACEPROC_DEBUG_SETTINGS
 *
 *   data structure passed needs to be of type
 *   faceproc_debug_settings_t
 **/
#define QWD_FACEPROC_DEBUG_SETTINGS (QIMG_FACEPROC_PARAM_OFF + 10)

/** QWD_FACEPROC_ORIENTATION_HINT
 *
 *   data structure passed needs to be of type
 *   int32_t
 **/
#define QWD_FACEPROC_ROTATION_HINT (QIMG_FACEPROC_PARAM_OFF + 11)

/** QWD_FACEPROC_UPDATE_CHROMATIX
 *
 *   data structure passed needs to be of type
 *   faceproc_dynamic_cfg_params_t
 **/
#define QWD_FACEPROC_DYNAMIC_UPDATE_CONFIG (QIMG_FACEPROC_PARAM_OFF + 12)

/** FD_PRINT_RECT:
   *  @p: fd rect
   *
   *  prints the crop region
   **/
#define FD_PRINT_RECT(p) ({ \
  IDBG_MED("%s:%d] FD crop info (%d %d %d %d)", __func__, __LINE__, \
    (p)->x, \
    (p)->y, \
    (p)->dx, \
    (p)->dy); \
})

/**
 * Function: faceproc_get_max_number_to_detect
 *
 * Description: max number of faces need to be detected by face component
 *
 * Input parameters:
 *  @p_header: fd chromatix tuning header
 *
 * Return values:
 *     number of face need to be detected
 *     by face detect component
 *
 * Notes: none
 **/
static inline uint32_t faceproc_get_max_number_to_detect(fd_chromatix_t* p_header) {
  if (p_header->region_filter.enable == 1) {
    return p_header->region_filter.max_face_num;
  } else {
    return p_header->max_num_face_to_detect;
  }
}
#endif //__FACEPROC_H__
