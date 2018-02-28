#created by zhangwei on 2017-04-10

include ../makefile.inc
OBJECTS=envir_proc.o
CFLAGS=-g $(INCLUDE_PATH) $(LIB_PATH)

all:libenvir.a

libenvir.a:$(OBJECTS)
	$(ARCH) libenvir.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm libenvir.a

