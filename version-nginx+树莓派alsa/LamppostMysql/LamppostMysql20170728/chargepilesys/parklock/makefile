#created by zhangwei on 2017-04-10

include ../makefile.inc
OBJECTS=parklock_proc.o LockCtrl.o
CFLAGS=-g $(INCLUDE_PATH) $(LIB_PATH)

all:libparklock.a

libparklock.a:$(OBJECTS)
	$(ARCH) libparklock.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c -I/usr/local/mysql/include $(CFLAGS) $< -o $@

clean:
	rm libparklock.a

