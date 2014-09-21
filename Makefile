

CC=gcc
CFLAGS= -Wall
LDFLAGS=

all: covert

covert: client_ops.o server_ops.o util.o socket_ops.o covert_main.o
	${CC} ${CFLAGS} client_ops.o server_ops.o util.o socket_ops.o covert_main.o -o covert 

covert_main.o: covert_main.c covert.h client_ops.h server_ops.h util.h socket_ops.h
	${CC} ${CFLAGS} -c covert_main.c

client_ops.o : client_ops.c client_ops.h covert.h socket_ops.h util.h
	${CC} ${CFLAGS} -c client_ops.c

server_ops.o: server_ops.c server_ops.h covert.h socket_ops.h util.h
	${CC} ${CFLAGS} -c server_ops.c

util.o: util.c util.h covert.h
	${CC} ${CFLAGS} -c util.c

socket_ops.o: socket_ops.c socket_ops.h covert.h
	${CC} ${CFLAGS} -c socket_ops.c

clean:
	rm -rf *.o *.c~ *.h~ ./covert
