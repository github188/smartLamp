#created by zhangwei on 2017-04-10

CC=gcc
ARCH=ar -rv
OBJECTS=md5.o uuid.o

all:libmd5.a

libmd5.a:$(OBJECTS)
	$(ARCH) libmd5.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm libmd5.a
