/*******************************************************************************
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************/

#ifndef EZTUNE_CAM_ADAPTER_H
#define EZTUNE_CAM_ADAPTER_H

#include <cstdint>
#include <pthread.h>

#include "eztune.h"
#include "eztune_interface.h"
#include "eztune_internal_types.h"
#include "eztune_diagnostics.h"
#include "eztune_vfe_diagnostics.h"

namespace eztune
{
//!  Implements Adapter to camera APIs
/*!
  This class deals with send/receive commands to camera and getting chromatix
  and tuning table data from camera
*/
class CamAdapter
{
public:
    CamAdapter(void *handle, eztune_server_t mode);

    ~CamAdapter();

    static void TuningSetVfe(vfemodule_t module, optype_t optype, int32_t value);

    static void TuningSetPproc(pp_module_t module, optype_t optype, int32_t value);

    static void TuningSet3A(aaa_set_optype_t optype, int32_t value);

    static void TuningSetFocus(void *eztune_t_ptr, aftuning_optype_t optype, int32_t value);

    static bool TuningPostBusMsg(eztune_bus_msg_t optype, int32_t msg);

    //! Sends command to camera backend to update Chromatix to updated value
    /*!
        In essence copies the cached version of values present in eztune layer to
        camera backend. This API is called when eztune updates any chromatix parameter
    */
    bool UpdateCamChromatixData();
    bool UpdateCamAFTuneData();

    //! Sets pointers to cached version of tuning tables (chromatix and AF tunetables)
    bool SetupTuningTables(bool reset_ptr);

    //! Gets pointer to latest metadata from camera backend
    /*!
        This API is called by eztune process layer when it needs to update any parameter. The implementation
        queries camera backend to get the latest meta data pointer
    */

    bool GetPreviewInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size);

    bool GetJPEGSnapshotInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size);

    bool GetRAWSnapshotInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size,
                          uint8_t& bit_depth, uint8_t& pack_type);
    // Helper functions to get the details about the Raw snapshot
    // Like Bayer Pattern, Bit width, Pack type
    eztune_raw_format_t GetRAWSnapshotBayerPattern();
    uint8_t GetRAWSnapshotBitWidth();
    uint8_t GetRAWSnapshotPackType();
    uint8_t GetPreviewFormat();


    bool Get3AliveInfo(uint8_t &fps, uint16_t &width, uint16_t &height, uint8_t &format);

    //! Notify camera client events. Events like new meta data available
    /*
       \param in type: Event type
       \param in data: Data associated with event
       \param in pipe_fd: Pipe to communicate with the Process Thread
       \param in live_started: Whether the 3A Live protocol is started or not
    */
    void EventNotify(uint32_t type, void *data, int pipe_fd, bool live_started);

    //! Adapter may update some data structure like meta data in async fashion. If clients calls this API
    //  Async updates are not done
    int Lock() { return pthread_mutex_lock(&m_lock); }

    //! Disable locks and Enable internal updates
    int UnLock() { return pthread_mutex_unlock(&m_lock); }

    //static wrapper call for C API
    static void SendCmdWrapper(void *ptr, uint32_t cmd, void *value);

    //! Sends command to camera backend
    /*!
        This API is called by eztune process layer when it needs get/set any parameters including getting
        chromatix tables and AF tune tables

        \param in cmd: Command that is understood by camera
        \param in value: Pointer to paramter structure for the command
        \get_param: If set to true, it gets a param else set param
    */
    bool SendCmd(uint32_t cmd, void *value, bool get_param);

    bool SendEvent(mct_event_module_type_t type, void *data);

    void TriggerPreviewBufferCopy(bool value);

    void TriggerSnapshotBufferCopy(bool value);

    void SetMetadataPending(bool value);

    bool TriggerSnapshot(eztune_bus_msg_t type);

    void GetPreviewDimension(uint32_t &width, uint32_t &height);

    metadata_buffer_t *GetMetadata();

    bool ConfigureBufferDivert(bool enable);

    void MetadataLock();

    void MetadataUnlock();

private:
    int TryLock() { return pthread_mutex_trylock(&m_lock); }

    bool DataWait();
    void DataWaitNotify();

    void CopyPreviewImage(void *buffer);
    eztune_bus_msg_t CopySnapshotImage(void *ptr);
    void SendProcessEvent(EztuneNotify event_id, void *data, uint32_t size, int pipe_fd);

    bool FetchChromatixData();
    bool FetchAFTuneData();

    static CamAdapter *m_singleton;

    void *m_client_handle;
    eztune_chromatix_t *m_chromatix;
    metadata_buffer_t m_metadata;
    actuator_driver_params_t m_af_driver;
    ez_af_tuning_params_t m_af_tuning;
    pthread_mutex_t m_lock, m_cond_mutex;
    pthread_cond_t m_data_cond;
    bool m_chromatix_init_done;
    bool m_af_init_done;
    bool m_pending_metadata_request;
    bool m_pending_preview_request;
    bool m_pending_snapshot_request;

    int32_t m_preview_image_width;
    int32_t m_preview_image_height;
    int32_t m_snapshot_image_width;
    int32_t m_snapshot_image_height;
    cam_format_t m_preview_format;
    cam_format_t m_snapshot_format;
    uint8_t *m_preview_buffer;
    uint8_t *m_snapshot_buffer;
    uint32_t m_old_preview_size;
    uint32_t m_curr_snapshot_size;
    eztune_server_t m_mode;
};

};

#endif //EZTUNE_CAM_ADAPTER_H
