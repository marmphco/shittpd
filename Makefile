INCLUDEDEPS = ${filter clean,${MAKECMDGOALS}}
COMPILER = clang
FLAGS = -Wall -Wextra -std=ansi
BIN = shittpd
CSOURCE = *.c
OBJECTS = ${patsubst %.c,%.o,${CSOURCE}}

all: shittpd

${BIN}: ${OBJECTS}
	${COMPILER} -o ${BIN} ${OBJECTS}

%.o: %.c
	${COMPILER} -c $^

dependencies: ${CSOURCE}
	${COMPILER} -MM ${CSOURCE}

clean:
	- rm ${OBJECTS}
	- rm dependencies

ifeq "${INCLUDEDEPS}" ""
include dependencies
endif
