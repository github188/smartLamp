#created by zhangwei on 2017-04-10

CC=gcc
ARCH=ar -rv
#CYGWIN_MACRO=-D_CYGWIN
OBJECTS=log.o

all:liblog.a

liblog.a:$(OBJECTS)
	$(ARCH) liblog.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $(CYGWIN_MACRO) $< -o $@

clean:
	rm liblog.a

