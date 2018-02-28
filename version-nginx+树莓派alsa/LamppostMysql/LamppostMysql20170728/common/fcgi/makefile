#created by zhangwei on 2017-04-10

CC=gcc
ARCH=ar -rv
OBJECTS=os_unix.o fcgiapp.o fcgi_stdio.o

all:libfcgi.a

libfcgi.a:$(OBJECTS)
	$(ARCH) libfcgi.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm libfcgi.a

