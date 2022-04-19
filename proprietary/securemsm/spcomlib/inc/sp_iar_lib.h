/*=============================================================================
Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#ifndef _SP_IAR_LIB_H_
#define _SP_IAR_LIB_H_

#include <stdbool.h>	/* bool */
#include <errno.h>
#include <stdint.h>     /* uint32_t */
#include <log/log.h>    /* SLOGE() */

/**
 * @brief - Secure Processor IAR API
 *
 * These APIs are for supporting Secure Processor IAR feature.
 */

/*===========================================================================*/
/*                           defines, enums , types                          */
/*===========================================================================*/

/**
 * IAR_STATE.
 *
 * states are based on 3 fuses that are blown
 */
#define SP_IAR_STATE_DISABLED           0
#define SP_IAR_STATE_INITIALIZING       1
#define SP_IAR_STATE_ACTIVATED          3
#define SP_IAR_STATE_ARI_END_OF_LIFE    7

#ifdef FEATURE_IAR

/**
 * IAR CMAC size in dwords
 */
#define CMAC_SIZE_DWORDS 4

/*===========================================================================*/
/* API for OEM App / spcom_utest test app */
/*===========================================================================*/

/**
 * iar_activate() - do IAR Activation process
 *
 * This should be call at manufacturing time.
 * Activation doesn't requires user authentication.
 * Activation should be triggered by OEM app.
 *
 * return: 0 on success, negative value on failure.
 */
int iar_activate(void);

/**
 * iar_get_status() - get IAR status
 *
 * This should be called by test app for debug and testing.
 * The IAR status 32 bit includes the following:
 * bit 0 - CE firmware detected
 * bit 1 - CE app detected
 * bit 8..15 - CE Counter
 * bit 16..23 - IAR-STATE
 * bit 24..31 - unused
 *
 * return: 0 on success, negative value on failure.
 */
int iar_get_status(uint32_t *iar_status);

/*===========================================================================*/
/* API for spdaemon */
/*===========================================================================*/

/**
 * iar_is_iar_state_activated()
 *
 * Check is IAR_STATE is ACTIVATED.
 *
 * return: true on SP_IAR_STATE_ACTIVATED, false on other iar states.
 */
bool iar_is_iar_state_activated(void);

/**
 * iar_load_spu_firmware_cmac() - set firmware cmac in the kernel.
 *
 * Read the FW cmac from the IAR-DB.
 * Pass the firmware CMAC to spss_utils kernel driver.
 * This api should be called by spdaemon, prior to triggering PIL to load SPU Firmware.
 *
 * return: 0 on success, negative value on failure.
 */
int iar_load_spu_firmware_cmac(void);

/**
 * iar_check_spu_pbl_ce() - handle PBL CE
 *
 * Check if SPU PBL CE happened.
 * Save the SPU PBL cmac upon CE.
 * This api should be called by spdaemon after PIL load SPU Firmware.
 *
 * return: 0 on success, negative value on failure.
 */
int iar_check_spu_pbl_ce(void);

/**
 * iar_is_recovery_needed()
 *
 * Checks if there is a mismatch between the SPU
 * IAR state and HLOS IAR state (SPU IAR state is
 * Activated, and HLOS IAR state is not).
 * In this case, a recovery is needed.
 *
 * return: true if recovery is needed, false otherwise.
 *
 */
bool iar_is_recovery_needed(void);

/**
 * iar_perform_recovery()
 *
 * Creates an default IAR DB so that in the next
 * device reboot the auto-correct will recover
 * the correct spu binaries cmac values.
 *
 * return: 0 on success, negative value on failure.
 *
 */
int iar_perform_recovery(void);

/**
 * iar_check_if_fw_update() - check if fw has updated.
 *
 * Compare the fw hash in the IAR-DB with the ones in the original path.
 *
 * return: when fw changed detected - return possitive number.
 *         when no images changed detected - return 0
 *         on failure - negative value.
 */
int iar_check_if_fw_update();

/*===========================================================================*/
/* API for AuthSecret */
/*===========================================================================*/

/**
 * iar_check_spu_images_update() - check for images update.
 *
 * Compare the images in the IAR-DB with the ones in the original path.
 * Check for update of spss firmware and apps images.
 * This api should be called by AuthSecret IAR module.
 *
 * return: positive value when images changed detected,
 *         0 when no images changed detected,
 *         negative value on failure.
 */
int iar_check_spu_images_update(void);

/**
 * iar_store_new_images_cmac() - store new CMACs
 *
 * Store CMACs calculated by SPU if result is success.
 * This api should be called by AuthSecret IAR module.
 *
 * @result:   cmacs calculation error code return from SPU.
 *
 * return: positive value when images changed detected,
 *         0 when no images changed detected,
 *         negative value on failure.
 */
int iar_store_new_images_cmac(int result);

/**
 * iar_activate_ex() - do IAR Activation process
 *
 * This should be call at manufacturing time.
 * Activation doesn't requires user authentication.
 * Activation should be triggered by OEM app.
 *
 * @apps_paths: paths of 3rd party applications to be hadled by IAR on activation.
 * @apps_names: names of 3rd party applications.
 *
 * @num_apps:   number of apps (entries in apps_paths).
 *
 * return: 0 on success, negative value on failure.
 *         negative value on failure.
 */
int iar_activate_ex(char *apps_paths[], char* apps_names[], const uint32_t num_apps);

/**
 * iar_handle_pre_app_load() - perform IAR related logic prior to calling load_app().
 *
 * This includes checking if the app has hash and cmac files.
 * If not - it creates hash file and hash segment file (for cmac calculation
 * during SW update).
 *
 * @app_name:     application name.
 * @ion_buff:     buffer holding the application.
 * @image_size:   size of image held in ion_buff.
 * @cmac [out]:   buffer to put the stored cmac in, for passing to SPU.
 *
 * return: positive value when operation succeeded,
 *         negative value on failure.
 */
int iar_handle_pre_app_load(const char *app_name, void *ion_buff, size_t image_size, void *cmac);

/**
 * iar_handle_post_app_load() - perform IAR related logic after load_app() completed.
 *
 * Check if "CMAC mismatch error" was returned from load_app(). If yes - return the error
 * and store the newly calculated cmac.
 *
 * @app_name:     application name.
 * @req_cmac:     cmac sent to load app via request struct.
 * @resp_cmac:    cmac returned from load app via response.
 * @resp_err:     response's error code.
 *
 * return: positive value when operation succeeded,
 *         negative value on failure.
 */
int iar_handle_post_app_load(const char *app_name, uint32_t *req_cmac, uint32_t *resp_cmac, uint32_t resp_err);

/**
 * iar_maintain_database_sanity() - maintain IAR DB sanity
 *
 * In case SPU fw hash file was deleted or has wrong size,
 * calculate hash again and store it.
 * This function should be called on boot.
 *
 * return: positive value when operation succeeded,
 *         negative value on failure.
 */
int iar_maintain_database_sanity(void);
#else
#define UNUSED(x) ((void)x)

    static inline bool iar_is_recovery_needed(void) {return false;}
    static inline bool iar_is_iar_state_activated(void) { return false;}
    static inline int iar_perform_recovery(void) {return -EINVAL;}
    static inline int iar_activate(void) { return -EINVAL;}
    static inline int iar_load_spu_firmware_cmac(void) { return -EINVAL;}
    static inline int iar_check_spu_pbl_ce(void) { return -EINVAL;}
    static inline int iar_check_spu_images_update(void) { return -EINVAL;}
    static inline int iar_check_if_fw_update() { return -EINVAL;}

    static inline int iar_store_new_images_cmac(int result)
    {
        UNUSED(result);
        return -EINVAL;
    }

    static inline int iar_activate_ex(char *apps_paths[], char* apps_names[], const uint32_t num_apps)
    {
        UNUSED(apps_paths);
        UNUSED(apps_names);
        UNUSED(num_apps);

        return -EINVAL;
    }

    static inline int iar_handle_pre_app_load(const char *app_name, void *ion_buff, size_t image_size, void *cmac)
    {
        UNUSED(app_name);
        UNUSED(ion_buff);
        UNUSED(image_size);
        UNUSED(cmac);

        return -EINVAL;
    }

    static inline int iar_handle_post_app_load(const char *app_name, uint32_t *req_cmac, uint32_t *resp_cmac, uint32_t resp_err)
    {
        UNUSED(app_name);
        UNUSED(req_cmac);
        UNUSED(resp_err);

        return -EINVAL;
    }

    static inline int iar_maintain_database_sanity(void) { return -EINVAL;}

#endif // FEATURE_IAR

#endif /* _SP_IAR_LIB_H_ */
