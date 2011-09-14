#include "video.h"

/* get a list of all available devices */
void create_capture_list() {
    char dev_node_str[255];

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

        video_capture_device_t* node = &capture_device_list[list_length];
        
        strncpy(node->device_str, dev_node_str, 255);
        strncpy(node->driver_str, (char*)cap.driver, 255);
        strncpy(node->card_str, (char*)cap.card, 255);
        strncpy(node->bus_info_str, (char*)cap.bus_info, 255);
        
        list_length++;
    }
}
/* free the list */
void free_capture_list() {

}
/* setup all video devices */
void setup_video_interface() {
    create_capture_list();
}
/*  */
void cleanup_video_interface() {
    free_capture_list();
}
