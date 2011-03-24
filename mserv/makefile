CC=gcc
CFLAGS=-I/usr/include/nspr -g
LDFLAGS=-lnspr4 -lm
LDLIBS=-lewfutils -L.
OBJECTFILES=mserv_login.o mserv_config.o mserv_packet.o mserv_connection.o mserv_crypto.o

all: mserv
rebuild: clean all
clean:
	rm -f *.o;

mserv: $(OBJECTFILES)

mserv_login.o: mserv_login.c
mserv_config.o: mserv_config.c
mserv_packet.o: mserv_packet.c
mserv_connection.o: mserv_connection.c
mserv_crypto.o: mserv_crypto.c
