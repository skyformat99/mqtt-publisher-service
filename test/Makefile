INCLUDES = -I../build/camflow-provenance-lib/include
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = ../build/camflow-provenance-lib/src/provenancelib.a -lpthread

.SUFFIXES: .c

all: std

std:
	$(CCC) $(INCLUDES) std.c $(LIB) -o std.o $(CCFLAGS)

clean:
	rm -f *.o
