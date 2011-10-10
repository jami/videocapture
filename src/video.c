#include "video.h"

void errno_exit(const char* s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

int xioctl(int fd, int request, void* argp) {
    int r;

    do {
        r = ioctl(fd, request, argp);
    } while (-1 == r && EINTR == errno);

    return r;
}

void readInit(unsigned int buffer_size) {
    capture_device_instance.capture_buffer = calloc(1, sizeof(*capture_device_instance.capture_buffer));

    if (!capture_device_instance.capture_buffer) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    capture_device_instance.capture_buffer[0].length = buffer_size;
    capture_device_instance.capture_buffer[0].start = malloc(buffer_size);

    if (!capture_device_instance.capture_buffer[0].start) {
        fprintf (stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
}

void YUV422toRGB888(unsigned char *src) {
    int line, column;
    unsigned char *py, *pu, *pv;
    unsigned char *tmp = (unsigned char*)capture_device_instance.image_buffer;

    /* In this format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr. 
     Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels. */
    py = src;
    pu = src + 1;
    pv = src + 3;

    #define CLIP(x) ( (x)>=0xFF ? 0xFF : ( (x) <= 0x00 ? 0x00 : (x) ) )

    for (line = 0; line < capture_device_instance.height; ++line) {
        for (column = 0; column < capture_device_instance.width; ++column) {
            *tmp++ = CLIP((double)*py + 1.402*((double)*pv-128.0));
            *tmp++ = CLIP((double)*py - 0.344*((double)*pu-128.0) - 0.714*((double)*pv-128.0));      
            *tmp++ = CLIP((double)*py + 1.772*((double)*pu-128.0));

            // increase py every time
            py += 2;
            // increase pu,pv every second time
            if ((column & 1)==1) {
                pu += 4;
                pv += 4;
            }
        }
    }
}

void imageProcess(const void* p) {
    unsigned char* src = (unsigned char*)p;
    
    // convert from YUV422 to RGB888
    YUV422toRGB888(src);
}

/* read a frame */
int frameRead(void) {
    struct v4l2_buffer buf;
    unsigned int i;

    switch (io) {
        case IO_METHOD_READ:
            if (-1 == read(capture_device_instance.file_descriptor, 
                capture_device_instance.capture_buffer[0].start, 
                capture_device_instance.capture_buffer[0].length)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;
                    case EIO:
                        // Could ignore EIO, see spec.
                        // fall through
                    default:
                        errno_exit("read");
                }
            }

            imageProcess(capture_device_instance.capture_buffer[0].start);
            break;
        case IO_METHOD_MMAP:
            ZEROMEM(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;
                    case EIO:
                        // Could ignore EIO, see spec
                        // fall through
                    default:
                        errno_exit("VIDIOC_DQBUF");
                }
            }

            imageProcess(capture_device_instance.capture_buffer[buf.index].start);

            if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
            break;
        case IO_METHOD_USERPTR:
            ZEROMEM(buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;
                    case EIO:
                        // Could ignore EIO, see spec.
                        // fall through
                    default:
                        errno_exit("VIDIOC_DQBUF");
                }
            }

            for (i = 0; i < capture_device_instance.n_buffers; ++i)
                if (buf.m.userptr == (unsigned long) capture_device_instance.capture_buffer[i].start 
                    && buf.length == capture_device_instance.capture_buffer[i].length)
                    break;

            imageProcess((void*)buf.m.userptr);

            if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
            break;
    }

    return 1;
}

void mmapInit(void) {
    struct v4l2_requestbuffers req;
    enum v4l2_buf_type type;
    ZEROMEM(req);

    req.count  = 4;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support memory mapping\n", capture_device_instance.device_str);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n", capture_device_instance.device_str);
        exit(EXIT_FAILURE);
    }

    capture_device_instance.capture_buffer = calloc(req.count, sizeof(*capture_device_instance.capture_buffer));

    if (!capture_device_instance.capture_buffer) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (capture_device_instance.n_buffers = 0; capture_device_instance.n_buffers < req.count; ++capture_device_instance.n_buffers) {
        struct v4l2_buffer buf;
        ZEROMEM(buf);

        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = capture_device_instance.n_buffers;

        if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_QUERYBUF, &buf)) {
            errno_exit("VIDIOC_QUERYBUF");
        }

        capture_device_instance.capture_buffer[capture_device_instance.n_buffers].length = buf.length;
        capture_device_instance.capture_buffer[capture_device_instance.n_buffers].start = mmap(
            NULL, 
            buf.length, 
            PROT_READ | PROT_WRITE, 
            MAP_SHARED, 
            capture_device_instance.file_descriptor, 
            buf.m.offset
        );

        if (MAP_FAILED == capture_device_instance.capture_buffer[capture_device_instance.n_buffers].start) {
            errno_exit("mmap");
        }
    }

    // prepare capturing
    int i;
    for (i = 0; i < capture_device_instance.n_buffers; ++i) {
        struct v4l2_buffer buf;
        ZEROMEM(buf);

        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = i;

        if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
    }
                
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_STREAMON, &type))
        errno_exit("VIDIOC_STREAMON");
}

void userptrInit(unsigned int buffer_size) {
    struct v4l2_requestbuffers req;
    enum v4l2_buf_type type;
    unsigned int page_size;
    int i;
    
    page_size = getpagesize();
    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    ZEROMEM(req);

    req.count  = 4;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support user pointer i/o\n", capture_device_instance.device_str);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    capture_device_instance.capture_buffer = calloc(4, sizeof(*capture_device_instance.capture_buffer));

    if (!capture_device_instance.capture_buffer) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (capture_device_instance.n_buffers = 0; capture_device_instance.n_buffers < 4; ++capture_device_instance.n_buffers) {
        capture_device_instance.capture_buffer[capture_device_instance.n_buffers].length = buffer_size;
        capture_device_instance.capture_buffer[capture_device_instance.n_buffers].start = memalign(page_size, buffer_size);

        if (!capture_device_instance.capture_buffer[capture_device_instance.n_buffers].start) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < capture_device_instance.n_buffers; ++i) {
        struct v4l2_buffer buf;
        ZEROMEM(buf);

        buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory    = V4L2_MEMORY_USERPTR;
        buf.index     = i;
        buf.m.userptr = (unsigned long)capture_device_instance.capture_buffer[i].start;
        buf.length    = capture_device_instance.capture_buffer[i].length;

        if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_QBUF, &buf)) {
            errno_exit("VIDIOC_QBUF");
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_STREAMON, &type))
        errno_exit("VIDIOC_STREAMON");
}

void close_video_device() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (capture_device_instance.active) {
        unsigned int i;

        switch (io) {
            case IO_METHOD_READ:
                free(capture_device_instance.capture_buffer[0].start);
                break;
            case IO_METHOD_MMAP:
                if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_STREAMOFF, &type)) {
                    errno_exit("VIDIOC_STREAMOFF");
                }
                for (i = 0; i < capture_device_instance.n_buffers; ++i) {
                    if (-1 == munmap(
                        capture_device_instance.capture_buffer[i].start, 
                        capture_device_instance.capture_buffer[i].length)) {
                        errno_exit("munmap");
                    }
                }
        
                break;
            case IO_METHOD_USERPTR:
                if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_STREAMOFF, &type)) {
                    errno_exit("VIDIOC_STREAMOFF");
                }
                
                for (i = 0; i < capture_device_instance.n_buffers; ++i) {
                    free(capture_device_instance.capture_buffer[i].start);
                }
                break;
        }

        if (-1 == close(capture_device_instance.file_descriptor)) {
            fprintf(stderr, "Cannot identify '%s': %d, %s\n", capture_device_instance.device_str, errno, strerror(errno));
        }
    }

    capture_device_instance.active = 0;
    if (capture_device_instance.image_buffer_size) {
        free(capture_device_instance.image_buffer);
        capture_device_instance.image_buffer_size = 0;
    }
}

void open_video_device(char* device_name) {
    if (strncmp(capture_device_instance.device_str, device_name, 255) == 0) {
        if (capture_device_instance.active == 1) {
            return;
        }
    }

    if (capture_device_instance.active == 1) {
        close_video_device();
    }

    strncpy(capture_device_instance.device_str, device_name, 255);
    
    struct stat st;
    // stat file
    if (-1 == stat(device_name, &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", device_name, errno, strerror(errno));
        return;
    }

    // check if its device
    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", device_name);
        return;
    }

    // open device
    capture_device_instance.file_descriptor = open(device_name, O_RDWR | O_NONBLOCK, 0);

    // check if opening was successfull
    if (-1 == capture_device_instance.file_descriptor) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", device_name, errno, strerror(errno));
        return;
    }

    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no V4L2 device\n", device_name);
        } else {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n",device_name);
        exit(EXIT_FAILURE);
    }

    switch (io) {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                fprintf(stderr, "%s does not support read i/o\n",device_name);
                exit(EXIT_FAILURE);
            }
            break;
        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                fprintf(stderr, "%s does not support streaming i/o\n",device_name);
                exit(EXIT_FAILURE);
            }
            break;
    }

    /* Select video input, video standard and tune here. */
    ZEROMEM(cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(capture_device_instance.file_descriptor, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
         }
    } else {        
        /* Errors ignored. */
    }

    ZEROMEM(fmt);

    // v4l2_format
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = width; 
    fmt.fmt.pix.height      = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (-1 == xioctl(capture_device_instance.file_descriptor, VIDIOC_S_FMT, &fmt))
        errno_exit("VIDIOC_S_FMT");

    /* Note VIDIOC_S_FMT may change width and height. */
    if (width != fmt.fmt.pix.width) {
        width = fmt.fmt.pix.width;
        fprintf(stderr,"Image width set to %i by device %s.\n", width, device_name);
    }
    
    if (height != fmt.fmt.pix.height) {
        height = fmt.fmt.pix.height;
        fprintf(stderr,"Image height set to %i by device %s.\n", height, device_name);
    }

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    switch (io) {
        case IO_METHOD_READ:
            readInit(fmt.fmt.pix.sizeimage);
            break;
        case IO_METHOD_MMAP:
            mmapInit();
            break;
        case IO_METHOD_USERPTR:
            userptrInit(fmt.fmt.pix.sizeimage);
            break;
    }

    capture_device_instance.active = 1;
    // create rgb image buffer
    capture_device_instance.image_buffer_size = width * height * 3 * sizeof(char);
    capture_device_instance.image_buffer = malloc(capture_device_instance.image_buffer_size);
    capture_device_instance.width = width;
    capture_device_instance.height = height;
    
    printf("Device opend\n");
}

/* get a list of all available devices */
video_capture_device_t* create_capture_list() {
    char dev_node_str[255];
    video_capture_device_t* capture_device_list = NULL;
    
    int i, list_length = 0;
    for (i = 0; i < MAXVIDEONODE; i++) {
        int fd;
        sprintf(dev_node_str, "/dev/video%d", i);
        
        if ((fd = open(dev_node_str, O_RDONLY)) == -1) {
            continue;
        }

        printf("Found video device on %s\n", dev_node_str);

        struct v4l2_capability cap;   
        struct v4l2_cropcap cropcap;   
        struct v4l2_crop crop;   
        struct v4l2_format fmt;   
        unsigned int min;   

        ioctl(fd, VIDIOC_QUERYCAP, &cap);
        
        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {   
            printf("%s is no video capture device\n", dev_node_str);

            close(fd);
            continue;
        }   
        
        close(fd);

        capture_device_list = (video_capture_device_t*)realloc(capture_device_list, 
            (list_length + 1) * sizeof(video_capture_device_t));      
        
        strncpy(capture_device_list[list_length].device_str, dev_node_str, 255);
        strncpy(capture_device_list[list_length].driver_str, (char*)cap.driver, 255);
        strncpy(capture_device_list[list_length].card_str, (char*)cap.card, 255);
        strncpy(capture_device_list[list_length].bus_info_str, (char*)cap.bus_info, 255);
        
        list_length++;
    }

    return capture_device_list;
}
/* free the list */
void free_capture_list() {
    
}
/* setup all video devices */
void setup_video_interface() {
    capture_device_instance.active = 0;
}
/*  */
void cleanup_video_interface() {
    printf("Close device\n");
    close_video_device();
}
