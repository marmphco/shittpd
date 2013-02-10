INCLUDEDEPS = ${filter clean,${MAKECMDGOALS}}
COMPILER = clang
FLAGS = -Wall -Wextra -std=ansi
BIN = shittpd
CSOURCE = shittpd.c sdlistener.c sdworker.c sdrequestqueue.c
OBJECTS = ${patsubst %.c,%.o,${CSOURCE}}

all: shittpd

run: shittpd
	./shittpd

${BIN}: ${OBJECTS}
	${COMPILER} -o ${BIN} ${OBJECTS}

%.o: %.c
	${COMPILER} -c $^

dependencies: ${CSOURCE}
	${COMPILER} -MM ${CSOURCE} > dependencies

clean:
	- rm ${OBJECTS}
	- rm dependencies
	- rm *.gch

ifeq "${INCLUDEDEPS}" ""
include dependencies
endif
