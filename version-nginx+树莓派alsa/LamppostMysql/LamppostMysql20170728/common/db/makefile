

CC=gcc
ARCH=ar -rv
#CYGWIN_MACRO=-D_CYGWIN
OBJECTS=gdmysql.o

all:libgdmysql.a

libgdmysql.a:$(OBJECTS)
	$(ARCH) libgdmysql.a $(OBJECTS)
	rm *.o
$(OBJECTS):%.o:%.c
	$(CC) -c -I/usr/local/mysql/include  $< -o $@

clean:
	rm libgdmysql.a

