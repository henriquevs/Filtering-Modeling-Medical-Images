CC      = gcc
CXX     = gcc
RM      = rm

CFLAGS 	= -O3

SRCS	= main.c
OBJS	= $(SRCS:.c=.o)
PRG	= iprocessing

all : ${OBJS}
	${CC} ${OBJS} -o ${PRG}

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) -f ${PRG} ${OBJS} *~ results.pgm


