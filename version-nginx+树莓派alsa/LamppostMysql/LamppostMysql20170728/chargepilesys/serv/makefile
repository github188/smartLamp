#created by zhangwei on 2017-04-10

include ../makefile.inc

EXEC=CPService
OBJECTS=serv.o upserv.o dealhash.o

CFLAGS=-g $(INCLUDE_PATH)

all:$(EXEC)
$(EXEC):$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXEC) $(LIB_PATH) $(LIB)
	cp $(EXEC) /usr/local/arm/smartlamppost/bin
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm *.o
	rm $(EXEC)
