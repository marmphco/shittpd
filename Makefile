UNAME = ${shell uname}
INCLUDEDEPS = ${filter clean,${MAKECMDGOALS}}

ifeq "${UNAME}" "Darwin"
COMPILER = clang
FLAGS = -Wall -Wextra 
else
COMPILER = gcc
FLAGS = -Wall -Wextra -pthread
endif

BIN = shittpd
CSOURCE = shittpd.c sdlistener.c sdworker.c sdrequestqueue.c
OBJECTS = ${patsubst %.c,%.o,${CSOURCE}}

all: shittpd

run: shittpd
	./shittpd

${BIN}: ${OBJECTS}
	${COMPILER} ${FLAGS} -o ${BIN} ${OBJECTS}

%.o: %.c
	${COMPILER} ${FLAGS} -c $^

dependencies: ${CSOURCE}
	${COMPILER} -MM ${CSOURCE} > dependencies

clean:
	- rm ${OBJECTS}
	- rm dependencies
	- rm *.gch

ifeq "${INCLUDEDEPS}" ""
include dependencies
endif
