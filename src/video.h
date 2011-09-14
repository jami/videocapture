#ifndef VIDEO_H
#define VIDEO_H

#define MAXVIDEONODE 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

typedef struct {
    char device_str[255]; 
    char driver_str[255];
    char card_str[255];
    char bus_info_str[255];
} video_capture_device_t;

static video_capture_device_t* capture_device_list = NULL;

extern void setup_video_interface();
extern void cleanup_video_interface();

#endif