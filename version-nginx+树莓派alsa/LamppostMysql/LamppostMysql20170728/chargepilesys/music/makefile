#created by zhangwei on 2017-04-10

include ../makefile.inc
OBJECTS=music_proc.o
CFLAGS=-g $(INCLUDE_PATH) $(LIB_PATH)

all:libmusic.a

libmusic.a:$(OBJECTS)
	$(ARCH) libmusic.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm libmusic.a

