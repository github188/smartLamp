#created by zhangwei on 2017-04-10

include ../makefile.inc
OBJECTS=jsondata.o servdata.o
CFLAGS=-g $(INCLUDE_PATH) $(LIB_PATH)

all:libmuxdemux.a

libmuxdemux.a:$(OBJECTS)
	$(ARCH) libmuxdemux.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm libmuxdemux.a

