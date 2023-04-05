#include "gst/gstpad.h"
#include "util.hpp"
//
// #include <array>
// #include <cmath>
// #include <complex>
// #include <cstdlib>
// #include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
// #include <vector>
// #include <unistd.h>

//

#include <gst/gst.h>
#include <gst/video/video.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *const pipeline_str =
    "v4l2src device=/dev/video0 ! video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! videoconvert! video/x-raw,format=BGR ! appsink name=sink sync=f";

/*
  This function will be called in a separate thread when our appsink
  says there is data for us. user_data has to be defined
  when calling g_signal_connect. It can be used to pass objects etc.
  from your other function to the callback.

  user_data can point to additional data for your usage
            marked as unused to prevent compiler warnings
*/


cv::Mat images[3];

static GstFlowReturn callback(GstElement* sink, void* user_data __attribute__((unused)))
{
    GstSample* sample = NULL;
    bool *capture = (bool*)(user_data);
    /* Retrieve the buffer */
    g_signal_emit_by_name(sink, "pull-sample", &sample, NULL);

    if (sample)
    {
        // we have a valid sample
        // do things with the image here
        static guint framecount = 0;
        int pixel_data = -1;

        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo info; // contains the actual image
        if (gst_buffer_map(buffer, &info, GST_MAP_READ))
        {
            GstVideoInfo* video_info = gst_video_info_new();
            if (!gst_video_info_from_caps(video_info, gst_sample_get_caps(sample)))
            {
                // Could not parse video info (should not happen)
                g_warning("Failed to parse video info");
                return GST_FLOW_ERROR;
            }

            // pointer to the image data

            // unsigned char* data = info.data;

            // Get the pixel value of the center pixel

            int stride = video_info->finfo->bits / 8;
            unsigned int pixel_offset = video_info->width / 2 * stride
                                        + video_info->width * video_info->height / 2 * stride;

            // this is only one pixel
            // when dealing with formats like BGRx
            // pixel_data will consist out of
            // pixel_offset   => B
            // pixel_offset+1 => G
            // pixel_offset+2 => R
            // pixel_offset+3 => x

            // pixel_data = info.data[pixel_offset];
            if (*capture){
              images[framecount%3]=cv::Mat(cv::Size(video_info->width,video_info->height),
                  CV_8UC3, info.data, cv::Mat::AUTO_STEP);
              std::cout<<"captured"<<framecount%3<<std::endl;
              framecount++;
              *capture= false;
              
            }
            //
            // cv::imshow("CSI Camera 1", image);
            // cv::waitKey(1);
            
            gst_buffer_unmap(buffer, &info);
            gst_video_info_free(video_info);
        }

        //

        // delete our reference so that gstreamer can handle the sample
        gst_sample_unref(sample);
    }
    return GST_FLOW_OK;
}


int main(int argc, char* argv[])
{
    /* this line sets the gstreamer default logging level
       it can be removed in normal applications
       gstreamer logging can contain verry useful information
       when debugging your application
       # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
       for further details
    */
    std::system("v4l2-ctl -d /dev/video0 -c auto_exposure=1");
    std::system("v4l2-ctl -d /dev/video0 -c exposure_dynamic_framerate=0");
    gst_debug_set_default_threshold(GST_LEVEL_WARNING);

    gst_init(&argc, &argv);


    GError* err = NULL;
    GstElement* pipeline = gst_parse_launch(pipeline_str, &err);

    /* test for error */
    if (pipeline == NULL)
    {
        printf("Could not create pipeline. Cause: %s\n", err->message);
        return 1;
    }


    /* retrieve the appsink from the pipeline */
    GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

    // tell appsink to notify us when it receives an image
    g_object_set(G_OBJECT(sink), "emit-signals", TRUE, NULL);

    // tell appsink what function to call when it notifies us
    bool take_frame=false;
    g_signal_connect(sink, "new-sample", G_CALLBACK(callback), &take_frame);

    gst_object_unref(sink);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    g_print("Press 'enter' to stop the stream.\n");

    /* wait for user input to end the program */

    cv::Mat merged;
    sleep(2);
    while (true) {

      std::system("v4l2-ctl -d /dev/video0 -c exposure_time_absolute=0");
      usleep(1000e3);
      take_frame=true;
      while (take_frame);
      std::system("v4l2-ctl -d /dev/video0 -c exposure_time_absolute=100");
      usleep(1000e3);
      take_frame=true;
      while (take_frame);
      std::system("v4l2-ctl -d /dev/video0 -c exposure_time_absolute=1000");
      usleep(1000e3);
      take_frame=true;
      while (take_frame);

      
      cv::vconcat(images[0],images[1],merged);
      cv::vconcat(merged,images[2],merged);

      cv::imshow("CSI Camera 1", merged);

      int keycode = cv::waitKey(2000) & 0xff;
      if (keycode == 27)
        break;
    
    }
    

    // this stops the pipeline and frees all resources
    gst_element_set_state(pipeline, GST_STATE_NULL);

    /*
      the pipeline automatically handles
      all elements that have been added to it.
      thus they do not have to be cleaned up manually
    */
    gst_object_unref(pipeline);

    return 0;
}
