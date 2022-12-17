CC		:= gcc
CFLAGS	:= -Wall -Werror -g
LDFLAGS	:= -L. -lmfs

LIB		:= mfs.c

DEPS	:= udp.c

.PHONY: all
all: libmfs.so server nofile

server: server.o ${DEPS}
	${CC} ${CFLAGS} -o server server.o ${DEPS}

client: client.o libmfs.so
	${CC} ${CFLAGS} -o client client.o ${LDFLAGS}

libmfs.so : mfs.o ${DEPS}
	${CC} ${CFLAGS} -fPIC -shared -Wl,-soname,libmfs.so -o libmfs.so mfs.o udp.c -lc

clean:
	# rm -f ./client ./server *.o libmfs.so testfile

nofile:
	rm -f testfile

mfs.o : ${LIB} Makefile
	${CC} ${CFLAGS} -c -fPIC ${LIB}

%.o: %.c Makefile
	${CC} ${CFLAGS} -c $<