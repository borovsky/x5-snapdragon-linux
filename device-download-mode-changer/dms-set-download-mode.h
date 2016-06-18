/*
 * libqmi-glib -- GLib/GIO based library to control QMI devices
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2012-2015 Aleksander Morgado <aleksander@aleksander.es>
 */

#ifndef _DMS_CHANGE_DEVICE_DOWNLOAD_MODE_H_
#define _DMS_CHANGE_DEVICE_DOWNLOAD_MODE_H_

G_BEGIN_DECLS

/*****************************************************************************/
/* REQUEST/RESPONSE: Qmi Message DMS Change Device Download Mode */


/* --- Input -- */

/**
 * QmiMessageDmsChangeDeviceDownloadModeInput:
 *
 * The #QmiMessageDmsChangeDeviceDownloadModeInput structure contains private data and should only be accessed
 * using the provided API.
 */
typedef struct _QmiMessageDmsChangeDeviceDownloadModeInput QmiMessageDmsChangeDeviceDownloadModeInput;
GType qmi_message_dms_change_device_download_mode_input_get_type (void) G_GNUC_CONST;
#define QMI_TYPE_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE_INPUT (qmi_message_dms_change_device_download_mode_input_get_type ())

gboolean qmi_message_dms_change_device_download_mode_input_get_mode (
    QmiMessageDmsChangeDeviceDownloadModeInput *self,
    guint8 *value_mode,
    GError **error);

gboolean qmi_message_dms_change_device_download_mode_input_set_mode (
    QmiMessageDmsChangeDeviceDownloadModeInput *self,
    guint8 value_mode,
    GError **error);

QmiMessageDmsChangeDeviceDownloadModeInput *qmi_message_dms_change_device_download_mode_input_ref (QmiMessageDmsChangeDeviceDownloadModeInput *self);
void qmi_message_dms_change_device_download_mode_input_unref (QmiMessageDmsChangeDeviceDownloadModeInput *self);
QmiMessageDmsChangeDeviceDownloadModeInput *qmi_message_dms_change_device_download_mode_input_new (void);

/* --- Output -- */

/**
 * QmiMessageDmsChangeDeviceDownloadModeOutput:
 *
 * The #QmiMessageDmsChangeDeviceDownloadModeOutput structure contains private data and should only be accessed
 * using the provided API.
 */
typedef struct _QmiMessageDmsChangeDeviceDownloadModeOutput QmiMessageDmsChangeDeviceDownloadModeOutput;
GType qmi_message_dms_change_device_download_mode_output_get_type (void) G_GNUC_CONST;
#define QMI_TYPE_MESSAGE_DMS_CHANGE_DEVICE_DOWNLOAD_MODE_OUTPUT (qmi_message_dms_change_device_download_mode_output_get_type ())

gboolean qmi_message_dms_change_device_download_mode_output_get_result (
    QmiMessageDmsChangeDeviceDownloadModeOutput *self,
    GError **error);

QmiMessageDmsChangeDeviceDownloadModeOutput *qmi_message_dms_change_device_download_mode_output_ref (QmiMessageDmsChangeDeviceDownloadModeOutput *self);
void qmi_message_dms_change_device_download_mode_output_unref (QmiMessageDmsChangeDeviceDownloadModeOutput *self);

G_END_DECLS

#endif /* _DMS_CHANGE_DEVICE_DOWNLOAD_MODE_H_ */
