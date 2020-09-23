#include "libuvc/libuvc.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>

static volatile int total_frame_count = 0;
static int enable_dump = 0;

void dumpee(const void *p, size_t size)
{
  char filename[256];
  snprintf(filename, sizeof(filename), "/tmp/raw-frame-%04d.data", total_frame_count);
  FILE *f = fopen(filename, "wb");
  if(f)
  {
    fwrite(p, size, 1, f);
    fclose(f);
  }
  else{
    printf("fail to open file %s\n", filename);
  }
}

/* This callback function runs once per frame. Use it to perform any
 * quick processing you need, or have it put the frame into your application's
 * input queue. If this function takes too long, you'll start losing frames. */
void cb(uvc_frame_t *frame, void *ptr) {
  uvc_frame_t *bgr;
  uvc_error_t ret;
  enum uvc_frame_format *frame_format = (enum uvc_frame_format *)ptr;

  switch (frame->frame_format) {
  case UVC_FRAME_FORMAT_GRAY16:
    {
      if(enable_dump)
      {
        dumpee(frame->data, frame->data_bytes);
      }

      IplImage*cvImg = cvCreateImageHeader(
        cvSize(frame->width, frame->height),
        IPL_DEPTH_16U,
        1);
      cvSetData(cvImg, frame->data, frame->width*2); 
      cvNamedWindow("Test", CV_WINDOW_AUTOSIZE);
      cvShowImage("Test", cvImg);
      cvWaitKey(10);
   
      cvReleaseImageHeader(&cvImg);
      total_frame_count++;
    }
  default:
    break;
  }
}

void status_cb(enum uvc_status_class status_class,
                                    int event,
                                    int selector,
                                    enum uvc_status_attribute status_attribute,
                                    void *data, size_t data_len,
                                    void *user_ptr)
{
  printf("status callback(%d, %d, %d, %d, %p, %d)\n", status_class, event, selector, status_attribute, data, data_len);
}

int main(int argc, char **argv) {

  int format_idx = -1;
  int exposure = 0;
  int max_frame_count = -1;

  for(int i = 1; i < argc; ++i) {
    if(!strcmp(argv[i], "--format") || !strcmp(argv[i], "-f"))
      format_idx = atoi(argv[++i]);
    else if(!strcmp(argv[i], "--exposure") || !strcmp(argv[i], "-e"))
      exposure = atoi(argv[++i]);
    else if(!strcmp(argv[i], "-n"))
      max_frame_count = atoi(argv[++i]);
    else if (!strcmp(argv[i], "--dump") || !strcmp(argv[i], "-d"))
      enable_dump = 1;
  }

  uvc_context_t *ctx;
  uvc_device_t *dev;
  uvc_device_handle_t *devh;
  uvc_stream_ctrl_t ctrl;
  uvc_error_t res;

  /* Initialize a UVC service context. Libuvc will set up its own libusb
   * context. Replace NULL with a libusb_context pointer to run libuvc
   * from an existing libusb context. */
  res = uvc_init(&ctx, NULL);

  if (res < 0) {
    uvc_perror(res, "uvc_init");
    return res;
  }

  puts("UVC initialized");

  /* Locates the first attached UVC device, stores in dev */
  res = uvc_find_device(
      ctx, &dev,
      0, 0, NULL); /* filter devices: vendor_id, product_id, "serial_num" */

  if (res < 0) {
    uvc_perror(res, "uvc_find_device"); /* no devices found */
  } else {
    puts("Device found");

    /* Try to open the device: requires exclusive access */
    res = uvc_open(dev, &devh);

    if (res < 0) {
      uvc_perror(res, "uvc_open"); /* unable to open device */
    } else {
      puts("Device opened");

      uvc_set_status_callback(devh, status_cb, 0);

      /* Print out a message containing all the information that libuvc
       * knows about the device */
      uvc_print_diag(devh, stderr);

      const uvc_format_desc_t *format_desc = uvc_get_format_descs(devh);

      const uvc_frame_desc_t *frame_desc = format_desc->frame_descs->next;
      if(format_idx > -1)
      {
        while(format_idx > 0)
        {
          frame_desc = format_desc->frame_descs->next;
          format_idx--;
        }
      }

      enum uvc_frame_format frame_format;
      int width = 640;
      int height = 480;
      int fps = 30;

      switch (format_desc->bDescriptorSubtype) {
      case UVC_VS_FORMAT_MJPEG:
        frame_format = UVC_COLOR_FORMAT_MJPEG;
        break;
      case UVC_VS_FORMAT_FRAME_BASED:
        frame_format = UVC_FRAME_FORMAT_H264;
        break;
      default:
        frame_format = UVC_FRAME_FORMAT_GRAY16;
        break;
      }

      if (frame_desc) {
        width = frame_desc->wWidth;
        height = frame_desc->wHeight;
        fps = 10000000 / frame_desc->dwDefaultFrameInterval;
      }

      printf("\nFirst format: (%4s) %dx%d %dfps\n", format_desc->fourccFormat, width, height, fps);

      /* Try to negotiate first stream profile */
      res = uvc_get_stream_ctrl_format_size(
          devh, &ctrl, /* result stored in ctrl */
          frame_format,
          width, height, fps /* width, height, fps */
      );

      /* Print out the result */
      uvc_print_stream_ctrl(&ctrl, stderr);

      if (res < 0) {
        uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
      } else {
        /* Start the video stream. The library will call user function cb:
         *   cb(frame, (void *) 12345)
         */
        res = uvc_start_streaming(devh, &ctrl, cb, (void *) 12345, 0);

        if (res < 0) {
          uvc_perror(res, "start_streaming"); /* unable to start stream */
        } else {
          puts("Streaming...");

          uvc_set_ae_mode(devh, 2); /* e.g., turn on auto exposure */

          while(total_frame_count < max_frame_count || max_frame_count < 0)
          {
            usleep(1000);
          }

          /* End the stream. Blocks until last callback is serviced */
          uvc_stop_streaming(devh);
          puts("Done streaming.");
        }
      }

      /* Release our handle on the device */
      uvc_close(devh);
      puts("Device closed");
    }

    /* Release the device descriptor */
    uvc_unref_device(dev);
  }

  /* Close the UVC context. This closes and cleans up any existing device handles,
   * and it closes the libusb context if one was not provided. */
  uvc_exit(ctx);
  puts("UVC exited");

  return 0;
}

