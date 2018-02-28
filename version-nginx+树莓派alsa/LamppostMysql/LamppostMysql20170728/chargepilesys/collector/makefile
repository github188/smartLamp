#created by zhangwei on 2017-04-10

include ../makefile.inc
OBJECTS=collector_proc.o
CFLAGS=-g $(INCLUDE_PATH) $(LIB_PATH)

all:libcollector.a

libcollector.a:$(OBJECTS)
	$(ARCH) libcollector.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm libcollector.a

