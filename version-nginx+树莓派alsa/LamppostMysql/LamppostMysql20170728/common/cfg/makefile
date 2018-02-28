#created by zhangwei on 2017-04-10

CC=gcc
ARCH=ar -rv
OBJECTS=config.o

all:libconfig.a

libconfig.a:$(OBJECTS)
	$(ARCH) libconfig.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm libconfig.a

