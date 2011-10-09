#ifndef VIDEO_H
#define VIDEO_H

#define MAXVIDEONODE 10
#define ZEROMEM(x) memset (&(x), 0, sizeof(x))

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>


typedef enum {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
} io_method;

typedef struct {
    char device_str[255]; 
    char driver_str[255];
    char card_str[255];
    char bus_info_str[255];
} video_capture_device_t;

typedef struct {
    void* start;
    size_t length;
} capture_buffer_t; 

struct {
    int active;
    int file_descriptor;
    char device_str[255];
    char selected_device_str[255];
    capture_buffer_t* capture_buffer;
    unsigned int n_buffers;
    char* image_buffer;
    int image_buffer_size;
    int height;
    int width;
} capture_device_instance;

static io_method io = IO_METHOD_MMAP;
static unsigned int width = 640;
static unsigned int height = 480;

extern void setup_video_interface();
extern void cleanup_video_interface();
extern video_capture_device_t* create_capture_list();
extern void open_video_device(char* device_name);
extern int frameRead(void);
#endif