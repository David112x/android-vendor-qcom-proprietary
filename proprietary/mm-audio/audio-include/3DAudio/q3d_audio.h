/*
* Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
* Not a Contribution.
* Apache license notifications and license are retained
* for attribution purposes only.
*/

/* Copyright 2016 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



/**
  @file q3d_audio.h

  @brief QTI Technologies 3D Audio Header File that provides an interface to 3D Audio Functionality.

   This file includes Interface definitions for developing VR Applications with
   3D Audio Support. This is the C API of a spatial audio
   rendering engine optimized for mobile VR.

   @htmlonly
   QTI Technologies 3D Audio Module allows the user to spatialize sound sources in 3D space,
   including distance and elevation cues. Specifically, the API's are capable of playing
   back spatial sound in three ways:

   - **Sound object rendering**: This allows the user to create a virtual sound
     source in 3D space. These sources, while spatialized, are fed with mono
     audio data.

   - **Ambisonic soundfields**: Ambisonic recordings are multi-channel audio
     files which are spatialized all around the listener in 360 degrees. These
     can be thought of as recorded or pre-baked soundfields. They can be of
     great use for background effects which sound perfectly spatial.  Examples
     include rain noise, crowd noise or even the sound of the ocean off to one
     side.

   - **Channel Audio**: The third method, referred to here as Multichannel Sounds,
     allow the user to directly play back non-spatialized sound upto 7.1 ch audio files.
     This is useful for music and other such audio.

   **Initialization**

       q3d_audio_context*
           q3d_audio_create(q3d_audio_rendering_mode rendering_mode);

   - q3d_audio_rendering_mode = Q3D_AUDIO_RENDERING_DIRECT is an enum which specifies
     "Direct Rendering of all Sounds".This uses the full HRTF data set and provide better quality
     than the virtual speaker rendering method.

   **3D Co-ordinate System**

   Start Space is right-handed, so when a viewer is positioned on the positive side of the Z axis and looking at the origin,
   they will see that the Y axis is 90 degrees
   counterclockwise from the X axis (illustrated below). \n
   * The Y axis points upwards (against gravity), Where +y is moving upwards and -y is moving down-wards. \n
   * The X and Z axes lie on the ground plane.\n
   * i.e +x is moving right and -x is moving left and respectively  +z is moving backward and -z is moving forward. \n
   Relative to the controller's pose when tracking starts, the X axis points to the right, and the Z axis points back. \n
   The axes do NOT move with the controller. They remain fixed. \n
   **Some examples of orientations are shown below(in the following format):**
   **Pose: Resulting rotation: (X, Y, Z, W)**

   * Initial pose, lying flat on table: None (identity) : (0,0,0,1) \n
   * Flat on table, rotated 90 deg to the left: 90 degrees about Y axis: (0,0.7071,0,0.7071) \n
   * Flat on table, rotated 90 deg to the right:-90 degrees about Y axis:(0,-0.7071,0,0.7071) \n
   * Flat on table, rotated 180 degrees (opposite to initial pose):180 degrees about Y axis:(0,1,0,0) \n
   * Pointing straight up:90 degrees about X axis: (0.7071,0,0,0.7071) \n
   * Pointing straight down:-90 degrees about X axis:(-0.7071,0,0,0.7071) \n
   * Rolled 90 degrees to the left:90 degrees about Z axis:(0,0,0.7071,0.7071) \n
   * Rolled 90 degrees to the right:-90 degrees about Z axis:0,0,-0.7071,0.7071 \n
   Notes: \n
   (1) All poses are given relative to the initial pose; they are not cumulative operations. \n
   (2) [X, Y, Z] is the vector component, W is the scalar component. Note that (X, Y, Z, W) expresses the same rotation as (-X, -Y, -Z, -W).
   @endhtmlonly
*/

#ifndef Q3D_AUDIO_PLATFORM_INCLUDE_H_
#define Q3D_AUDIO_PLATFORM_INCLUDE_H_


#if __ANDROID__
#include <jni.h>
#endif  // __ANDROID__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus





/**
   @name Q3D Audio Platform Version String
   @{
*/
/** Define to hold the version String. */
#define Q3D_AUDIO_PLATFORM_NDK_VERSION_STRING "1.0"
/** @} */ /* end_name Q3D Audio Platform Version String */


/**
   @name Q3D Audio Platform Version
   @{
*/
/** Major Version of the Spatial Audio Platform */
#define Q3D_AUDIO_PLATFORM_NDK_MAJOR_VERSION  (1)

/** Minor Version of the Spatial Audio Platform */
#define Q3D_AUDIO_PLATFORM_NDK_MINOR_VERSION  (0)
/** @} */ /* end_name Q3D Audio Platform Version  */

/**
   @name Q3D Audio Error Types
   @{
*/
/** Success. */
#define Q3D_AUDIO_SUCCESS               (0)
/** Unknown Error Occurred. */
#define Q3D_AUDIO_ERROR_UNKNOWN        (-1)
/** Bad Version Identified. */
#define Q3D_AUDIO_ERROR_BADVERSION     (-2)
 /** Invalid Parameter Passed. */
#define Q3D_AUDIO_ERROR_INVALIDPARAM   (-3)
/** A file with unsupported Sample Rate was Preloaded. */
#define Q3D_AUDIO_ERROR_BADSAMPLERATE  (-4)
/** Unsupported Number of Channels Passed. */
#define Q3D_AUDIO_ERROR_BADCHANNELS    (-5)
/** Insufficient Memory Available. */
#define Q3D_AUDIO_ERROR_NOMEMORY       (-6)
/** Not Supported. */
#define Q3D_AUDIO_ERROR_NOT_SUPPORTED  (-7)
/** @} */ /* end_name Q3D Audio Error Types */

/** @cond */
/** Q3D_AUDIO_PLATFORM_RESULT_DEFINED - to specify the result */
#ifndef Q3D_AUDIO_PLATFORM_RESULT_DEFINED
#define Q3D_AUDIO_PLATFORM_RESULT_DEFINED
/* @endcond **/

/** A int32 value specifying the result. */
 typedef int32_t q3d_audio_result;

/** @cond */
#endif
/* @endcond **/

 /** Version information for the Spatial Audio API. */
typedef struct q3d_audio_version {
  int32_t major; /**< Major Version Number */
  int32_t minor; /**< Minor Version Number */
} /** @cond */ q3d_audio_version /* @endcond **/;

/** Context of the Spatial Audio Engine. */
typedef struct q3d_audio_context_ q3d_audio_context;

/** Sound object and sound field and Sound Channel identifier. */
typedef int32_t q3d_audio_sound_id;

/**
   @name Q3D Audio Head Pose Data Types
   @{
*/
/** A floating point 3D vector to hold the head position. */
typedef struct q3d_audio_vec3f {
  float x; /**< x co-ordinate in X-Axis. */
  float y; /**< y co-ordinate in Y-Axis. */
  float z; /**< z co-ordinate in Z-Axis. */
} /** @cond */ q3d_audio_vec3f /* @endcond **/;

/**
  A floating point quaternion, in JPL format, to hold the head rotation

  We use this simple struct in order not to impose a dependency on a
  particular math library. The user of this API is free to encapsulate this
  into any math library they want.
*/
typedef struct q3d_audio_quatf {
  float qx; /**< qx the vector component. */
  float qy; /**< qy the vector component. */
  float qz; /**< qz the vector component. */
  float qw; /**< qw is the linear component. */
} /** @cond */ q3d_audio_quatf /* @endcond **/;

/**
  A floating point 4x4 matrix that holds head pose(includes both rotation & position) data.
*/
typedef struct q3d_audio_mat4f {
  float m[4][4]; /**< float value in 4x4 matrix to hold head pose data. */
} /** @cond */ q3d_audio_mat4f/* @endcond **/;
/** @} */ /* end_name Q3D Audio Head Pose Data Types  */

/**
   @name Q3D Audio Rendering Mode Type
   @{
*/
/** Rendering modes that define CPU Performance & Rendering quality */
typedef enum q3d_audio_rendering_mode {
  Q3D_AUDIO_RENDERING_DIRECT = 0, /**< Direct Rendering Mode. */
} /** @cond */ q3d_audio_rendering_mode /* @endcond **/;
/** @} */ /* end_name Q3D Audio Rendering Mode Type  */

/**
   @name Q3D Audio Wall Material Types
   @{
*/
/** Room surface material names, used to set room properties. */
typedef enum q3d_audio_material_type {
  Q3D_AUDIO_MATERIAL_TRANSPARENT = 0,                /**< Acoustically transparent material, reflects no sound. */
  Q3D_AUDIO_MATERIAL_ACOUSTIC_CEILING_TILES = 1,     /**< Acoustic ceiling tiles, absorbs most frequencies.*/
  Q3D_AUDIO_MATERIAL_BRICK_BARE = 2,                 /**< Bare brick, relatively reflective. */
  Q3D_AUDIO_MATERIAL_BRICK_PAINTED = 3,              /**< Painted brick */
  Q3D_AUDIO_MATERIAL_CONCRETE_BLOCK_COARSE = 4,      /**< Coarse surface concrete block. */
  Q3D_AUDIO_MATERIAL_CONCRETE_BLOCK_PAINTED = 5,     /**< Painted concrete block. */
  Q3D_AUDIO_MATERIAL_CURTAIN_HEAVY = 6,              /**< Heavy curtains. */
  Q3D_AUDIO_MATERIAL_FIBER_GLASS_INSULATION = 7,     /**< Fiber glass insulation. */
  Q3D_AUDIO_MATERIAL_GLASS_THIN = 8,                 /**< Thin glass. */
  Q3D_AUDIO_MATERIAL_GLASS_THICK = 9,                /**< Thick glass. */
  Q3D_AUDIO_MATERIAL_GRASS = 10,                     /**< Grass. */
  Q3D_AUDIO_MATERIAL_LINOLEUM_ON_CONCRETE = 11,      /**< Linoleum on concrete. */
  Q3D_AUDIO_MATERIAL_MARBLE = 12,                    /**< Marble. */
  Q3D_AUDIO_MATERIAL_METAL = 13,                     /**< Galvanized sheet metal. */
  Q3D_AUDIO_MATERIAL_PARQUET_ON_CONCRETE = 14,       /**< Wooden parquet on concrete. */
  Q3D_AUDIO_MATERIAL_PLASTER_ROUGH = 15,             /**< Rough plaster surface. */
  Q3D_AUDIO_MATERIAL_PLASTER_SMOOTH = 16,            /**< Smooth plaster surface. */
  Q3D_AUDIO_MATERIAL_PLYWOOD_PANEL = 17,             /**< Plywood panel. */
  Q3D_AUDIO_MATERIAL_POLISHED_CONCRETE_OR_TILE = 18, /**< Polished concrete OR tile surface. */
  Q3D_AUDIO_MATERIAL_SHEET_ROCK = 19,                /**< Sheet rock. */
  Q3D_AUDIO_MATERIAL_WATER_OR_ICE_SURFACE = 20,      /**< Surface of water or ice. */
  Q3D_AUDIO_MATERIAL_WOOD_CEILING = 21,              /**< Wooden ceiling. */
  Q3D_AUDIO_MATERIAL_WOOD_PANEL = 22,                /**< Wood paneling. */
} /** @cond */ q3d_audio_material_type /* @endcond **/;
/** @} */ /* end_name Q3D Audio Wall Material Types  */

/**
   @name Q3D Audio Distance Rolloff Types
   @{
*/
/** Distance rolloff models used for distance attenuation. */
typedef enum q3d_audio_distance_rolloff_type {
  Q3D_AUDIO_ROLLOFF_LOGARITHMIC = 0, /**< Logarithmic distance rolloff model. */
  Q3D_AUDIO_ROLLOFF_LINEAR = 1,      /**< Linear distance rolloff model. */
  Q3D_AUDIO_ROLLOFF_NONE = 2,        /**< No distance attenuation will be applied. */
} /** @cond */ q3d_audio_distance_rolloff_type /** @endcond */;
/** @} */ /* end_name Q3D Audio Distance Rolloff Types  */


/** Version of the Spatial Audio Platform. */
typedef q3d_audio_version Version;

/** 3D vector utilized to convey Position information. */
typedef q3d_audio_vec3f Vec3f;

/** Matrix Pose data to convey Position and Rotation information. */
typedef q3d_audio_mat4f Mat4f;

/** Quaternion to convey  Rotation Information. */
typedef q3d_audio_quatf Quatf;

/** Rendering modes define CPU Performance & Rendering quality . */
typedef q3d_audio_rendering_mode AudioRenderingMode;

/** Audio Material Names . */
typedef q3d_audio_material_type AudioMaterialName;

/** Source Id of the Rendering Sources. */
typedef q3d_audio_sound_id AudioSourceId;

/** @addtogroup Audio_C_API
  @{
*/

/**
 @latexonly \label{q3d_audio_create} @endlatexonly

 Function to create a instance of an engine to spatialize audio.This api will also intialize the audio engine.
 Note that the created instance must be deleted with #q3d_audio_destroy
 @param[in]
     rendering_mode Rendering modes define CPU Performance & Rendering quality
 @param[in]
     env The JNI Env associated with the current thread.
 @param[in]
     android_context The Android application context. This must be the
     application context, NOT an Activity context (Note: from any Android
     Activity in your app, you can call getApplicationContext() to
     retrieve the application context).
 @param[in]
     class_loader The class loader to use when loading Java
     classes. This must be your app's main class loader (usually
     accessible through activity.getClassLoader() on any of your Activities).
 @return
    Returns a unique id of the new sound object - upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: This is the first function that needs to called to \n
 create the context/instance of the engine.
*/
#ifdef __ANDROID__
q3d_audio_context* q3d_audio_create(JNIEnv* env, jobject android_context,
                                    jobject class_loader,
                                    q3d_audio_rendering_mode rendering_mode);
#else
q3d_audio_context* q3d_audio_create(q3d_audio_rendering_mode rendering_mode);
#endif  // #ifdef __ANDROID__


/**
 @latexonly \label{q3d_audio_get_version} @endlatexonly

 Function to query Version of 3D Audio NDK.
 @return
    Returns version information. #q3d_audio_version
 @dependencies
    Function Timing requirements:This function can be called at any time.
*/
q3d_audio_version q3d_audio_get_version();

/**
 @latexonly \label{q3d_audio_get_version_string} @endlatexonly

 Function to query Version of 3D Audio NDK in String Format
 @return
    Returns version information in string format.
 @dependencies
    Function Timing requirements:This function can be called at any time.
*/
const char* q3d_audio_get_version_string();

/**
   @name Q3D Audio Engine & Sound Specific Control APIs
   @{
*/

/**
 @latexonly \label{q3d_audio_destroy} @endlatexonly

 Destroys the Spatial Audio Engine. Make sure all preloaded files are unloaded to avoid leaks.
 @param[in]
    api pointer to the pointer of the spatial audio system context
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: This function is called at the end.\n
 as a last function to destroy in the instance of the engine.
*/
q3d_audio_result q3d_audio_destroy(q3d_audio_context** api);

/**
 @latexonly \label{q3d_audio_resume} @endlatexonly

 Resumes the Spatial Audio Engine. Resuming the engine when in resume state will be no-op.
 @param[in]
    api pointer to spatial audio system context.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: This function can be called at any time. \n
 after the spatialization process is going on.
*/
q3d_audio_result q3d_audio_resume(q3d_audio_context* api);

/**
 @latexonly \label{q3d_audio_pause} @endlatexonly

 Pauses the Spatial Audio Engine. All the sounds will be paused.
 Can be utilized to call this API if the app loses audio focus.
 @param[in]
    api pointer to spatial audio system context
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: This function can be called at any time \n
 after the spatialization process is going on.
*/
q3d_audio_result q3d_audio_pause(q3d_audio_context* api);

/**
 @latexonly \label{q3d_audio_update} @endlatexonly

 Call this API at regular rate, This function executes background
 operations outside of the audio thread.
 @param[in]
    api pointer to spatial audio system context
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: This function can be called at any time, and should be called at regular
    intervals.
*/
q3d_audio_result q3d_audio_update(q3d_audio_context* api);

/**
 @latexonly \label{q3d_audio_get_rendering_delay} @endlatexonly

 Provides the rendering latency information of Spatial Audio Engine.
 App can use this info to provide predicted head pose information to engine.
 @param[in]
    api pointer to spatial audio system context.
 @param[in]
    period_ms pointer to pass the latency period in milliseconds
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: This function is called at any time after engine creation.
*/
q3d_audio_result q3d_audio_get_rendering_delay(q3d_audio_context* api, uint32_t* period_ms);


/**
 @latexonly \label{q3d_audio_preload_soundfile} @endlatexonly

 Pre-load the local sound file. This is required to reduce the latency of the system.
 Each sound file needs to pre-loaded to the memory before calling Create.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    filename filename that is to be preloaded
 @return
    Returns TRUE upon success, FALSE if there are errors.
 @dependencies
 Function Timing requirements:
    This function can be called prior to creating objects/channels/sound fields.
*/
bool q3d_audio_preload_soundfile(q3d_audio_context* api, const char* filename);

/**
 @latexonly \label{q3d_audio_unload_soundfile} @endlatexonly

 Unload the previously preloaded file from spatial audio system memory.
 Make sure the stream has been stopped before calling unload sound file.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    filename filename that is to be unloaded
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
 This function cannot be called until all objects/channels/sound fields that were created with this
 filename have been stopped playing.
*/
q3d_audio_result q3d_audio_unload_soundfile(q3d_audio_context* api, const char* filename);


/**
 @latexonly \label{q3d_audio_create_sound_object} @endlatexonly

 Create the new sound object. Given filename must have been preloaded and should contain
 48Khz, 16bit Mono channel data. Returned handle gets invalidated automatically once the
 Stop Sound is called(if looping enabled) and once the file playback
 reach End of Stream (incase of looping disabled). loop parameter is passed via #q3d_audio_play_sound
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    filename filename that contains the appropriate PCM sound data that has been preloaded
 @return
    Returns a unique id of the new sound object - upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    This function cannot be called until the given filename has been preloaded.
*/
q3d_audio_sound_id q3d_audio_create_sound_object(q3d_audio_context* api,
                                                 const char* filename);

/**
 @latexonly \label{q3d_audio_create_soundfield} @endlatexonly

 Create the new sound field. Given filename has to be preloaded and should contain
 4 separate audio channel data with 48Khz/16bit representing first order ambisonics.
 Returned handle gets invalidated automatically once the
 Stop Sound is called(if looping enabled) and once the file playback
 reach End of Stream (incase of looping disabled). loop parameter is passed via #q3d_audio_play_sound
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    filename filename that contains the appropriate PCM sound data that has been preloaded.
 @return
    Returns a unique id of the new sound field - upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    This function cannot be called until the given filename has been preloaded.
*/
q3d_audio_sound_id q3d_audio_create_soundfield(q3d_audio_context* api,
                                               const char* filename);

/**
 @latexonly \label{q3d_audio_create_sound_channel} @endlatexonly

 Create the new channel source. These are rendered wth no spatialization. Supports upto 7.1 Channel
 Given filename must has to be preloaded and should contain
 upto 7.1 channel data at 48khz/16bit.  Returned handle gets invalidated automatically once the
 Stop Sound is called(if looping enabled) and once the file playback
 reach End of Stream (incase of looping disabled). loop parameter is passed via #q3d_audio_play_sound
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    filename filename that contains the appropriate PCM sound data that has been preloaded.
 @return
    Returns a unique id of the new sound object - upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    This function cannot be called until the given filename has been preloaded.
*/
q3d_audio_sound_id q3d_audio_create_sound_channel(q3d_audio_context* api,
                                                 const char* filename);



/**
 @latexonly \label{q3d_audio_play_sound} @endlatexonly

 Starts sound playback. Note that if this sound is not played with loopingnnnnn
 enabled, q3d_audio_sound_id is invalidated once the file reaches end of stream.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    sound_id identifier of the sound source
 @param[in]
    looping_enabled parameter to determine whether playback should happen in a loop.
     - if true, plays in a loop until the #q3d_audio_stop_sound is called.
     - if false, plays until the end of stream is reached.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after creation of the sound object.
*/
q3d_audio_result q3d_audio_play_sound(q3d_audio_context* api, q3d_audio_sound_id sound_id,
                          bool looping_enabled);


/**
 @latexonly \label{q3d_audio_pause_sound} @endlatexonly

 Pauses playback of the sound.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    sound_id identifier of the sound source
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after creation of the sound object.
*/
q3d_audio_result q3d_audio_pause_sound(q3d_audio_context* api, q3d_audio_sound_id sound_id);

/**
 @latexonly \label{q3d_audio_resume_sound} @endlatexonly

 Resumes playback of the sound.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    sound_id identifier of the sound source
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after creation of the sound object..
*/
q3d_audio_result q3d_audio_resume_sound(q3d_audio_context* api,
                            q3d_audio_sound_id sound_id);

/**
 @latexonly \label{q3d_audio_stop_sound} @endlatexonly

 Stops playback of the sound and invalidates the #q3d_audio_sound_id.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    sound_id identifier of the sound source
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after creation of the sound object.
*/
q3d_audio_result q3d_audio_stop_sound(q3d_audio_context* api, q3d_audio_sound_id sound_id);

/**
 @latexonly \label{q3d_audio_set_sound_volume} @endlatexonly

 Sets the volume of the sound.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    sound_id identifier of the sound source
 @param[in]
    volume Volume level of the sound. Should range from 0 (mute) to 1 (max).
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after creation of the sound object.
*/
q3d_audio_result q3d_audio_set_sound_volume(q3d_audio_context* api,
                                q3d_audio_sound_id sound_id, float volume);

/**
 @latexonly \label{q3d_audio_get_sound_volume} @endlatexonly

 Returns the present volume level of the sound.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    sound_id identifier of the sound source
 @param[out]
    volume Volume level of the sound. Shall range from 0 (mute) to 1 (max).
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after creation of the sound object.
*/
 q3d_audio_result q3d_audio_get_sound_volume(q3d_audio_context* api,
                                q3d_audio_sound_id sound_id, float& volume);

/**
 @latexonly \label{q3d_audio_is_sound_playing} @endlatexonly

 Returns TRUE if the sound is playing and FALSE otherwise.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    sound_id identifier of the sound source
 @return
    Returns True is the sound is playing, False otherwise.
 @dependencies
 Function Timing requirements:
    Typically called after the creation of the sound object.
*/
bool q3d_audio_is_sound_playing(const q3d_audio_context* api,
                                q3d_audio_sound_id sound_id);

/**
 @latexonly \label{q3d_audio_set_sound_object_position} @endlatexonly

 Set the sound object's position.
 @param[in]
    api  pointer to spatial audio system context
 @param[in]
    sound_object_id identifier of the sound object.
 @param[in]
    x  X co-ordinates in meters of the sound object in space.
 @param[in]
    y  Y co-ordinates in meters of the sound object in space.
 @param[in]
    z  Z co-ordinates in meters of the sound object in space.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: Typically called after creation of the sound object. \n
 If this API is not called to set a object position, Default position assumed is (0,0,0)
*/
q3d_audio_result q3d_audio_set_sound_object_position(q3d_audio_context* api,
                                         q3d_audio_sound_id sound_object_id,
                                         float x, float y, float z);

/**
 @latexonly \label{q3d_audio_set_sound_object_position_in_vec3f} @endlatexonly

 Set the sound object's position.
 @param[in]
    api  pointer to spatial audio system context
 @param[in]
    sound_object_id identifier of the sound object.
 @param[in]
    position   3D Vector representation of the position.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements
    Typically called after creation of the sound object.
*/
q3d_audio_result q3d_audio_set_sound_object_position_in_vec3f(q3d_audio_context* api,
                                         q3d_audio_sound_id sound_object_id,
                                         const q3d_audio_vec3f& position);

/**
 @latexonly \label{q3d_audio_get_sound_object_position_in_vec3f} @endlatexonly

 Returns the sound object's current position.
 @param[in]
    api  pointer to spatial audio system context
 @param[in]
    sound_object_id identifier of the sound object.
 @param[out]
    position 3D Vector representation of the sound object's position.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements
    Typically called after creation of the sound object.
*/
q3d_audio_result q3d_audio_get_sound_object_position_in_vec3f(q3d_audio_context* api,
                                         q3d_audio_sound_id sound_object_id,
                                         q3d_audio_vec3f& position);

/**
 @latexonly \label{q3d_audio_set_sound_object_distance_rolloff_model} @endlatexonly

 Set Object Roll off Model for distance-based volume attenuation.
 Maximum distance should be greater than Minimum distance for this api to be functional.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    sound_object_id identifier of the sound object.
 @param[in]
    rolloff_model can be set to Linear or logarithmic distance rolloff models. Note \n
     setting the rolloff model to |Q3D_AUDIO_ROLLOFF_NONE| will allow the user to \n
     perform their own distance attenuation model. @sa q3d_audio_distance_rolloff_type
 @param[in]
    min_distance Minimum distance is the distance at which the sound is played at full volume.
 @param[in]
    max_distance Maximum distance at which the sound has a sub-zero volume.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: Typically called after creation of the sound object.
*/
q3d_audio_result q3d_audio_set_sound_object_distance_rolloff_model(
    q3d_audio_context* api, q3d_audio_sound_id sound_object_id,
    int32_t rolloff_model, float min_distance, float max_distance);

/**
 @latexonly \label{q3d_audio_set_soundfield_rotation} @endlatexonly

 Set Sound Field Rotation
 @param[in]
    api   pointer to spatial audio system context
 @param[in]
    soundfield_id   identifier of the sound field
 @param[in]
    soundfield_rotation quaternion representation of soundfield's rotation.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after creation of the sound field.
*/
q3d_audio_result q3d_audio_set_soundfield_rotation(q3d_audio_context* api,
                                       q3d_audio_sound_id soundfield_id,
                                       const q3d_audio_quatf& soundfield_rotation);


/**
 @latexonly \label{q3d_audio_get_soundfield_rotation} @endlatexonly

 Returns the current sound field Rotation.
 @param[in]
    api   pointer to spatial audio system context
 @param[in]
    soundfield_id   identifier of the sound field.
 @param[out]
    soundfield_rotation quaternion of the rotation.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements
    Typically called after creation of the sound field.
*/
q3d_audio_result q3d_audio_get_soundfield_rotation(q3d_audio_context* api,
                                       q3d_audio_sound_id soundfield_id,
                                       q3d_audio_quatf& soundfield_rotation);
/** @} */ /* end_name Q3D Audio Engine & Sound Specific Control APIs  */


/**
   @name Head Pose Control APIs
   @{
*/

/**
 @latexonly \label{q3d_audio_set_head_position_in_vec3f} @endlatexonly

 Set Listener Head Position.
 @param[in]
    api   pointer to spatial audio system context
 @param[in]
    position   3D Vector representation of the position.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after engine creation.
*/
q3d_audio_result q3d_audio_set_head_position_in_vec3f(q3d_audio_context* api,
                                     const q3d_audio_vec3f& position);

/**
 @latexonly \label{q3d_audio_set_head_position} @endlatexonly

 Set Listener Head Position.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    x   X co-ordinate (in meters) of the listener head position.
 @param[in]
    y   Y co-ordinates (in meters) of the listener head position.
 @param[in]
    z   Z co-ordinates (in meters) of the listener head position.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: Typically called after engine creation. \n
 If this (or) other forms of head position API is not called to set head pose.
 Default position assumed is (0,0,0)
*/
q3d_audio_result q3d_audio_set_head_position(q3d_audio_context* api, float x, float y,
                                 float z);

/**
 @latexonly \label{q3d_audio_get_head_position_in_vec3f} @endlatexonly

 Returns the current Head Position of Listener.
 @param[in]
    api   pointer to spatial audio system context
 @param[out]
    position   3D Vector representation of the position.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: Typically called after engine creation.
 If this (or) other forms of head position API is not called to set head pose.
 Default position assumed i.e. (0,0,0)
*/
q3d_audio_result q3d_audio_get_head_position_in_vec3f(q3d_audio_context* api,
                                     q3d_audio_vec3f& position);

/**
 @latexonly \label{q3d_audio_set_head_rotation_in_quatf} @endlatexonly

 Set Listener Head Rotation.
 @param[in]
    api   pointer to spatial audio system context
 @param[in]
    rotation   Unit-length Quaternion representation of the listener's head rotation.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements: Typically called after engine creation.
 */
q3d_audio_result q3d_audio_set_head_rotation_in_quatf(q3d_audio_context* api,
                                     const q3d_audio_quatf& rotation);

/**
 @latexonly \label{q3d_audio_set_head_rotation} @endlatexonly

 Set Listener Head Rotation.
 @param[in]
    api   pointer to spatial audio system context
 @param[in]
    x,y,z,w   X,Y,Z,W co-ordinates defines the quaternion representation of the listener's head rotation.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after engine creation.
*/
q3d_audio_result q3d_audio_set_head_rotation(q3d_audio_context* api, float x, float y,
                                 float z, float w);


/**
 @latexonly \label{q3d_audio_get_head_rotation_in_quatf} @endlatexonly

 Returns the current Head Rotation of the listener.
 @param[in]
    api   pointer to spatial audio system context
 @param[out]
    rotation   Unit-length Quaternion representation of the listener head rotation.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements
    Typically called after engine creation.
*/
q3d_audio_result q3d_audio_get_head_rotation_in_quatf(q3d_audio_context* api,
                                     q3d_audio_quatf& rotation);


/** @} */ /* end_name Head Pose Control APIs   */

/**
   @name Reverb Zone Control APIs
   @{
*/
/**
 @latexonly \label{q3d_audio_enable_room} @endlatexonly

 Enable Shoebox reverb.
 @param[in]
    api pointer to spatial audio system context
 @param[in]
    enable TRUE to enable room reverb effects,  FALSE to disable them.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after engine creation.
*/
q3d_audio_result q3d_audio_enable_room(q3d_audio_context* api, bool enable);


/**
 @latexonly \label{q3d_audio_set_room_properties} @endlatexonly

 Set Room Properties i.e. Dimension(in Meters) and Room surface materials for reverb zone.
 Dimensions are always positive and states the distance between wall to wall./cieling to floor respectively.
 If this api is not called to set room properities. All properities of wall is defaulted to
 #Q3D_AUDIO_MATERIAL_TRANSPARENT.

 @param[in]
    api   pointer to spatial audio system context
 @param[in]
    size_x   Shoebox Dimension along the X axis.
 @param[in]
    size_y   Shoebox Dimension along the Y axis.
 @param[in]
    size_z   Shoebox Dimension along the Z axis.
 @param[in]
    wall_material wall material surface as stated in q3d_audio_material_type for all 4 walls.
 @param[in]
    ceiling_material ceiling material surface as stated in q3d_audio_material_type for ceiling.
 @param[in]
    floor_material floor material surface as stated in q3d_audio_material_type for floor.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after engine creation and before enabling the room.
 */
q3d_audio_result q3d_audio_set_room_properties(q3d_audio_context* api, float size_x,
                                   float size_y, float size_z,
                                   q3d_audio_material_type wall_material,
                                   q3d_audio_material_type ceiling_material,
                                   q3d_audio_material_type floor_material);

/**
 @latexonly \label{q3d_audio_set_room_reverb_adjustments} @endlatexonly

 Adjusts the properties of the currently active reverb zone, allowing changes to the
 reverb's gain, duration and low/high frequency balance
 @param[in]
    api  pointer to spatial audio system context
 @param[in]
    gain   Reverb volume (linear) adjustment. Use range [0, 1] for
                           attenuation, range [1, inf) for gain boost.
 @param[in]
    time_adjust   Reverb time adjustment multiplier to scale the
                           reverberation tail length. This value should be >= 0.
 @param[in]
    brightness_adjust Reverb brightness adjustment controls the
                           reverberation ratio across low and high frequency bands.
 @return
    Returns 0 upon success, < 0 if there are errors.\n
    see "Q3D Audio Error Types" in Defines section
 @dependencies
 Function Timing requirements:
    Typically called after engine creation.
*/
q3d_audio_result q3d_audio_set_room_reverb_adjustments(q3d_audio_context* api, float gain,
                                           float time_adjust,
                                           float brightness_adjust);

/** @} */ /* end_name Reverb Zone Control APIs   */

/** @} */  /* end_addtogroup Audio_C_API */

#ifdef __cplusplus
}  // extern "C"
#endif


/** Convenience C++ wrapper. */
#if defined(__cplusplus) && !defined(SAP_NO_CPP_WRAPPER)

#include <memory>
#include <string>

/** @addtogroup Audio_CPP_API
  @{
*/

/**
  Namespace Q3DAudioPlatform that provides a collection of C++ interfaces that wraps the 3D Audio C
  APIs.
*/
namespace Q3DAudioPlatform {
/**
  This is a convenience C++ wrapper for the Spatial Audio C API
  THREADING: this class is thread-safe and reentrant after Init() is called.
*/
class SpatialAudioApi {
 public:
  /**
   @latexonly \label{SpatialAudioApi()} @endlatexonly
    Creates an (uninitialized) ControllerApi object. You must initialize
    it by calling Init() before interacting with it.
  */
  SpatialAudioApi() : context_(0) {}

  ~SpatialAudioApi() {
    if (context_) {
      q3d_audio_destroy(&context_);
    }
  }

/**
 @latexonly \label{SpatialAudioApi::Init} @endlatexonly
  Creates and initializes a q3d_audio_context.
  For more information, @sa #q3d_audio_create().
*/
#ifdef __ANDROID__
  bool Init(JNIEnv* env, jobject android_context, jobject class_loader,
            AudioRenderingMode rendering_mode) {
    context_ =
        q3d_audio_create(env, android_context, class_loader, rendering_mode);
    return context_ != 0;
  }
#else
  bool Init(AudioRenderingMode rendering_mode) {
    context_ = q3d_audio_create(rendering_mode);
    return context_ != 0;
  }
#endif  // #ifdef __ANDROID__

  /**
    @latexonly \label{SpatialAudioApi::Pause} @endlatexonly
    Pauses the audio engine.
    For more information, @sa #q3d_audio_pause().
   */
  q3d_audio_result Pause() { return q3d_audio_pause(context_); }

  /**
    @latexonly \label{SpatialAudioApi::Resume} @endlatexonly
    Resumes the audio engine.
    For more information, @sa #q3d_audio_resume().
   */
  q3d_audio_result Resume() { return q3d_audio_resume(context_); }

  /**
    @latexonly \label{SpatialAudioApi::Update} @endlatexonly
    For more information, @sa #q3d_audio_update().
  */
  q3d_audio_result Update() { return q3d_audio_update(context_); }

  /**
    Preloads a local sound file.
    For more information, @sa #q3d_audio_preload_soundfile().
  */
  bool PreloadSoundfile(const std::string& filename) {
    return q3d_audio_preload_soundfile(context_, filename.c_str());
  }

  /**
    Unloads a previously preloaded sample from memory.
    For more information, @sa #q3d_audio_preload_soundfile().
  */
  q3d_audio_result UnloadSoundfile(const std::string& filename) {
    return q3d_audio_unload_soundfile(context_, filename.c_str());
  }

  /**
    Returns a new sound object.
    For more information, @sa #q3d_audio_create_sound_object().
  */
  AudioSourceId CreateSoundObject(const std::string& filename) {
    return q3d_audio_create_sound_object(context_, filename.c_str());
  }

  /**
    Returns a new sound field.
    For more information, @sa #q3d_audio_create_soundfield().
  */
  AudioSourceId CreateSoundfield(const std::string& filename) {
    return q3d_audio_create_soundfield(context_, filename.c_str());
  }

  /**
    Returns a new stereo sound.
    For more information, @sa #q3d_audio_create_sound_channel().
  */
  AudioSourceId CreateChannelSound(const std::string& filename) {
    return q3d_audio_create_sound_channel(context_, filename.c_str());
  }

  /**
    Starts playback of a sound.
    For more information, @sa #q3d_audio_play_sound().
  */
  q3d_audio_result PlaySound(AudioSourceId source_id, bool looping_enabled) {
    return q3d_audio_play_sound(context_, source_id, looping_enabled);
  }

  /**
    Pauses playback of a sound.
    For more information, @sa #q3d_audio_pause_sound().
  */
  q3d_audio_result PauseSound(AudioSourceId source_id) {
    return q3d_audio_pause_sound(context_, source_id);
  }

  /**
    Resumes playback of a sound.
    For more information, @sa #q3d_audio_resume_sound().
  */
  q3d_audio_result ResumeSound(AudioSourceId source_id) {
    return q3d_audio_resume_sound(context_, source_id);
  }

  /**
    Stops playback of a sound.
    For more information, @sa #q3d_audio_stop_sound().
  */
  q3d_audio_result StopSound(AudioSourceId source_id) {
    return q3d_audio_stop_sound(context_, source_id);
  }

  /**
    Repositions an existing sound object.
    For more information, @sa #q3d_audio_set_sound_object_position().
  */
  q3d_audio_result SetSoundObjectPosition(AudioSourceId sound_object_id, float x, float y,
                              float z) {
    return q3d_audio_set_sound_object_position(context_, sound_object_id, x, y, z);
  }

  /**
    Repositions an existing sound object.
    For more information, @sa #q3d_audio_set_sound_object_position_in_vec3f().
  */
  q3d_audio_result SetSoundObjectPositionInVec3f(AudioSourceId sound_object_id,
          const Vec3f& position) {
    return q3d_audio_set_sound_object_position_in_vec3f(context_, sound_object_id, position);
  }

  /**
    Returns the current position of an existing sound object.
    For more information, @sa #q3d_audio_get_sound_object_position_in_vec3f().
  */
  q3d_audio_result GetSoundObjectPositionInVec3f(AudioSourceId sound_object_id,
          Vec3f& position) {
    return q3d_audio_get_sound_object_position_in_vec3f(context_, sound_object_id, position);
  }

  /**
    Set Sound Object Roll Off Model for Distance Attenuation.
    For more information, @sa #q3d_audio_set_sound_object_distance_rolloff_model().
  */
  q3d_audio_result SetSoundObjectDistanceRolloffModel(
      AudioSourceId sound_object_id,
      q3d_audio_distance_rolloff_type rolloff_model, float min_distance,
      float max_distance) {
    return q3d_audio_set_sound_object_distance_rolloff_model(
        context_, sound_object_id, rolloff_model, min_distance, max_distance);
  }

  /**
    Sets the rotation an existing soundfield.
    For more information, @sa #q3d_audio_set_soundfield_rotation().
  */
  q3d_audio_result SetSoundfieldRotation(AudioSourceId soundfield_id,
                             const Quatf& soundfield_rotation) {
    return q3d_audio_set_soundfield_rotation(context_, soundfield_id,
                                      soundfield_rotation);
  }

  /**
    Returns the rotation of an existing soundfield.
    For more information, @sa #q3d_audio_get_soundfield_rotation().
  */
  q3d_audio_result GetSoundfieldRotation(AudioSourceId soundfield_id,
                             Quatf& soundfield_rotation) {
    return q3d_audio_get_soundfield_rotation(context_, soundfield_id,
                                      soundfield_rotation);
  }

  /**
    Changes the volume of an existing sound.
    For more information, @sa #q3d_audio_set_sound_volume().
  */
  q3d_audio_result SetSoundVolume(AudioSourceId source_id, float volume) {
    return q3d_audio_set_sound_volume(context_, source_id, volume);
  }

  /**
    Returns the volume set on an existing sound.
    For more information, @sa #q3d_audio_set_sound_volume().
  */
  q3d_audio_result GetSoundVolume(AudioSourceId source_id, float& volume) {
    return q3d_audio_get_sound_volume(context_, source_id, volume);
  }

  /**
    Checks if a sound is playing.
    For more information, @sa #q3d_audio_is_sound_playing().
  */
  bool IsSoundPlaying(AudioSourceId source_id) const {
    return q3d_audio_is_sound_playing(context_, source_id);
  }


  /**
    Sets the listener head position from a Vec3f.
    For more information, @sa #q3d_audio_set_head_position_in_vec3f().
  */
  q3d_audio_result SetHeadPositionInVec3f(const Vec3f& position) {
    return q3d_audio_set_head_position_in_vec3f(context_, position);
  }

  /**
    Sets the listener head position in 3 floats.
    For more information, @sa #q3d_audio_set_head_position().
  */
  q3d_audio_result SetHeadPosition(float x, float y, float z) {
    return q3d_audio_set_head_position(context_, x, y, z);
  }
  /**
    Sets the listener head rotation in 4 floats, which are unit-lenth Quaternions.
    For more information, @sa #q3d_audio_set_head_rotation().
  */
  q3d_audio_result SetHeadRotation(float qx, float qy, float qz, float qw) {
    return q3d_audio_set_head_rotation(context_, qx, qy, qz, qw);
  }

  /**
    Sets the listener head rotation via unit-length Quaternion representation.
    For more information, @sa #q3d_audio_set_head_rotation_in_quatf().
  */
  q3d_audio_result SetHeadRotationInQuatf(const Quatf& rotation) {
    return q3d_audio_set_head_rotation_in_quatf(context_, rotation);
  }

  /**
    Returns the head Position in Vec3f format.
    For more information, @sa #q3d_audio_set_head_position_in_vec3f().
  */
  q3d_audio_result GetHeadPositionInVec3f(Vec3f& position) {
    return q3d_audio_get_head_position_in_vec3f(context_, position);
  }

  /**
    Returns the head Rotation in unit-length Quaternion representation.
    For more information, @sa #q3d_audio_get_head_rotation_in_quatf().
  */
  q3d_audio_result GetHeadRotationInQuatf(Quatf& rotation) {
    return q3d_audio_get_head_rotation_in_quatf(context_, rotation);
  }

  /**
    Turns on/off the room reverberation effect.
    For more information, @sa #q3d_audio_enable_room().
  */
  q3d_audio_result EnableRoom(bool enable) {
      return q3d_audio_enable_room(context_, enable);
  }

  /**
    Sets the room properties describing the dimensions and surface materials
    of a given room. For more information, @sa
    #q3d_audio_set_room_properties().
  */
  q3d_audio_result SetRoomProperties(float size_x, float size_y, float size_z,
                         q3d_audio_material_type wall_material,
                         q3d_audio_material_type ceiling_material,
                         q3d_audio_material_type floor_material) {
    return q3d_audio_set_room_properties(context_, size_x, size_y, size_z,
                                  wall_material, ceiling_material,
                                  floor_material);
  }

  /** @name Wrapper manipulation
      @{ */
  /**
    Creates a C++ wrapper for a C object and takes ownership.
  */
  explicit SpatialAudioApi(q3d_audio_context* context)
      : context_(context) {}

  /**
    Returns the wrapped C object. Does not affect ownership.
  */
  q3d_audio_context* cobj() { return context_; }
  const q3d_audio_context* cobj() const { return context_; }

  /**
    Returns the wrapped C object and transfers its ownership to the caller.
    The wrapper becomes invalid and should not be used.
  */
  q3d_audio_context* Release() {
    q3d_audio_context* result = context_;
    context_ = 0;
    return result;
  }
  /** @} */ /* end_name Wrapper manipulation */

 private:
  q3d_audio_context* context_;

  /** Disallow copy and assign. */
  SpatialAudioApi(const SpatialAudioApi&);
  void operator=(const SpatialAudioApi&);
};

}  /**< namespace Q3DAudioPlatform */

/** @} */  /* end_addtogroup Audio_CPP_API */

#endif  // #if defined(__cplusplus) && !defined(SAP_NO_CPP_WRAPPER)

#endif  // Q3D_AUDIO_PLATFORM_INCLUDE_H_
#pragma once
