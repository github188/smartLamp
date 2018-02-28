#created by zhangwei on 2017-04-10

include ../makefile.inc
OBJECTS=envparams_proc.o
CFLAGS=-g $(INCLUDE_PATH) $(LIB_PATH)
LOCALSTATICLIB=libenvparamsproc.a

all:$(LOCALSTATICLIB)

$(LOCALSTATICLIB):$(OBJECTS)
	$(ARCH) $(LOCALSTATICLIB) $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm $(LOCALSTATICLIB)

