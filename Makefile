CC = gcc
AR = ar
CFLAGS = -c -g -fpic -Wall -static -lpthread
C_INCLUDE = -I.
C_STUB = stub.c
C_OBJECTS = $(C_STUB:.c=.o)
LIB_PATH = /usr/lib
INCLUDE_PATH = /usr/include

all:$(C_OBJECTS)
	$(AR) -cr lib$(C_STUB:.c=.a) $(C_OBJECTS)
	$(CC) -shared -fpic -o lib$(C_STUB:.c=.so) $(C_OBJECTS)
	cp lib* $(LIB_PATH)
	cp stub.h $(INCLUDE_PATH)

%.o:%.c
	$(CC) $(CFLAGS) $< $(LIB) -o $@

clean:
	rm -rf $(C_OBJECTS) lib*
