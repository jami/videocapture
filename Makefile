CC=gcc
CFLAGS=-Wall -g `pkg-config --cflags gtk+-2.0`
LIBS=`pkg-config --libs gtk+-2.0`

OBJS=main.o window.o 

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


#divelog: $(OBJS)
#	$(CC) $(LDFLAGS) -o videocapture $(OBJS) $(LIBS)
#
#main.o: src/main.c 
#	$(CC) $(CFLAGS) -c src/main.c
##
#window.o: src/window.c 
#	$(CC) $(CFLAGS) -c src/window.c
