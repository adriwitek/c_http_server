################################################################################
# aSERVIDOR HTTP EN C
################################################################################

# banderas de compilacion (PUEDE SER UTIL CAMBIARLAS)
CC = gcc
#CFLAGS =  -std=gnu11  -g -I -Wall -Wextra -Werror
CFLAGS =  -std=gnu11  -g -I -Wall -Wextra 

LIBRERIAS= cgi.a config.a daemon.a http_lib.a picohttpparser.a sockets.a
LIBRERIAS_FILES = lib/cgi.a lib/config.a lib/daemon.a lib/http_lib.a lib/picohttpparser.a lib/sockets.a

all:	clean server   



#LIBRERIAS
cgi.a: cgi.o
	ar -r lib/$@ obj/$^
config.a: config.o
	ar -r lib/$@  obj/$^
daemon.a: daemon.o
	ar -r lib/$@  obj/$^
http_lib.a: http_lib.o
	ar -r lib/$@  obj/$^
picohttpparser.a: picohttpparser.o
	ar -r lib/$@  obj/$^
sockets.a:  sockets.o
	ar -r lib/$@  obj/$^




server: src/server.c $(LIBRERIAS)
	$(CC) $(CFLAGS) src/server.c $(LIBRERIAS_FILES) -o $@  -lconfuse





#OBJETOS
cgi.o: srclib/cgi.c
	$(CC) $(CFLAGS)  $^ -c  -o obj/$@
config.o: srclib/config.c
	$(CC) $(CFLAGS)  $^ -c  -o obj/$@ -lconfuse
daemon.o: srclib/daemon.c
	$(CC) $(CFLAGS)  $^ -c  -o obj/$@
http_lib.o: srclib/http_lib.c
	$(CC) $(CFLAGS)  $^ -c  -o obj/$@
picohttpparser.o: srclib/picohttpparser.c
	$(CC) $(CFLAGS)  $^ -c  -o obj/$@
sockets.o: srclib/sockets.c
	$(CC) $(CFLAGS)  $^ -c  -o obj/$@






clean:
	rm -rf server obj/*.o lib/*.a *.o *.a
clear:
	clear

.PHONY: clean

