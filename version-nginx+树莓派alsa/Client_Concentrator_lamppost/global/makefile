#created by zhangwei on 2017-04-10

include ../makefile.inc
OBJECTS=global.o
CFLAGS=-g $(INCLUDE_PATH) $(LIB_PATH)
LOCALSTATICLIB=libglobal.a

all:$(LOCALSTATICLIB)

$(LOCALSTATICLIB):$(OBJECTS)
	$(ARCH) $(LOCALSTATICLIB) $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm $(LOCALSTATICLIB)

