CC=gcc
CFLAGS=-Wall -g `pkg-config --cflags gtk+-2.0`
LIBS=`pkg-config --libs gtk+-2.0`

OBJS=main.o window.o 

divelog: $(OBJS)
	$(CC) $(LDFLAGS) -o videocapture $(OBJS) $(LIBS)

main.o: src/main.c 
	$(CC) $(CFLAGS) -c src/main.c

window.o: src/window.c 
	$(CC) $(CFLAGS) -c src/window.c
