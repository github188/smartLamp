#created by zhangwei on 2017-04-10

CC=gcc
ARCH=ar -rv
OBJECTS=cJSON.o

all:libcJSON.a

libcJSON.a:$(OBJECTS)
	$(ARCH) libcJSON.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c $< -o $@

clean:
	rm libcJSON.a
