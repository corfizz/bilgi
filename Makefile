CFLAGS	+=	-Wall -std=c99 -g
PROG	=	bilgi
MANDIR	?=	/usr/share/man

${PROG}: ${PROG}.c
	@${CC} ${CFLAGS} -o ${PROG} ${PROG}.c

clean:
	@rm -f ${PROG}



