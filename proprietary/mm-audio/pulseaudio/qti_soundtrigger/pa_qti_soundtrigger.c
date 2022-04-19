/* pa_qti_soundtrigger.c
 *
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#define LOG_TAG "pa_qti_soundtrigger"
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

/* #define VERY_VERBOSE_LOGGING */
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <gio/gio.h>
#include <glib/gprintf.h>

#include "pa_qti_soundtrigger.h"

#define PA_QST_DBUS_OBJECT_PATH_PREFIX "/org/pulseaudio/ext/qsthw"
#define PA_QST_DBUS_MODULE_IFACE "org.PulseAudio.Ext.Qsthw"
#define PA_QST_DBUS_SESSION_IFACE "org.PulseAudio.Ext.Qsthw.Session"
#define PA_QST_DBUS_MODULE_OBJ_PATH_SIZE 256

#ifndef memscpy
#define memscpy(dst, dst_size, src, bytes_to_copy) \
        (void) memcpy(dst, src, MIN(dst_size, bytes_to_copy))
#endif

struct pa_qst_module_data {
    GDBusConnection *conn;
    char g_obj_path[PA_QST_DBUS_MODULE_OBJ_PATH_SIZE];
    GHashTable *ses_hash_table;
};

struct pa_qst_session_data {
    char *obj_path;
    GThread *thread_loop;
    GMainLoop *loop;
    guint sub_id;

    pa_qst_recognition_callback_t callback;
    void *cookie;
};

static pa_qst_ses_handle_t parse_ses_handle(char *obj_path) {
    char **handle_string;
    pa_qst_ses_handle_t handle;

    handle_string = g_strsplit(obj_path, "_", -1);
    handle = g_ascii_strtoll(handle_string[1], NULL, 0);
    g_printf("session handle %d\n",handle);
    g_strfreev(handle_string);

    return handle;
}

static void on_det_event_callback(GDBusConnection *conn,
                                  const gchar *sender_name,
                                  const gchar *object_path,
                                  const gchar *interface_name,
                                  const gchar *signal_name,
                                  GVariant *parameters,
                                  gpointer data) {
    struct pa_qst_session_data *ses_data = (struct pa_qst_session_data *)data;
    GError *error = NULL;
    GVariant *struct_v, *struct_v2, *array_v, *array_v2;
    GVariantIter arg_i, struct_i, struct_ii, array_i, array_ii;
    struct pa_qst_phrase_recognition_event *pa_qst_event;
    struct sound_trigger_phrase_recognition_event phrase_event = {0, };
    gint i = 0, j = 0;
    gsize n_elements = 0;
    gsize element_size = sizeof(guchar);
    gconstpointer value;
    guint64 timestamp;

    g_printf("signal handler: Ondetection event signal received\n");
    g_variant_iter_init(&arg_i, parameters);
    /* iterate over common sound_trigger_recognition_event */
    struct_v = g_variant_iter_next_value(&arg_i);
    g_variant_iter_init(&struct_i, struct_v);
    g_variant_iter_next(&struct_i, "i", &phrase_event.common.status);
    g_variant_iter_next(&struct_i, "i", &phrase_event.common.type);
    g_variant_iter_next(&struct_i, "i", &phrase_event.common.model);
    g_variant_iter_next(&struct_i, "b", &phrase_event.common.capture_available);
    g_variant_iter_next(&struct_i, "i", &phrase_event.common.capture_session);
    g_variant_iter_next(&struct_i, "i", &phrase_event.common.capture_delay_ms);
    g_variant_iter_next(&struct_i, "i", &phrase_event.common.capture_preamble_ms);
    g_variant_iter_next(&struct_i, "b", &phrase_event.common.trigger_in_data);
    struct_v2 = g_variant_iter_next_value(&struct_i);
    g_variant_iter_init(&struct_ii, struct_v2);
    g_variant_iter_next(&struct_ii, "u", &phrase_event.common.audio_config.sample_rate);
    g_variant_iter_next(&struct_ii, "u", &phrase_event.common.audio_config.channel_mask);
    g_variant_iter_next(&struct_ii, "u", &phrase_event.common.audio_config.format);
    g_variant_iter_next(&struct_ii, "u", &phrase_event.common.audio_config.frame_count);

    array_v = g_variant_iter_next_value(&arg_i);
    g_variant_iter_init(&array_i, array_v);
    i = 0;
    while ((struct_v = g_variant_iter_next_value(&array_i)) &&
           (phrase_event.num_phrases < SOUND_TRIGGER_MAX_PHRASES)) {
        g_variant_iter_init(&struct_i, struct_v);
        g_variant_iter_next(&struct_i, "u", &phrase_event.phrase_extras[i].id);
        g_variant_iter_next(&struct_i, "u", &phrase_event.phrase_extras[i].recognition_modes);
        g_variant_iter_next(&struct_i, "u", &phrase_event.phrase_extras[i].confidence_level);

        array_v2 = g_variant_iter_next_value(&struct_i);
        g_variant_iter_init(&array_ii, array_v2);
        j = 0;
        while ((struct_v2 = g_variant_iter_next_value(&array_ii)) &&
              (phrase_event.phrase_extras[i].num_levels < SOUND_TRIGGER_MAX_USERS)) {
            g_variant_iter_init(&struct_ii, struct_v2);
            g_variant_iter_next(&struct_ii, "u", &phrase_event.phrase_extras[i].levels[j].user_id);
            g_variant_iter_next(&struct_ii, "u", &phrase_event.phrase_extras[i].levels[j].level);
            phrase_event.phrase_extras[i].num_levels++;
            j++;
        }
        phrase_event.num_phrases++;
        i++;
        g_variant_unref(struct_v);
    }
    g_variant_iter_next(&arg_i, "t", &timestamp);

    phrase_event.common.data_offset = sizeof(struct pa_qst_phrase_recognition_event);
    array_v = g_variant_iter_next_value(&arg_i);
    value = g_variant_get_fixed_array(array_v, &n_elements, element_size);
    phrase_event.common.data_size = n_elements;

    pa_qst_event = g_malloc0(sizeof(struct pa_qst_phrase_recognition_event) +
                   phrase_event.common.data_size);
    memcpy(&pa_qst_event->phrase_event, &phrase_event, sizeof(phrase_event));
    pa_qst_event->timestamp = timestamp;

    memscpy((char*)pa_qst_event + pa_qst_event->phrase_event.common.data_offset,
            pa_qst_event->phrase_event.common.data_size,
            value, n_elements);

    ses_data->callback(&pa_qst_event->phrase_event.common, ses_data->cookie);
    g_printf("return Ondetection event signal\n");
    g_free(pa_qst_event);
}

static int subscribe_detection_event(struct pa_qst_module_data *m_data,
                                     struct pa_qst_session_data *ses_data,
                                     bool subscribe) {
    GVariant *result;
    GVariant *argument_sig_listener = NULL;
    GError *error = NULL;
    guint id;
    char signal_name[128];
    gint ret = 0;

    g_snprintf(signal_name, sizeof(signal_name),
               "%s.%s", PA_QST_DBUS_SESSION_IFACE, "DetectionEvent");
    if (subscribe) {
       /* Add listener for signal to PulseAudio core.
        * this is done during load of first session i.e. empty hash table
        * Empty obj path array is sent to listen for signals from all objects on
        * this connection
        */
        if (g_hash_table_size(m_data->ses_hash_table) == 0) {
            const gchar *obj_str[] = {};
            argument_sig_listener = g_variant_new("(@s@ao)",
                            g_variant_new_string(signal_name),
                            g_variant_new_objv(obj_str, 0));

            result = g_dbus_connection_call_sync(m_data->conn,
                                    NULL,
                                    "/org/pulseaudio/core1",
                                    "org.PulseAudio.Core1",
                                    "ListenForSignal",
                                    argument_sig_listener,
                                    NULL,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);
            if (result == NULL) {
                g_printerr ("Error invoking ListenForSignal(): %s\n", error->message);
                g_error_free(error);
                ret = -EINVAL;
                goto exit;
            }

            g_variant_unref(result);
        }

        /* subscribe for detection event signal */
        ses_data->sub_id = g_dbus_connection_signal_subscribe(m_data->conn,
                           NULL,
                           PA_QST_DBUS_SESSION_IFACE,
                           "DetectionEvent",
                           ses_data->obj_path,
                           NULL,
                           G_DBUS_SIGNAL_FLAGS_NONE,
                           on_det_event_callback,
                           ses_data,
                           NULL);
    } else {
       /* Remove signal listener to PulseAudio core.
        * this is done during unload of last session i.e. hash table size == 1.
        */
        if (g_hash_table_size(m_data->ses_hash_table) == 1) {
            argument_sig_listener = g_variant_new("(@s)",
                            g_variant_new_string(signal_name));
            result = g_dbus_connection_call_sync(m_data->conn,
                                    NULL,
                                    "/org/pulseaudio/core1",
                                    "org.PulseAudio.Core1",
                                    "StopListeningForSignal",
                                    argument_sig_listener,
                                    NULL,
                                    G_DBUS_CALL_FLAGS_NONE,
                                    -1,
                                    NULL,
                                    &error);
            if (result == NULL) {
                g_printerr ("Error invoking ListenForSignal(): %s\n", error->message);
                g_error_free(error);
                ret = -EINVAL;
                goto exit;
            }

            g_variant_unref(result);
        }

        if (ses_data->sub_id)
            g_dbus_connection_signal_unsubscribe(m_data->conn, ses_data->sub_id);
    }

exit:
    return ret;
}

static void *signal_threadloop(void *cookie) {
    struct pa_qst_session_data *ses_data = (struct pa_qst_session_data *)cookie;

    g_printf("Enter %s %p\n", __func__, ses_data);
    if (!ses_data) {
        g_printerr("Invalid thread params");
        goto exit;
    }

    ses_data->loop = g_main_loop_new(NULL, FALSE);
    g_printf("initiate main loop run for detections %d\n", ses_data->sub_id);
    g_main_loop_run(ses_data->loop);

    g_printf("out of main loop\n");
    g_main_loop_unref(ses_data->loop);

exit:
    return NULL;
}

static gint unload_sm(struct pa_qst_module_data *m_data,
                      struct pa_qst_session_data *ses_data) {
    GVariant *result;
    GError *error = NULL;
    gint ret = 0;

    if (subscribe_detection_event(m_data, ses_data, false /*unsubscribe */)) {
        g_printerr("Failed to unsubscribe for detection event");
    }

    /* Quit mainloop started to listen for detection signals */
    if (!ses_data->thread_loop) {
        g_main_loop_quit(ses_data->loop);
        g_thread_join(ses_data->thread_loop);
        ses_data->thread_loop = NULL;
    }

    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL,
                            ses_data->obj_path,
                            PA_QST_DBUS_SESSION_IFACE,
                            "UnloadSoundModel",
                            NULL,
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr ("Error invoking UnloadSoundmodel(): %s\n", error->message);
        g_error_free(error);
        ret = -EINVAL;
    } else {
        g_variant_unref(result);
    }

    return ret;
}

gint pa_qst_load_sound_model(const pa_qst_handle_t *mod_handle,
                             struct sound_trigger_sound_model *sm,
                             pa_qst_sound_model_callback_t callback,
                             void *cookie,
                             pa_qst_ses_handle_t *handle) {
    GVariant *result;
    GVariant *value_1, *value_2, *value_3, *value_4, *value_arr;
    GVariant *argument_1, *argument_2, *argument_load;
    GError *error = NULL;
    GVariantBuilder builder_1;
    gint i = 0, j = 0;
    struct sound_trigger_phrase_sound_model *sound_model;;
    struct pa_qst_session_data *ses_data;
    struct pa_qst_module_data *m_data;
    pa_qst_ses_handle_t ses_handle;
    gchar thread_name[16];

    if (!mod_handle || !sm || !handle) {
        g_printerr("Invalid input params\n");
        return -EINVAL;
    }

    m_data = (struct pa_qst_module_data *)mod_handle;
    sound_model = (struct sound_trigger_phrase_sound_model *)sm;

    ses_data = g_malloc0(sizeof(struct pa_qst_session_data));

    /* build loadsoundmodel message */
    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("(uqqqay)"));
    g_variant_builder_add(&builder_1, "u", (guint32)sound_model->common.uuid.timeLow);
    g_variant_builder_add(&builder_1, "q", (guint16)sound_model->common.uuid.timeMid);
    g_variant_builder_add(&builder_1, "q", (guint16)sound_model->common.uuid.timeHiAndVersion);
    g_variant_builder_add(&builder_1, "q", (guint16)sound_model->common.uuid.clockSeq);
    value_arr = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
        (gconstpointer)&sound_model->common.uuid.node[0], 6, sizeof(guchar));
    g_variant_builder_add_value(&builder_1, value_arr);
    value_1 = g_variant_builder_end(&builder_1);

    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("(uqqqay)"));
    g_variant_builder_add(&builder_1, "u", (guint32)sound_model->common.vendor_uuid.timeLow);
    g_variant_builder_add(&builder_1, "q", (guint16)sound_model->common.vendor_uuid.timeMid);
    g_variant_builder_add(&builder_1, "q", (guint16)sound_model->common.vendor_uuid.timeHiAndVersion);
    g_variant_builder_add(&builder_1, "q", (guint16)sound_model->common.vendor_uuid.clockSeq);
    value_arr = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
        (gconstpointer)&sound_model->common.vendor_uuid.node[0], 6, sizeof(guchar));
    g_variant_builder_add_value(&builder_1, value_arr);
    value_2 = g_variant_builder_end(&builder_1);

    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("a(uuauss)"));
    for (i = 0; i < sound_model->num_phrases; i++) {
        g_variant_builder_open(&builder_1, G_VARIANT_TYPE("(uuauss)"));
        g_variant_builder_add(&builder_1, "u", (guint32)sound_model->phrases[i].id);
        g_variant_builder_add(&builder_1, "u", (guint32)sound_model->phrases[i].recognition_mode);
        g_variant_builder_open(&builder_1, G_VARIANT_TYPE("au"));
        for (j = 0; j < sound_model->phrases[i].num_users; j++) {
            g_variant_builder_add(&builder_1, "u", (guint32)sound_model->phrases[i].users[j]);
        }
        g_variant_builder_close(&builder_1);
        g_variant_builder_add(&builder_1, "s", sound_model->phrases[i].locale);
        g_variant_builder_add(&builder_1, "s", sound_model->phrases[i].text);
        g_variant_builder_close(&builder_1);
    }
    value_3 = g_variant_builder_end(&builder_1);

    value_4 = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
        (gconstpointer)((gchar *)sound_model + sound_model->common.data_offset),
        sound_model->common.data_size, sizeof(guchar));

    argument_1 = g_variant_new("(@i@(uqqqay)@(uqqqay))",
                               g_variant_new_int32(sound_model->common.type),
                               value_1,
                               value_2);

    argument_2 = g_variant_new("(@(i(uqqqay)(uqqqay))@a(uuauss))",
                               argument_1,
                               value_3);

    argument_load = g_variant_new("(@((i(uqqqay)(uqqqay))a(uuauss))@ay)",
                               argument_2,
                               value_4);

    /*
     * Use global obj and intf path to call LoadSoundModel which
     * will return per session obj path.
     */
    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL, /* bus_name */
                            m_data->g_obj_path,
                            PA_QST_DBUS_MODULE_IFACE,
                            "LoadSoundModel",
                            argument_load,
                            G_VARIANT_TYPE("(o)"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr ("Error invoking LoadSoundModel(): %s\n", error->message);
        g_error_free(error);
        goto exit;
    }

    g_variant_get(result, "(o)", &ses_data->obj_path);
    g_printf("The server answered: obj path: '%s'\n", ses_data->obj_path);
    g_variant_unref(result);

    /* parse sm handle from object path */
    ses_handle = parse_ses_handle(ses_data->obj_path);

    /* Start threadloop to listen to signals from server */
    sprintf(thread_name, "pa_loop_%d", ses_handle);
    g_printf("create thread %s\n", thread_name);
    ses_data->thread_loop = g_thread_try_new(thread_name, signal_threadloop,
                              ses_data, &error);
    if (!ses_data->thread_loop) {
        g_printf("Could not create thread %s, error %s\n", thread_name, error->message);
        g_error_free(error);
        goto exit_1;
    }

    if (subscribe_detection_event(m_data, ses_data, true /* subscribe */)) {
        g_printerr("Failed to subscribe for detection event");
        goto exit_1;
    }

    /* add session to module hash table */
    g_hash_table_insert(m_data->ses_hash_table, GINT_TO_POINTER(ses_handle), ses_data);
    *handle = ses_handle;
    return 0;

exit_1:
    /* unload sound model internally */
    unload_sm(m_data, ses_data);

exit:
    if (ses_data)
        g_free(ses_data);
    *handle = -1;
    return -EINVAL;
}

gint pa_qst_unload_sound_model(const pa_qst_handle_t *mod_handle,
                               pa_qst_ses_handle_t handle) {
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    struct pa_qst_session_data *ses_data;
    gint ret = 0;

    if (!m_data) {
        g_printf("Invalid input params\n");
        ret = -EINVAL;
        goto exit;
    }

    if ((ses_data = (struct pa_qst_session_data *)g_hash_table_lookup(m_data->ses_hash_table,
                        GINT_TO_POINTER(handle))) == NULL) {
        g_printerr("No session exists for given handle %d\n", handle);
        ret = -EINVAL;
        goto exit;
    }

    ret = unload_sm(m_data, ses_data);
    g_hash_table_remove(m_data->ses_hash_table, GINT_TO_POINTER(handle));
    g_free(ses_data);

exit:
    return ret;
}

int pa_qst_get_properties(const pa_qst_handle_t *mod_handle,
                          struct sound_trigger_properties *properties) {
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    GError *error = NULL;
    GVariant *result, *struct_v, *struct_v2, *array_v;
    GVariantIter arg_i, struct_i, struct_ii;
    gint ret = 0, i = 0;
    gsize n_elements = 0;
    gsize element_size = sizeof(guchar);
    gconstpointer value;

    if (!m_data || !properties) {
        g_printf("Invalid input params\n");
        ret = -EINVAL;
        goto exit;
    }

    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL,
                            m_data->g_obj_path,
                            PA_QST_DBUS_MODULE_IFACE,
                            "GetProperties",
                            NULL,
                            G_VARIANT_TYPE("((ssu(uqqqay)uuuububbu))"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr ("Error invoking GetProperties(): %s\n", error->message);
        g_error_free(error);
        ret = -EINVAL;
        goto exit;
    }
    g_variant_iter_init(&arg_i, result);
    struct_v = g_variant_iter_next_value(&arg_i);
    g_variant_iter_init(&struct_i, struct_v);
    g_variant_iter_next(&struct_i, "s", &properties->implementor);
    g_variant_iter_next(&struct_i, "s", &properties->description);
    g_variant_iter_next(&struct_i, "u", &properties->version);

    struct_v2 = g_variant_iter_next_value(&struct_i);
    g_variant_iter_init(&struct_ii, struct_v2);
    g_variant_iter_next(&struct_ii, "u", &properties->uuid.timeLow);
    g_variant_iter_next(&struct_ii, "q", &properties->uuid.timeMid);
    g_variant_iter_next(&struct_ii, "q", &properties->uuid.timeHiAndVersion);
    g_variant_iter_next(&struct_ii, "q", &properties->uuid.clockSeq);
    array_v = g_variant_iter_next_value(&struct_ii);
    value = g_variant_get_fixed_array(array_v, &n_elements, element_size);
    memscpy(&properties->uuid.node[0], 6, value, n_elements);

    g_variant_iter_next(&struct_i, "u", &properties->max_sound_models);
    g_variant_iter_next(&struct_i, "u", &properties->max_key_phrases);
    g_variant_iter_next(&struct_i, "u", &properties->max_users);
    g_variant_iter_next(&struct_i, "u", &properties->recognition_modes);
    g_variant_iter_next(&struct_i, "b", &properties->capture_transition);
    g_variant_iter_next(&struct_i, "u", &properties->max_buffer_ms);
    g_variant_iter_next(&struct_i, "b", &properties->concurrent_capture);
    g_variant_iter_next(&struct_i, "b", &properties->trigger_in_event);
    g_variant_iter_next(&struct_i, "u", &properties->power_consumption_mw);

    g_variant_unref(result);

exit:
    return ret;
}

gint pa_qst_start_recognition(const pa_qst_handle_t *mod_handle,
                              pa_qst_ses_handle_t handle,
                              const struct sound_trigger_recognition_config *rc_config,
                              pa_qst_recognition_callback_t callback,
                              void *cookie) {
    GVariant *value_1, *value_2;
    GVariantBuilder builder_1;
    GVariant *argument, *argument_start;
    GVariant *result;
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    gint ret = 0, i = 0, j = 0;
    GError *error = NULL;
    struct pa_qst_session_data *ses_data;

    if (!m_data) {
        g_printerr("Invalid input params\n");
        ret = -EINVAL;
        goto exit;
    }

    if ((ses_data = (struct pa_qst_session_data *)g_hash_table_lookup(m_data->ses_hash_table,
                        GINT_TO_POINTER(handle))) == NULL) {
        g_printerr("No session exists for given handle %d\n", handle);
        ret = -EINVAL;
        goto exit;
    }

    g_variant_builder_init(&builder_1, G_VARIANT_TYPE("a(uuua(uu))"));
    for (i = 0; i < rc_config->num_phrases; i++) {
        g_variant_builder_open(&builder_1, G_VARIANT_TYPE("(uuua(uu))"));
        g_variant_builder_add(&builder_1, "u", (guint32)rc_config->phrases[i].id);
        g_variant_builder_add(&builder_1, "u", (guint32)rc_config->phrases[i].recognition_modes);
        g_variant_builder_add(&builder_1, "u", (guint32)rc_config->phrases[i].confidence_level);

        g_variant_builder_open(&builder_1, G_VARIANT_TYPE("a(uu)"));
        for (j = 0; j < rc_config->phrases[i].num_levels; j++) {
            g_variant_builder_open(&builder_1, G_VARIANT_TYPE("(uu)"));
            g_variant_builder_add(&builder_1, "u", (guint32)rc_config->phrases[i].levels[j].user_id);
            g_variant_builder_add(&builder_1, "u", (guint32)rc_config->phrases[i].levels[j].level);
            g_variant_builder_close(&builder_1);
        }
        g_variant_builder_close(&builder_1);
        g_variant_builder_close(&builder_1);
    }
    value_1 = g_variant_builder_end(&builder_1);

    value_2 = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
        (gconstpointer)((gchar *)rc_config + rc_config->data_offset),
        rc_config->data_size, sizeof(guchar));

    argument = g_variant_new("(@i@u@b@a(uuua(uu)))",
                   g_variant_new_int32(rc_config->capture_handle),
                   g_variant_new_uint32(rc_config->capture_device),
                   g_variant_new_boolean(rc_config->capture_requested),
                   value_1);

    argument_start = g_variant_new("(@(iuba(uuua(uu)))@ay)",
                   argument,
                   value_2);

    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL,
                            ses_data->obj_path,
                            PA_QST_DBUS_SESSION_IFACE,
                            "StartRecognition",
                            argument_start,
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr ("Error invoking StartRecognition(): %s\n", error->message);
        g_error_free(error);
        ret = -EINVAL;
        goto exit;
    }

    g_variant_unref(result);
    ses_data->callback = callback;
    ses_data->cookie = cookie;

exit:
    return ret;
}

gint pa_qst_stop_recognition(const pa_qst_handle_t *mod_handle,
                             pa_qst_ses_handle_t handle) {
    GVariant *result;
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    GError *error = NULL;
    struct pa_qst_session_data *ses_data;
    gint ret = 0;

    if (!m_data) {
        g_printf("Invalid input params\n");
        ret = -EINVAL;
        goto exit;
    }

    if ((ses_data = (struct pa_qst_session_data *)g_hash_table_lookup(m_data->ses_hash_table,
                        GINT_TO_POINTER(handle))) == NULL) {
        g_printerr("No session exists for given handle %d\n", handle);
        ret = -EINVAL;
        goto exit;
    }

    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL,
                            ses_data->obj_path,
                            PA_QST_DBUS_SESSION_IFACE,
                            "StopRecognition",
                            NULL,
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr ("Error invoking StopRecognition(): %s\n", error->message);
        g_error_free(error);
        ret = -EINVAL;
        goto exit;
    }

    g_variant_unref(result);

exit:
    return ret;
}

int pa_qst_set_parameters(const pa_qst_handle_t *mod_handle,
                          pa_qst_ses_handle_t handle,
                          const char *kv_pairs) {
    GVariant *result;
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    GError *error = NULL;
    struct pa_qst_session_data *ses_data;
    GVariant *argument = NULL;
    gint ret = 0;

    if (!m_data || !kv_pairs) {
        g_printf("Invalid input params\n");
        ret = -EINVAL;
        goto exit;
    }

    argument = g_variant_new("(@s)",
                    g_variant_new_string(kv_pairs));

    if (handle == 0) {
        /* handle global set param here */
        result = g_dbus_connection_call_sync(m_data->conn,
                                NULL,
                                m_data->g_obj_path,
                                PA_QST_DBUS_MODULE_IFACE,
                                "SetParameters",
                                argument,
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    } else {
        /* handle per session set param here */
        if ((ses_data =
                  (struct pa_qst_session_data *)g_hash_table_lookup(m_data->ses_hash_table,
                            GINT_TO_POINTER(handle))) == NULL) {
            g_printerr("No session exists for given handle %d\n", handle);
            ret = -EINVAL;
            goto exit;
        }
        result = g_dbus_connection_call_sync(m_data->conn,
                                NULL,
                                ses_data->obj_path,
                                PA_QST_DBUS_SESSION_IFACE,
                                "SetParameters",
                                argument,
                                NULL,
                                G_DBUS_CALL_FLAGS_NONE,
                                -1,
                                NULL,
                                &error);
    }

    if (result == NULL) {
        g_printerr ("Error invoking SetParameters(): %s\n", error->message);
        g_error_free(error);
        ret = -EINVAL;
        goto exit;
    }

    g_variant_unref(result);

exit:
    return ret;
}

int pa_qst_get_param_data(const pa_qst_handle_t *mod_handle,
                          pa_qst_ses_handle_t handle,
                          const char *param,
                          void *payload,
                          size_t payload_size,
                          size_t *param_data_size) {
    GVariant *result, *array_v;
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    GError *error = NULL;
    struct pa_qst_session_data *ses_data;
    size_t buf_size = 0;
    gsize n_elements;
    gsize element_size = sizeof(guchar);
    gconstpointer value;
    GVariant *argument = NULL;
    gint ret = 0;

    if (!m_data || !payload || !param_data_size) {
        g_printerr("Invalid input params\n");
        ret = -EINVAL;
        goto exit;
    }

    /* Initialize payload to 0 and return null payload in case of error */
    memset(payload, 0, payload_size);

    if ((ses_data = (struct pa_qst_session_data *)g_hash_table_lookup(m_data->ses_hash_table,
                        GINT_TO_POINTER(handle))) == NULL) {
        g_printerr("No session exists for given handle %d\n", handle);
        ret = -EINVAL;
        goto exit;
    }

    argument = g_variant_new("(@s)",
                    g_variant_new_string(param));
    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL,
                            ses_data->obj_path,
                            PA_QST_DBUS_SESSION_IFACE,
                            "GetParamData",
                            argument,
                            G_VARIANT_TYPE("(ay)"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr("Error invoking GetParamData(): %s\n", error->message);
        g_error_free(error);
        goto exit;
    }

    array_v = g_variant_get_child_value(result, 0);
    value = g_variant_get_fixed_array(array_v, &n_elements, element_size);
    if (n_elements <= payload_size) {
        memcpy(payload, value, n_elements);
        *param_data_size = n_elements;
    } else {
        g_printerr("Insufficient payload size to copy payload data\n");
        ret = -ENOMEM;
    }

    g_variant_unref(array_v);
    g_variant_unref(result);

exit:
    return ret;
}

size_t pa_qst_get_buffer_size(const pa_qst_handle_t *mod_handle,
                              pa_qst_ses_handle_t handle) {
    GVariant *result;
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    GError *error = NULL;
    struct pa_qst_session_data *ses_data;
    size_t buf_size = 0;

    if (!m_data) {
        g_printf("Invalid input params\n");
        goto exit;
    }

    if ((ses_data = (struct pa_qst_session_data *)g_hash_table_lookup(m_data->ses_hash_table,
                        GINT_TO_POINTER(handle))) == NULL) {
        g_printerr("No session exists for given handle %d\n", handle);
        goto exit;
    }

    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL,
                            ses_data->obj_path,
                            PA_QST_DBUS_SESSION_IFACE,
                            "GetBufferSize",
                            NULL,
                            G_VARIANT_TYPE("(i)"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr ("Error invoking GetBufferSize(): %s\n", error->message);
        g_error_free(error);
        goto exit;
    }

    g_variant_get(result, "(i)", &buf_size);
    g_printf("The server answered: buf size: '%zu'\n", buf_size);
    g_variant_unref(result);

exit:
    return buf_size;
}

int pa_qst_read_buffer(const pa_qst_handle_t *mod_handle,
                        pa_qst_ses_handle_t handle,
                        unsigned char *buf,
                        size_t bytes) {
    GVariant *result, *array_v;
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    GError *error = NULL;
    struct pa_qst_session_data *ses_data;
    gsize n_elements = 0;
    gsize element_size = sizeof(guchar);
    gconstpointer value;
    GVariant *argument = NULL;

    if (!m_data || !buf) {
        g_printf("Invalid input params\n");
        goto exit;
    }

    /* Initialize buffer to 0 and return silent buffer in case of error */
    memset(buf, 0, bytes);

    if ((ses_data = (struct pa_qst_session_data *)g_hash_table_lookup(m_data->ses_hash_table,
                        GINT_TO_POINTER(handle))) == NULL) {
        g_printerr("No session exists for given handle %d\n", handle);
        goto exit;
    }

    argument = g_variant_new("(@u)",
                    g_variant_new_uint32(bytes));
    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL,
                            ses_data->obj_path,
                            PA_QST_DBUS_SESSION_IFACE,
                            "ReadBuffer",
                            argument,
                            G_VARIANT_TYPE("(ay)"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr ("Error invoking ReadBuffer(): %s\n", error->message);
        g_error_free(error);
        goto exit;
    }

    array_v = g_variant_get_child_value(result, 0);
    value = g_variant_get_fixed_array(array_v, &n_elements, element_size);
    memscpy(buf, bytes, value, n_elements);
    g_variant_unref(array_v);
    g_variant_unref(result);

exit:
    return n_elements;
}

int pa_qst_stop_buffering(const pa_qst_handle_t *mod_handle,
                          pa_qst_ses_handle_t handle) {
    GVariant *result;
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    GError *error = NULL;
    struct pa_qst_session_data *ses_data;
    gint ret = 0;

    if (!m_data) {
        g_printf("Invalid input params\n");
        ret = -EINVAL;
        goto exit;
    }

    if ((ses_data = (struct pa_qst_session_data *)g_hash_table_lookup(m_data->ses_hash_table,
                        GINT_TO_POINTER(handle))) == NULL) {
        g_printerr("No session exists for given handle %d\n", handle);
        ret = -EINVAL;
        goto exit;
    }

    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL,
                            ses_data->obj_path,
                            PA_QST_DBUS_SESSION_IFACE,
                            "StopBuffering",
                            NULL,
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr ("Error invoking StopBuffering(): %s\n", error->message);
        g_error_free(error);
        ret = -EINVAL;
        goto exit;
    }

    g_variant_unref(result);

exit:
    return ret;
}

int pa_qst_get_version(const pa_qst_handle_t *mod_handle) {
    gint version;
    GVariant *result;
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    GError *error = NULL;

    if (!m_data) {
        g_printf("Invalid input params\n");
        version = -EINVAL;
        goto exit;
    }

    result = g_dbus_connection_call_sync(m_data->conn,
                            NULL,
                            m_data->g_obj_path,
                            PA_QST_DBUS_MODULE_IFACE,
                            "GetVersion",
                            NULL,
                            G_VARIANT_TYPE("(i)"),
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);
    if (result == NULL) {
        g_printerr ("Error invoking GetVersion(): %s\n", error->message);
        g_error_free(error);
        version = -EINVAL;
        goto exit;
    }

    g_variant_get(result, "(i)", &version);
    g_printf("The server answered: version: '%d'\n", version);
    g_variant_unref(result);

exit:
    return version;
}

pa_qst_handle_t *pa_qst_init(const char *module_name) {
    struct pa_qst_module_data *m_data;
    const gchar *s_address = NULL;
    GError *error = NULL;
    char module_string[128];

    if (!g_strcmp0(module_name, PA_QST_MODULE_ID_PRIMARY)) {
        g_strlcpy(module_string, "primary", sizeof(module_string));
    } else {
        g_printerr("Unsupported module %s", module_name);
        goto exit;
    }

    m_data = g_malloc0(sizeof(struct pa_qst_module_data));

    s_address = getenv("PULSE_DBUS_SERVER");
    if (!s_address) {
        g_printf("Pulse DBus server address not set, use default address\n");
        m_data->conn = g_dbus_connection_new_for_address_sync("unix:path=/var/run/pulse/dbus-socket",
                  G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, NULL, NULL, &error);
    } else {
        g_printf("server address %s\n", s_address);
        m_data->conn = g_dbus_connection_new_for_address_sync(s_address,
                  G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, NULL, NULL, &error);
    }

    if (m_data->conn == NULL) {
        g_printerr("Error connecting to D-Bus address %s: %s\n", s_address, error->message);
        g_error_free(error);
        goto exit;
    }

    g_snprintf(m_data->g_obj_path, PA_QST_DBUS_MODULE_OBJ_PATH_SIZE,
               "%s/%s", PA_QST_DBUS_OBJECT_PATH_PREFIX, module_string);

    /* hash table to retrieve session information */
    m_data->ses_hash_table = g_hash_table_new(g_direct_hash, g_direct_equal);

    return (pa_qst_handle_t *)m_data;

exit:
    if (m_data)
        g_free(m_data);
    return NULL;
}

int pa_qst_deinit(const pa_qst_handle_t *mod_handle) {
    struct pa_qst_module_data *m_data = (struct pa_qst_module_data *)mod_handle;
    GError *error = NULL;

    if (m_data) {
        if (m_data->ses_hash_table) {
            g_hash_table_destroy(m_data->ses_hash_table);
        }
        if (m_data->conn) {
            if (!g_dbus_connection_close_sync(m_data->conn, NULL, &error)) {
                g_printerr("Error in connection close(): %s\n", error->message);
                g_error_free(error);
            }
        }
        g_free(m_data);
        return 0;
    } else {
        g_printerr("Invalid module handle\n");
        return -EINVAL;
    }
}
