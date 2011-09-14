#include <stdio.h>
#include "window.h"

int main(int argc, char **argv) {
        gtk_init(&argc, &argv);

	create_main_window();
	printf("Hallo video projekt\n");

	gtk_main();
	return 0;
}
