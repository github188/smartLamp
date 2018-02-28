#created by zhangwei on 2017-04-10

CC=gcc
ARCH=ar -rv
OBJECTS=frame.o

all:libframe.a

libframe.a:$(OBJECTS)
	$(ARCH) libframe.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm libframe.a

