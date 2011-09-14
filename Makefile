CC=gcc
CFLAGS=-Wall -g `pkg-config --cflags gtk+-2.0`
LIBS=`pkg-config --libs gtk+-2.0`

OBJS=main.o window.o video.o 

PROGRAM=videocapture

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	rm -f $(PROGRAM)
	$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS)

%.o: src/%.c 
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f core *.o $(PROGNAME) nohup.out

distclean: clean 
	rm -f *~
