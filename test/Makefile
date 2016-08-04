INCLUDES = -I../build/camflow-provenance-lib/include
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = ../build/camflow-provenance-lib/src/provenancelib.a -lpthread

.SUFFIXES: .c

all: std file exec

std:
	$(CCC) $(INCLUDES) std.c $(LIB) -o std.o $(CCFLAGS)

file:
	$(CCC) $(INCLUDES) file.c $(LIB) -o file.o $(CCFLAGS)

exec:
	$(CCC) $(INCLUDES) exec.c $(LIB) -o exec.o $(CCFLAGS)

exec:
	$(CCC) $(INCLUDES) fork.c $(LIB) -o fork.o $(CCFLAGS)

clean:
	rm -f *.o
