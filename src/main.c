#include <stdio.h>
#include "window.h"
#include "video.h"

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    setup_video_interface();
    
	create_main_window();
	printf("Hallo video projekt\n");

	gtk_main();
	return 0;
}
