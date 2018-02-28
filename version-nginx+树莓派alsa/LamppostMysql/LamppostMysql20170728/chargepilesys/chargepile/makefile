#created by zhangwei on 2017-04-10

include ../makefile.inc
OBJECTS=chargepile_proc.o
CFLAGS=-g $(INCLUDE_PATH) $(LIB_PATH)

all:libchargepile.a

libchargepile.a:$(OBJECTS)
	$(ARCH) libchargepile.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm libchargepile.a

