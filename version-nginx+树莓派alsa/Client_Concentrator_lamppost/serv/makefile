#created by zhangwei on 2017-04-10

include ../makefile.inc

EXEC=LamppostTerminal
OBJECTS=serv.o

CFLAGS=-g $(INCLUDE_PATH)

all:$(EXEC)
$(EXEC):$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXEC) $(LIB_PATH) $(LIB)
$(OBJECTS):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm *.o
	rm $(EXEC)
