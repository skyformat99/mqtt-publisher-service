INCLUDES = -I../build/camflow-provenance-lib/include/provenancelib
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = ../build/camflow-provenance-lib/src/provenancelib.a /usr/local/lib/libpaho-mqtt3c.so
.SUFFIXES: .c

all:
	$(CCC) $(INCLUDES) publisher.c $(LIB) -o publisher.o $(CCFLAGS)

debug:
	$(CCC) $(INCLUDES) publisher.c $(LIB) -o publisher.o $(CCFLAGS) -g

install:

clean:
	rm -f config.o
