INCLUDES = -I../build/camflow-provenance-lib/include
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = ../build/camflow-provenance-lib/src/provenancelib.a -lpthread

.SUFFIXES: .c

all: std file

std:
	$(CCC) $(INCLUDES) std.c $(LIB) -o std.o $(CCFLAGS)

file:
	$(CCC) $(INCLUDES) file.c $(LIB) -o file.o $(CCFLAGS)

clean:
	rm -f *.o
