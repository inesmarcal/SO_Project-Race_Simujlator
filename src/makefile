CC=gcc
OBJS=mainv2.o 
FLAGS=-pthread -lrt -D_REENTRANT -Wall 
PROG=prog

all:	${PROG}

clean:
			rm ${OBJS} *~ ${PROG}

${PROG}:	${OBJS}
					${CC} ${FLAGS} ${OBJS} -o $@

.c.o:
			${CC} ${FLAGS} $< -c -o $@

###########################


mainv2.o:	mainv2.c mainv2.h race_Simulatorv2.c
