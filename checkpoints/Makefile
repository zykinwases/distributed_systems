SHELL=/bin/sh
BENCHMARK=is
BENCHMARKU=IS

include ./make.def

include ./make.common

OBJS = is.o ${COMMON}/c_print_results.o ${COMMON}/c_timers.o

run: ${PROGRAM}
	mpirun --hostfile hostfile -n ${N} ${PROGRAM} 
    
${PROGRAM}: setparams config ${OBJS}
	${CLINK} ${CLINKFLAGS} -o ${PROGRAM} ${OBJS} ${CMPI_LIB}

.c.o:
	${CCOMPILE} $<

is.o:             is.c  npbparams.h

setparams: setparams.c ./make.def
	$(CC) ${CONVERTFLAG} -o setparams setparams.c

clean:
	- rm -f *.o *~ mputil*
	- rm -f is npbparams.h core
	- rm -f setparams setparams.h npbparams.h
	- rm -f is.*.x
	- rm -f *.txt
