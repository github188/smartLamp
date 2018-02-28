#created by zhangwei on 2017-04-10

CC=gcc
ARCH=ar -rv
OBJECTS=netinf.o netinfo.o

all:libnetinf.a

libnetinf.a:$(OBJECTS)
	$(ARCH) libnetinf.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm libnetinf.a
