/** @file libuvc_internal.h
  * @brief Implementation-specific UVC constants and structures.
  * @cond include_hidden
  */
#ifndef LIBUVC_INTERNAL_H
#define LIBUVC_INTERNAL_H

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "utlist.h"

/** Converts an unaligned four-byte little-endian integer into an int32 */
#define DW_TO_INT(p) ((p)[0] | ((p)[1] << 8) | ((p)[2] << 16) | ((p)[3] << 24))
/** Converts an unaligned two-byte little-endian integer into an int16 */
#define SW_TO_SHORT(p) ((p)[0] | ((p)[1] << 8))
/** Converts an int16 into an unaligned two-byte little-endian integer */
#define SHORT_TO_SW(s, p) \
  (p)[0] = (s); \
  (p)[1] = (s) >> 8;
/** Converts an int32 into an unaligned four-byte little-endian integer */
#define INT_TO_DW(i, p) \
  (p)[0] = (i); \
  (p)[1] = (i) >> 8; \
  (p)[2] = (i) >> 16; \
  (p)[3] = (i) >> 24;

/** Selects the nth item in a doubly linked list. n=-1 selects the last item. */
#define DL_NTH(head, out, n) \
  do { \
    int dl_nth_i = 0; \
    LDECLTYPE(head) dl_nth_p = (head); \
    if ((n) < 0) { \
      while (dl_nth_p && dl_nth_i > (n)) { \
        dl_nth_p = dl_nth_p->prev; \
        dl_nth_i--; \
      } \
    } else { \
      while (dl_nth_p && dl_nth_i < (n)) { \
        dl_nth_p = dl_nth_p->next; \
        dl_nth_i++; \
      } \
    } \
    (out) = dl_nth_p; \
  } while (0);

/** Video interface subclass code (A.2) */
enum uvc_int_subclass_code {
  UVC_SC_UNDEFINED = 0x00,
  UVC_SC_VIDEOCONTROL = 0x01,
  UVC_SC_VIDEOSTREAMING = 0x02,
  UVC_SC_VIDEO_INTERFACE_COLLECTION = 0x03
};

/** Video interface protocol code (A.3) */
enum uvc_int_proto_code {
  UVC_PC_PROTOCOL_UNDEFINED = 0x00
};

/** VideoControl interface descriptor subtype (A.5) */
enum uvc_vc_desc_subtype {
  UVC_VC_DESCRIPTOR_UNDEFINED = 0x00,
  UVC_VC_HEADER = 0x01,
  UVC_VC_INPUT_TERMINAL = 0x02,
  UVC_VC_OUTPUT_TERMINAL = 0x03,
  UVC_VC_SELECTOR_UNIT = 0x04,
  UVC_VC_PROCESSING_UNIT = 0x05,
  UVC_VC_EXTENSION_UNIT = 0x06
};

/** VideoStreaming interface descriptor subtype (A.6) */
enum uvc_vs_desc_subtype {
  UVC_VS_UNDEFINED = 0x00,
  UVC_VS_INPUT_HEADER = 0x01,
  UVC_VS_OUTPUT_HEADER = 0x02,
  UVC_VS_STILL_IMAGE_FRAME = 0x03,
  UVC_VS_FORMAT_UNCOMPRESSED = 0x04,
  UVC_VS_FRAME_UNCOMPRESSED = 0x05,
  UVC_VS_FORMAT_MJPEG = 0x06,
  UVC_VS_FRAME_MJPEG = 0x07,
  UVC_VS_FORMAT_MPEG2TS = 0x0a,
  UVC_VS_FORMAT_DV = 0x0c,
  UVC_VS_COLORFORMAT = 0x0d,
  UVC_VS_FORMAT_FRAME_BASED = 0x10,
  UVC_VS_FRAME_FRAME_BASED = 0x11,
  UVC_VS_FORMAT_STREAM_BASED = 0x12
};

/** UVC endpoint descriptor subtype (A.7) */
enum uvc_ep_desc_subtype {
  UVC_EP_UNDEFINED = 0x00,
  UVC_EP_GENERAL = 0x01,
  UVC_EP_ENDPOINT = 0x02,
  UVC_EP_INTERRUPT = 0x03
};

/** VideoControl interface control selector (A.9.1) */
enum uvc_vc_ctrl_selector {
  UVC_VC_CONTROL_UNDEFINED = 0x00,
  UVC_VC_VIDEO_POWER_MODE_CONTROL = 0x01,
  UVC_VC_REQUEST_ERROR_CODE_CONTROL = 0x02
};

/** Terminal control selector (A.9.2) */
enum uvc_term_ctrl_selector {
  UVC_TE_CONTROL_UNDEFINED = 0x00
};

/** Selector unit control selector (A.9.3) */
enum uvc_su_ctrl_selector {
  UVC_SU_CONTROL_UNDEFINED = 0x00,
  UVC_SU_INPUT_SELECT_CONTROL = 0x01
};

/** Extension unit control selector (A.9.6) */
enum uvc_xu_ctrl_selector {
  UVC_XU_CONTROL_UNDEFINED = 0x00
};

/** VideoStreaming interface control selector (A.9.7) */
enum uvc_vs_ctrl_selector {
  UVC_VS_CONTROL_UNDEFINED = 0x00,
  UVC_VS_PROBE_CONTROL = 0x01,
  UVC_VS_COMMIT_CONTROL = 0x02,
  UVC_VS_STILL_PROBE_CONTROL = 0x03,
  UVC_VS_STILL_COMMIT_CONTROL = 0x04,
  UVC_VS_STILL_IMAGE_TRIGGER_CONTROL = 0x05,
  UVC_VS_STREAM_ERROR_CODE_CONTROL = 0x06,
  UVC_VS_GENERATE_KEY_FRAME_CONTROL = 0x07,
  UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL = 0x08,
  UVC_VS_SYNC_DELAY_CONTROL = 0x09
};

/** USB terminal type (B.1) */
enum uvc_term_type {
  UVC_TT_VENDOR_SPECIFIC = 0x0100,
  UVC_TT_STREAMING = 0x0101
};

/** Input terminal type (B.2) */
enum uvc_it_type {
  UVC_ITT_VENDOR_SPECIFIC = 0x0200,
  UVC_ITT_CAMERA = 0x0201,
  UVC_ITT_MEDIA_TRANSPORT_INPUT = 0x0202
};

/** Output terminal type (B.3) */
enum uvc_ot_type {
  UVC_OTT_VENDOR_SPECIFIC = 0x0300,
  UVC_OTT_DISPLAY = 0x0301,
  UVC_OTT_MEDIA_TRANSPORT_OUTPUT = 0x0302
};

/** External terminal type (B.4) */
enum uvc_et_type {
  UVC_EXTERNAL_VENDOR_SPECIFIC = 0x0400,
  UVC_COMPOSITE_CONNECTOR = 0x0401,
  UVC_SVIDEO_CONNECTOR = 0x0402,
  UVC_COMPONENT_CONNECTOR = 0x0403
};

/** Status packet type (2.4.2.2) */
enum uvc_status_type {
  UVC_STATUS_TYPE_CONTROL = 1,
  UVC_STATUS_TYPE_STREAMING = 2
};

/** Payload header flags (2.4.3.3) */
#define UVC_STREAM_EOH (1 << 7)
#define UVC_STREAM_ERR (1 << 6)
#define UVC_STREAM_STI (1 << 5)
#define UVC_STREAM_RES (1 << 4)
#define UVC_STREAM_SCR (1 << 3)
#define UVC_STREAM_PTS (1 << 2)
#define UVC_STREAM_EOF (1 << 1)
#define UVC_STREAM_FID (1 << 0)

/** Control capabilities (4.1.2) */
#define UVC_CONTROL_CAP_GET (1 << 0)
#define UVC_CONTROL_CAP_SET (1 << 1)
#define UVC_CONTROL_CAP_DISABLED (1 << 2)
#define UVC_CONTROL_CAP_AUTOUPDATE (1 << 3)
#define UVC_CONTROL_CAP_ASYNCHRONOUS (1 << 4)

struct uvc_format_desc;
struct uvc_frame_desc;
struct uvc_streaming_interface;
struct uvc_device_info;

/** Frame descriptor
 *
 * A "frame" is a configuration of a streaming format
 * for a particular image size at one of possibly several
 * available frame rates.
 */
typedef struct uvc_frame_desc {
  struct uvc_format_desc *parent;
  struct uvc_frame_desc *prev, *next;
  /** Type of frame, such as JPEG frame or uncompressed frme */
  enum uvc_vs_desc_subtype bDescriptorSubtype;
  /** Index of the frame within the list of specs available for this format */
  uint8_t bFrameIndex;
  uint8_t bmCapabilities;
  /** Image width */
  uint16_t wWidth;
  /** Image height */
  uint16_t wHeight;
  /** Bitrate of corresponding stream at minimal frame rate */
  uint32_t dwMinBitRate;
  /** Bitrate of corresponding stream at maximal frame rate */
  uint32_t dwMaxBitRate;
  /** Maximum number of bytes for a video frame */
  uint32_t dwMaxVideoFrameBufferSize;
  /** Default frame interval (in 100ns units) */
  uint32_t dwDefaultFrameInterval;
  /** Minimum frame interval for continuous mode (100ns units) */
  uint32_t dwMinFrameInterval;
  /** Maximum frame interval for continuous mode (100ns units) */
  uint32_t dwMaxFrameInterval;
  /** Granularity of frame interval range for continuous mode (100ns) */
  uint32_t dwFrameIntervalStep;
  /** Available frame rates, zero-terminated (in 100ns units) */
  uint32_t *intervals;
} uvc_frame_desc_t;

/** Format descriptor
 *
 * A "format" determines a stream's image type (e.g., raw YUYV or JPEG)
 * and includes many "frame" configurations.
 */
typedef struct uvc_format_desc {
  struct uvc_streaming_interface *parent;
  struct uvc_format_desc *prev, *next;
  /** Type of image stream, such as JPEG or uncompressed. */
  enum uvc_vs_desc_subtype bDescriptorSubtype;
  /** Identifier of this format within the VS interface's format list */
  uint8_t bFormatIndex;
  union {
    /** Format and depth specification for uncompressed stream */
    struct {
      uint8_t guidFormat[16];
      uint8_t bBitsPerPixel;
    };
    /** Flags for JPEG stream */
    uint8_t bmFlags;
  };
  /** Default {uvc_frame_desc} to choose given this format */
  uint8_t bDefaultFrameIndex;
  uint8_t bAspectRatioX;
  uint8_t bAspectRatioY;
  uint8_t bmInterlaceFlags;
  uint8_t bCopyProtect;
  /** Available frame specifications for this format */
  struct uvc_frame_desc *frame_descs;
} uvc_format_desc_t;


/** VideoStream interface */
typedef struct uvc_streaming_interface {
  struct uvc_device_info *parent;
  struct uvc_streaming_interface *prev, *next;
  /** Interface number */
  uint8_t bInterfaceNumber;
  /** Video formats that this interface provides */
  struct uvc_format_desc *format_descs;
  /** USB endpoint to use when communicating with this interface */
  uint8_t bEndpointAddress;
  uint8_t bTerminalLink;
} uvc_streaming_interface_t;

/** Representation of the interface that brings data into the UVC device */
typedef struct uvc_input_terminal {
  struct uvc_input_terminal *prev, *next;
  /** Index of the terminal within the device */
  uint8_t bTerminalID;
  /** Type of terminal (e.g., camera) */
  enum uvc_it_type wTerminalType;
  uint16_t wObjectiveFocalLengthMin;
  uint16_t wObjectiveFocalLengthMax;
  uint16_t wOcularFocalLength;
  /** Camera controls (meaning of bits given in {uvc_ct_ctrl_selector}) */
  uint64_t bmControls;
} uvc_input_terminal_t;

typedef struct uvc_output_terminal {
  struct uvc_output_terminal *prev, *next;
} uvc_output_terminal_t;

/** Represents post-capture processing functions */
typedef struct uvc_processing_unit {
  struct uvc_processing_unit *prev, *next;
  /** Index of the processing unit within the device */
  uint8_t bUnitID;
  /** Index of the terminal from which the device accepts images */
  uint8_t bSourceID;
  /** Processing controls (meaning of bits given in {uvc_pu_ctrl_selector}) */
  uint64_t bmControls;
} uvc_processing_unit_t;

/** Custom processing or camera-control functions */
typedef struct uvc_extension_unit {
  struct uvc_extension_unit *prev, *next;
  /** Index of the extension unit within the device */
  uint8_t bUnitID;
  /** GUID identifying the extension unit */
  uint8_t guidExtensionCode[16];
  /** Bitmap of available controls (manufacturer-dependent) */
  uint64_t bmControls;
} uvc_extension_unit_t;

typedef struct uvc_control_interface {
  struct uvc_device_info *parent;
  struct uvc_input_terminal *input_term_descs;
  // struct uvc_output_terminal *output_term_descs;
  struct uvc_processing_unit *processing_unit_descs;
  struct uvc_extension_unit *extension_unit_descs;
  uint16_t bcdUVC;
  uint8_t bEndpointAddress;
} uvc_control_interface_t;

struct uvc_stream_ctrl;

struct uvc_device {
  struct uvc_context *ctx;
  int ref;
  libusb_device *usb_dev;
};

typedef struct uvc_device_info {
  /** Configuration descriptor for USB device */
  struct libusb_config_descriptor *config;
  /** VideoControl interface provided by device */
  uvc_control_interface_t ctrl_if;
  /** VideoStreaming interfaces on the device */
  uvc_streaming_interface_t *stream_ifs;
} uvc_device_info_t;

/** Handle on an open UVC device
 *
 * @todo move most of this into a uvc_device struct?
 */
struct uvc_device_handle {
  struct uvc_device *dev;
  struct uvc_device_handle *prev, *next;
  /** Underlying USB device handle */
  libusb_device_handle *usb_devh;
  struct uvc_device_info *info;
  struct libusb_transfer *status_xfer;
  uint8_t status_buf[8];
  /** if true, device is streaming video to host */
  uint8_t streaming;
  /** Current control block, valid iff streaming */
  struct uvc_stream_ctrl cur_ctrl;
  /** Whether the camera is an iSight that sends one header per frame */
  uint8_t is_isight;
  struct {
    /* listeners may only access hold*, and only when holding a 
     * lock on cb_mutex (probably signaled with cb_cond) */
    uint8_t stop;
    uint8_t fid;
    uint32_t seq, hold_seq;
    uint32_t pts, hold_pts;
    uint32_t last_scr, hold_last_scr;
    size_t got_bytes, hold_bytes;
    uint8_t *outbuf, *holdbuf;
    pthread_mutex_t cb_mutex;
    pthread_cond_t cb_cond;
    pthread_t cb_thread;
    uint32_t last_polled_seq;
    uvc_frame_callback_t *user_cb;
    void *user_ptr;
    struct libusb_transfer *transfers[5];
    uint8_t *transfer_bufs[5];
    struct uvc_frame frame;
    enum uvc_color_format color_format;
  } stream;
};

/** Context within which we communicate with devices */
struct uvc_context {
  /** Underlying context for USB communication */
  struct libusb_context *usb_ctx;
  /** True iff libuvc initialized the underlying USB context */
  uint8_t own_usb_ctx;
  /** List of open devices in this context */
  uvc_device_handle_t *open_devices;
  pthread_t handler_thread;
  uint8_t kill_handler_thread;
};

uvc_error_t uvc_query_stream_ctrl(
    uvc_device_handle_t *devh,
    uvc_stream_ctrl_t *ctrl,
    uint8_t probe,
    enum uvc_req_code req);

#endif // !def(LIBUVC_INTERNAL_H)
/** @endcond */

