#created by zhangwei on 2017-04-10

CC=gcc
ARCH=ar -rv
OBJECTS=thpool.o

all:libthpool.a

libthpool.a:$(OBJECTS)
	$(ARCH) libthpool.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm libthpool.a
