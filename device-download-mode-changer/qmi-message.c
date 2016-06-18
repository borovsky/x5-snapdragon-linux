#include <endian.h>
#include <glib.h>
#include <libqmi-glib/libqmi-glib.h>
#include <libqmi-glib/qmi-enums-private.h>
#include <string.h>

#include "qmi-message.h"

/**
 * SECTION:qmi-message
 * @title: QmiMessage
 * @short_description: Generic QMI message handling routines
 *
 * #QmiMessage is a generic type representing a QMI message of any kind
 * (request, response, indication) or service (including #QMI_SERVICE_CTL).
 *
 * This set of generic routines help in handling these message types, and
 * allow creating any kind of message with any kind of TLV.
 **/

#define PACKED __attribute__((packed))

struct qmux {
  guint16 length;
  guint8 flags;
  guint8 service;
  guint8 client;
} PACKED;

struct control_header {
  guint8 flags;
  guint8 transaction;
  guint16 message;
  guint16 tlv_length;
} PACKED;

struct service_header {
  guint8 flags;
  guint16 transaction;
  guint16 message;
  guint16 tlv_length;
} PACKED;

struct tlv {
  guint8 type;
  guint16 length;
  guint8 value[];
} PACKED;

struct control_message {
  struct control_header header;
  struct tlv tlv[];
} PACKED;

struct service_message {
  struct service_header header;
  struct tlv tlv[];
} PACKED;

struct full_message {
  guint8 marker;
  struct qmux qmux;
  union {
    struct control_message control;
    struct service_message service;
  } qmi;
} PACKED;

static inline gboolean message_is_control(QmiMessage *self) {
  return ((struct full_message *)(self->data))->qmux.service == QMI_SERVICE_CTL;
}

static inline guint16 get_qmux_length(QmiMessage *self) {
  return GUINT16_FROM_LE(((struct full_message *)(self->data))->qmux.length);
}

static inline void set_qmux_length(QmiMessage *self, guint16 length) {
  ((struct full_message *)(self->data))->qmux.length = GUINT16_TO_LE(length);
}

static inline guint8 get_qmux_flags(QmiMessage *self) {
  return ((struct full_message *)(self->data))->qmux.flags;
}

static inline guint8 get_qmi_flags(QmiMessage *self) {
  if (message_is_control(self))
    return ((struct full_message *)(self->data))->qmi.control.header.flags;

  return ((struct full_message *)(self->data))->qmi.service.header.flags;
}

/**
 * qmi_message_is_request:
 * @self: a #QmiMessage.
 *
 * Checks whether the given #QmiMessage is a request.
 *
 * Returns: %TRUE if @self is a request message, %FALSE otherwise.
 */
gboolean qmi_message_is_request(QmiMessage *self) {
  return (!qmi_message_is_response(self) && !qmi_message_is_indication(self));
}

/**
 * qmi_message_is_response:
 * @self: a #QmiMessage.
 *
 * Checks whether the given #QmiMessage is a response.
 *
 * Returns: %TRUE if @self is a response message, %FALSE otherwise.
 */
gboolean qmi_message_is_response(QmiMessage *self) {
  if (message_is_control(self)) {
    if (((struct full_message *)(self->data))->qmi.control.header.flags &
        QMI_CTL_FLAG_RESPONSE)
      return TRUE;
  } else {
    if (((struct full_message *)(self->data))->qmi.service.header.flags &
        QMI_SERVICE_FLAG_RESPONSE)
      return TRUE;
  }

  return FALSE;
}

/**
 * qmi_message_is_indication:
 * @self: a #QmiMessage.
 *
 * Checks whether the given #QmiMessage is an indication.
 *
 * Returns: %TRUE if @self is an indication message, %FALSE otherwise.
 */
gboolean qmi_message_is_indication(QmiMessage *self) {
  if (message_is_control(self)) {
    if (((struct full_message *)(self->data))->qmi.control.header.flags &
        QMI_CTL_FLAG_INDICATION)
      return TRUE;
  } else {
    if (((struct full_message *)(self->data))->qmi.service.header.flags &
        QMI_SERVICE_FLAG_INDICATION)
      return TRUE;
  }

  return FALSE;
}

/**
 * qmi_message_get_service:
 * @self: a #QmiMessage.
 *
 * Gets the service corresponding to the given #QmiMessage.
 *
 * Returns: a #QmiService.
 */
QmiService qmi_message_get_service(QmiMessage *self) {
  g_return_val_if_fail(self != NULL, QMI_SERVICE_UNKNOWN);

  return (QmiService)((struct full_message *)(self->data))->qmux.service;
}

/**
 * qmi_message_get_client_id:
 * @self: a #QmiMessage.
 *
 * Gets the client ID of the message.
 *
 * Returns: the client ID.
 */
guint8 qmi_message_get_client_id(QmiMessage *self) {
  g_return_val_if_fail(self != NULL, 0);

  return ((struct full_message *)(self->data))->qmux.client;
}

/**
 * qmi_message_get_transaction_id:
 * @self: a #QmiMessage.
 *
 * Gets the transaction ID of the message.
 *
 * Returns: the transaction ID.
 */
guint16 qmi_message_get_transaction_id(QmiMessage *self) {
  g_return_val_if_fail(self != NULL, 0);

  if (message_is_control(self))
    /* note: only 1 byte for transaction in CTL message */
    return (guint16)((struct full_message *)(self->data))
        ->qmi.control.header.transaction;

  return GUINT16_FROM_LE(
      ((struct full_message *)(self->data))->qmi.service.header.transaction);
}

/**
 * qmi_message_set_transaction_id:
 * @self: a #QmiMessage.
 * @transaction_id: transaction id.
 *
 * Overwrites the transaction ID of the message.
 */
void qmi_message_set_transaction_id(QmiMessage *self, guint16 transaction_id) {
  g_return_if_fail(self != NULL);

  if (message_is_control(self))
    ((struct full_message *)self->data)->qmi.control.header.transaction =
        (guint8)transaction_id;
  else
    ((struct full_message *)self->data)->qmi.service.header.transaction =
        GUINT16_TO_LE(transaction_id);
}

/**
 * qmi_message_get_message_id:
 * @self: a #QmiMessage.
 *
 * Gets the ID of the message.
 *
 * Returns: the ID.
 */
guint16 qmi_message_get_message_id(QmiMessage *self) {
  g_return_val_if_fail(self != NULL, 0);

  if (message_is_control(self))
    return GUINT16_FROM_LE(
        ((struct full_message *)(self->data))->qmi.control.header.message);

  return GUINT16_FROM_LE(
      ((struct full_message *)(self->data))->qmi.service.header.message);
}

/**
 * qmi_message_get_length:
 * @self: a #QmiMessage.
 *
 * Gets the length of the raw data corresponding to the given #QmiMessage.
 *
 * Returns: the length of the raw data.
 */
gsize qmi_message_get_length(QmiMessage *self) {
  g_return_val_if_fail(self != NULL, 0);

  return self->len;
}

static inline guint16 get_all_tlvs_length(QmiMessage *self) {
  if (message_is_control(self))
    return GUINT16_FROM_LE(
        ((struct full_message *)(self->data))->qmi.control.header.tlv_length);

  return GUINT16_FROM_LE(
      ((struct full_message *)(self->data))->qmi.service.header.tlv_length);
}

static inline void set_all_tlvs_length(QmiMessage *self, guint16 length) {
  if (message_is_control(self))
    ((struct full_message *)(self->data))->qmi.control.header.tlv_length =
        GUINT16_TO_LE(length);
  else
    ((struct full_message *)(self->data))->qmi.service.header.tlv_length =
        GUINT16_TO_LE(length);
}

static inline struct tlv *qmi_tlv(QmiMessage *self) {
  if (message_is_control(self))
    return ((struct full_message *)(self->data))->qmi.control.tlv;

  return ((struct full_message *)(self->data))->qmi.service.tlv;
}

static inline guint8 *qmi_end(QmiMessage *self) {
  return (guint8 *)self->data + self->len;
}

static inline struct tlv *tlv_next(struct tlv *tlv) {
  return (struct tlv *)((guint8 *)tlv + sizeof(struct tlv) +
                        GUINT16_FROM_LE(tlv->length));
}

static inline struct tlv *qmi_tlv_first(QmiMessage *self) {
  if (get_all_tlvs_length(self)) return qmi_tlv(self);

  return NULL;
}

static inline struct tlv *qmi_tlv_next(QmiMessage *self, struct tlv *tlv) {
  struct tlv *end;
  struct tlv *next;

  end = (struct tlv *)qmi_end(self);
  next = tlv_next(tlv);

  return (next < end ? next : NULL);
}

gsize qmi_message_tlv_write_init(QmiMessage *self, guint8 type, GError **error);

/*
 * Checks the validity of a QMI message.
 *
 * In particular, checks:
 * 1. The message has space for all required headers.
 * 2. The length of the buffer, the qmux length field, and the QMI tlv_length
 *    field are all consistent.
 * 3. The TLVs in the message fit exactly in the payload size.
 *
 * Returns: %TRUE if the message is valid, %FALSE otherwise.
 */
static gboolean message_check(QmiMessage *self, GError **error) {
  gsize header_length;
  guint8 *end;
  struct tlv *tlv;

  if (((struct full_message *)(self->data))->marker !=
      QMI_MESSAGE_QMUX_MARKER) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_INVALID_MESSAGE,
                "Marker is incorrect");
    return FALSE;
  }

  if (get_qmux_length(self) < sizeof(struct qmux)) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_INVALID_MESSAGE,
                "QMUX length too short for QMUX header (%u < %" G_GSIZE_FORMAT
                ")",
                get_qmux_length(self), sizeof(struct qmux));
    return FALSE;
  }

  /*
   * qmux length is one byte shorter than buffer length because qmux
   * length does not include the qmux frame marker.
   */
  if (get_qmux_length(self) != self->len - 1) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_INVALID_MESSAGE,
                "QMUX length and buffer length don't match (%u != %u)",
                get_qmux_length(self), self->len - 1);
    return FALSE;
  }

  header_length = sizeof(struct qmux) + (message_is_control(self)
                                             ? sizeof(struct control_header)
                                             : sizeof(struct service_header));

  if (get_qmux_length(self) < header_length) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_INVALID_MESSAGE,
                "QMUX length too short for QMI header (%u < %" G_GSIZE_FORMAT
                ")",
                get_qmux_length(self), header_length);
    return FALSE;
  }

  if (get_qmux_length(self) - header_length != get_all_tlvs_length(self)) {
    g_set_error(
        error, QMI_CORE_ERROR, QMI_CORE_ERROR_INVALID_MESSAGE,
        "QMUX length and QMI TLV lengths don't match (%u - %" G_GSIZE_FORMAT
        " != %u)",
        get_qmux_length(self), header_length, get_all_tlvs_length(self));
    return FALSE;
  }

  end = qmi_end(self);
  for (tlv = qmi_tlv(self); tlv < (struct tlv *)end; tlv = tlv_next(tlv)) {
    if (tlv->value > end) {
      g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_INVALID_MESSAGE,
                  "TLV header runs over buffer (%p > %p)", tlv->value, end);
      return FALSE;
    }
    if (tlv->value + GUINT16_FROM_LE(tlv->length) > end) {
      g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_INVALID_MESSAGE,
                  "TLV value runs over buffer (%p + %u  > %p)", tlv->value,
                  GUINT16_FROM_LE(tlv->length), end);
      return FALSE;
    }
  }

  /*
   * If this assert triggers, one of the if statements in the loop is wrong.
   * (It shouldn't be reached on malformed QMI messages.)
   */
  g_assert(tlv == (struct tlv *)end);

  return TRUE;
}

/**
 * qmi_message_new:
 * @service: a #QmiService
 * @client_id: client ID of the originating control point.
 * @transaction_id: transaction ID.
 * @message_id: message ID.
 *
 * Create a new #QmiMessage with the specified parameters.
 *
 * Note that @transaction_id must be less than #G_MAXUINT8 if @service is
 * #QMI_SERVICE_CTL.
 *
 * Returns: (transfer full): a newly created #QmiMessage. The returned value
 * should be freed with qmi_message_unref().
 */
QmiMessage *qmi_message_new(QmiService service, guint8 client_id,
                            guint16 transaction_id, guint16 message_id) {
  GByteArray *self;
  struct full_message *buffer;
  gsize buffer_len;

  /* Transaction ID in the control service is 8bit only */
  g_return_val_if_fail(
      (service != QMI_SERVICE_CTL || transaction_id <= G_MAXUINT8), NULL);

  /* Create array with enough size for the QMUX marker, the QMUX header and
   * the QMI header */
  buffer_len = (1 + sizeof(struct qmux) +
                (service == QMI_SERVICE_CTL ? sizeof(struct control_header)
                                            : sizeof(struct service_header)));

  /* NOTE:
   * Don't use g_byte_array_new_take() along with g_byte_array_set_size()!
   * Not yet, at least, see:
   * https://bugzilla.gnome.org/show_bug.cgi?id=738170
   */

  /* Create the GByteArray with buffer_len bytes preallocated */
  self = g_byte_array_sized_new(buffer_len);
  /* Actually flag as all the buffer_len bytes being used. */
  g_byte_array_set_size(self, buffer_len);

  buffer = (struct full_message *)(self->data);
  buffer->marker = QMI_MESSAGE_QMUX_MARKER;
  buffer->qmux.flags = 0;
  buffer->qmux.service = service;
  buffer->qmux.client = client_id;

  if (service == QMI_SERVICE_CTL) {
    buffer->qmi.control.header.flags = 0;
    buffer->qmi.control.header.transaction = (guint8)transaction_id;
    buffer->qmi.control.header.message = GUINT16_TO_LE(message_id);
  } else {
    buffer->qmi.service.header.flags = 0;
    buffer->qmi.service.header.transaction = GUINT16_TO_LE(transaction_id);
    buffer->qmi.service.header.message = GUINT16_TO_LE(message_id);
  }

  /* Update length fields. */
  set_qmux_length(self,
                  buffer_len - 1); /* QMUX marker not included in length */
  set_all_tlvs_length(self, 0);

  /* We shouldn't create invalid empty messages */
  g_assert(message_check(self, NULL));

  return (QmiMessage *)self;
}

/**
 * qmi_message_ref:
 * @self: a #QmiMessage.
 *
 * Atomically increments the reference count of @self by one.
 *
 * Returns: (transfer full) the new reference to @self.
 */
QmiMessage *qmi_message_ref(QmiMessage *self) {
  g_return_val_if_fail(self != NULL, NULL);

  return (QmiMessage *)g_byte_array_ref(self);
}

/**
 * qmi_message_unref:
 * @self: a #QmiMessage.
 *
 * Atomically decrements the reference count of @self by one.
 * If the reference count drops to 0, @self is completely disposed.
 */
void qmi_message_unref(QmiMessage *self) {
  g_return_if_fail(self != NULL);

  g_byte_array_unref(self);
}

/**
 * qmi_message_get_raw:
 * @self: a #QmiMessage.
 * @length: (out): return location for the size of the output buffer.
 * @error: return location for error or %NULL.
 *
 * Gets the raw data buffer of the #QmiMessage.
 *
 * Returns: (transfer none): The raw data buffer, or #NULL if @error is set.
 */
const guint8 *qmi_message_get_raw(QmiMessage *self, gsize *length,
                                  GError **error) {
  g_return_val_if_fail(self != NULL, NULL);
  g_return_val_if_fail(length != NULL, NULL);

  *length = self->len;
  return self->data;
}

/*****************************************************************************/
/* TLV builder & writer */

static gboolean tlv_error_if_write_overflow(QmiMessage *self, gsize len,
                                            GError **error) {
  /* Check for overflow of message size. */
  if (self->len + len > G_MAXUINT16) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_TLV_TOO_LONG,
                "Writing TLV would overflow");
    return FALSE;
  }

  return TRUE;
}

static struct tlv *tlv_get_header(QmiMessage *self, gssize init_offset) {
  g_assert(init_offset <= self->len);
  return (struct tlv *)(&self->data[init_offset]);
}

/**
 * qmi_message_tlv_write_init:
 * @self: a #QmiMessage.
 * @type: specific ID of the TLV to add.
 * @error: return location for error or %NULL.
 *
 * Starts building a new TLV in the #QmiMessage.
 *
 * In order to finish adding the TLV, qmi_message_tlv_write_complete() needs to
 * be
 * called.
 *
 * If any error happens adding fields on the TLV, the previous state can be
 * recovered using qmi_message_tlv_write_reset().
 *
 * Returns: the offset where the TLV was started to be added, or 0 if an error
 * happens.
 *
 * Since: 1.12
 */
gsize qmi_message_tlv_write_init(QmiMessage *self, guint8 type,
                                 GError **error) {
  gsize init_offset;
  struct tlv *tlv;

  g_return_val_if_fail(self != NULL, 0);
  g_return_val_if_fail(self->len > 0, 0);

  /* Check for overflow of message size. Note that a valid TLV will at least
   * have 1 byte of value. */
  if (!tlv_error_if_write_overflow(self, sizeof(struct tlv) + 1, error))
    return 0;

  /* Store where exactly we started adding the TLV */
  init_offset = self->len;

  /* Resize buffer to fit the TLV header */
  g_byte_array_set_size(self, self->len + sizeof(struct tlv));

  /* Write the TLV header */
  tlv = tlv_get_header(self, init_offset);
  tlv->type = type;
  tlv->length = 0; /* Correct value will be set in complete() */

  return init_offset;
}

/**
 * qmi_message_tlv_write_guint8:
 * @self: a #QmiMessage.
 * @in: a #guint8.
 * @error: return location for error or %NULL.
 *
 * Appends an unsigned byte to the TLV being built.
 *
 * Returns: %TRUE if the variable is successfully added, otherwise %FALSE is
 * returned and @error is set.
 *
 * Since: 1.12
 */
gboolean qmi_message_tlv_write_guint8(QmiMessage *self, guint8 in,
                                      GError **error) {
  g_return_val_if_fail(self != NULL, FALSE);

  /* Check for overflow of message size */
  if (!tlv_error_if_write_overflow(self, sizeof(in), error)) return FALSE;

  g_byte_array_append(self, &in, sizeof(in));
  return TRUE;
}

/**
 * qmi_message_tlv_write_guint16:
 * @self: a #QmiMessage.
 * @endian: target endianness, swapped from host byte order if necessary.
 * @in: a #guint16 in host byte order.
 * @error: return location for error or %NULL.
 *
 * Appends an unsigned 16-bit integer to the TLV being built. The number to be
 * written is expected to be given in host endianness, and this method takes
 * care of converting the value written to the byte order specified by @endian.
 *
 * Returns: %TRUE if the variable is successfully added, otherwise %FALSE is
 * returned and @error is set.
 *
 * Since: 1.12
 */
gboolean qmi_message_tlv_write_guint16(QmiMessage *self, QmiEndian endian,
                                       guint16 in, GError **error) {
  guint16 tmp;

  g_return_val_if_fail(self != NULL, FALSE);

  /* Check for overflow of message size */
  if (!tlv_error_if_write_overflow(self, sizeof(in), error)) return FALSE;

  tmp = (endian == QMI_ENDIAN_BIG ? GUINT16_TO_BE(in) : GUINT16_TO_LE(in));
  g_byte_array_append(self, (guint8 *)&tmp, sizeof(tmp));
  return TRUE;
}

/**
 * qmi_message_tlv_write_complete:
 * @self: a #QmiMessage.
 * @tlv_offset: offset that was returned by qmi_message_tlv_write_init().
 * @error: return location for error or %NULL.
 *
 * Completes building a TLV in the #QmiMessage.
 *
 * In case of error the TLV will be reseted; i.e. there is no need to explicitly
 * call qmi_message_tlv_write_reset().
 *
 * Returns: %TRUE if the TLV is successfully completed, otherwise %FALSE is
 * returned and @error is set.
 *
 * Since: 1.12
 */
gboolean qmi_message_tlv_write_complete(QmiMessage *self, gsize tlv_offset,
                                        GError **error) {
  gsize tlv_length;
  struct tlv *tlv;

  g_return_val_if_fail(self != NULL, FALSE);
  g_return_val_if_fail(self->len >= (tlv_offset + sizeof(struct tlv)), FALSE);

  tlv_length = self->len - tlv_offset;
  if (tlv_length == sizeof(struct tlv)) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_TLV_NOT_FOUND,
                "Empty TLV, no value set");
    g_byte_array_set_size(self, tlv_offset);
    return FALSE;
  }

  /* Update length fields. */
  tlv = tlv_get_header(self, tlv_offset);
  tlv->length = GUINT16_TO_LE(tlv_length - sizeof(struct tlv));
  set_qmux_length(self, (guint16)(get_qmux_length(self) + tlv_length));
  set_all_tlvs_length(self, (guint16)(get_all_tlvs_length(self) + tlv_length));

  /* Make sure we didn't break anything. */
  g_assert(message_check(self, NULL));

  return TRUE;
}

/**
 * qmi_message_response_new:
 * @request: a request #QmiMessage.
 * @error: a #QmiProtocolError to set in the result TLV.
 *
 * Create a new response #QmiMessage for the specified @request.
 *
 * Returns: (transfer full): a newly created #QmiMessage. The returned value
 * should be freed with qmi_message_unref().
 */
QmiMessage *qmi_message_response_new(QmiMessage *request,
                                     QmiProtocolError error) {
  QmiMessage *response;
  gsize tlv_offset;

  response = qmi_message_new(qmi_message_get_service(request),
                             qmi_message_get_client_id(request),
                             qmi_message_get_transaction_id(request),
                             qmi_message_get_message_id(request));

  /* Set the response flag */
  if (message_is_control(request))
    ((struct full_message *)(((GByteArray *)response)->data))
        ->qmi.control.header.flags |= QMI_CTL_FLAG_RESPONSE;
  else
    ((struct full_message *)(((GByteArray *)response)->data))
        ->qmi.service.header.flags |= QMI_SERVICE_FLAG_RESPONSE;

  /* Add result TLV, should never fail */
  g_assert((tlv_offset = qmi_message_tlv_write_init(response, 0x02, NULL)) > 0);
  g_assert(qmi_message_tlv_write_guint16(
      response, QMI_ENDIAN_LITTLE, (error != QMI_PROTOCOL_ERROR_NONE), NULL));
  g_assert(
      qmi_message_tlv_write_guint16(response, QMI_ENDIAN_LITTLE, error, NULL));
  g_assert(qmi_message_tlv_write_complete(response, tlv_offset, NULL));

  /* We shouldn't create invalid response messages */
  g_assert(message_check(response, NULL));

  return response;
}

/**
 * qmi_message_tlv_read_init:
 * @self: a #QmiMessage.
 * @type: specific ID of the TLV to read.
 * @out_tlv_length: optional return location for the TLV size.
 * @error: return location for error or %NULL.
 *
 * Starts reading a given TLV from the #QmiMessage.
 *
 * Returns: the offset where the TLV starts, or 0 if an error happens.
 *
 * Since: 1.12
 */
gsize qmi_message_tlv_read_init(QmiMessage *self, guint8 type,
                                guint16 *out_tlv_length, GError **error) {
  struct tlv *tlv;
  guint16 tlv_length;

  g_return_val_if_fail(self != NULL, 0);
  g_return_val_if_fail(self->len > 0, 0);

  for (tlv = qmi_tlv_first(self); tlv; tlv = qmi_tlv_next(self, tlv)) {
    if (tlv->type == type) break;
  }

  if (!tlv) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_TLV_NOT_FOUND,
                "TLV 0x%02X not found", type);
    return 0;
  }

  tlv_length = GUINT16_FROM_LE(tlv->length);
  if (!tlv_length) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_TLV_NOT_FOUND,
                "TLV 0x%02X is empty", type);
    return 0;
  }

  if (((guint8 *)tlv_next(tlv)) > ((guint8 *)qmi_end(self))) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_TLV_TOO_LONG,
                "Invalid length for TLV 0x%02X: %" G_GUINT16_FORMAT, type,
                tlv_length);
    return 0;
  }

  if (out_tlv_length) *out_tlv_length = tlv_length;

  return (((guint8 *)tlv) - self->data);
}

static const guint8 *tlv_error_if_read_overflow(QmiMessage *self,
                                                gsize tlv_offset, gsize offset,
                                                gsize len, GError **error) {
  const guint8 *ptr;
  struct tlv *tlv;

  tlv = (struct tlv *)&(self->data[tlv_offset]);
  ptr = ((guint8 *)tlv) + sizeof(struct tlv) + offset;

  if (((guint8 *)(ptr + len) > (guint8 *)tlv_next(tlv)) ||
      ((guint8 *)(ptr + len) > (guint8 *)qmi_end(self))) {
    g_set_error(error, QMI_CORE_ERROR, QMI_CORE_ERROR_TLV_TOO_LONG,
                "Reading TLV would overflow");
    return NULL;
  }

  return ptr;
}

/**
 * qmi_message_tlv_read_guint8:
 * @self: a #QmiMessage.
 * @tlv_offset: offset that was returned by qmi_message_tlv_read_init().
 * @offset: address of the offset within the TLV value.
 * @out: return location for the read #guint8.
 * @error: return location for error or %NULL.
 *
 * Reads an unsigned byte from the TLV.
 *
 * @offset needs to point to a valid @gsize specifying the index to start
 * reading from within the TLV value (0 for the first item). If the variable
 * is successfully read, @offset will be updated to point past the read item.
 *
 * Returns: %TRUE if the variable is successfully read, otherwise %FALSE is
 * returned and @error is set.
 *
 * Since: 1.12
 */
gboolean qmi_message_tlv_read_guint8(QmiMessage *self, gsize tlv_offset,
                                     gsize *offset, guint8 *out,
                                     GError **error) {
  const guint8 *ptr;

  g_return_val_if_fail(self != NULL, FALSE);
  g_return_val_if_fail(offset != NULL, FALSE);
  g_return_val_if_fail(out != NULL, FALSE);

  if (!(ptr = tlv_error_if_read_overflow(self, tlv_offset, *offset,
                                         sizeof(*out), error)))
    return FALSE;

  *offset = *offset + 1;
  *out = *ptr;
  return TRUE;
}

/**
 * qmi_message_tlv_read_guint16:
 * @self: a #QmiMessage.
 * @tlv_offset: offset that was returned by qmi_message_tlv_read_init().
 * @offset: address of a the offset within the TLV value.
 * @endian: source endianness, which will be swapped to host byte order if
 * necessary.
 * @out: return location for the read #guint16.
 * @error: return location for error or %NULL.
 *
 * Reads an unsigned 16-bit integer from the TLV, in host byte order.
 *
 * @offset needs to point to a valid @gsize specifying the index to start
 * reading from within the TLV value (0 for the first item). If the variable
 * is successfully read, @offset will be updated to point past the read item.
 *
 * Returns: %TRUE if the variable is successfully read, otherwise %FALSE is
 * returned and @error is set.
 *
 * Since: 1.12
 */
gboolean qmi_message_tlv_read_guint16(QmiMessage *self, gsize tlv_offset,
                                      gsize *offset, QmiEndian endian,
                                      guint16 *out, GError **error) {
  const guint8 *ptr;

  g_return_val_if_fail(self != NULL, FALSE);
  g_return_val_if_fail(offset != NULL, FALSE);
  g_return_val_if_fail(out != NULL, FALSE);

  if (!(ptr = tlv_error_if_read_overflow(self, tlv_offset, *offset,
                                         sizeof(*out), error)))
    return FALSE;

  memcpy(out, ptr, 2);
  if (endian == QMI_ENDIAN_BIG)
    *out = GUINT16_FROM_BE(*out);
  else
    *out = GUINT16_FROM_LE(*out);
  *offset = *offset + 2;
  return TRUE;
}

guint16 __qmi_message_tlv_read_remaining_size(QmiMessage *self,
                                              gsize tlv_offset, gsize offset) {
  struct tlv *tlv;

  g_return_val_if_fail(self != NULL, FALSE);

  tlv = (struct tlv *)&(self->data[tlv_offset]);

  g_warn_if_fail(GUINT16_FROM_LE(tlv->length) >= offset);
  return (GUINT16_FROM_LE(tlv->length) >= offset
              ? (GUINT16_FROM_LE(tlv->length) - offset)
              : 0);
}
