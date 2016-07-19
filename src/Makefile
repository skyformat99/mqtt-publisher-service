INCLUDES = -I../build/camflow-provenance-lib/include -I../build/camflow-provenance-lib/simplelogger
CCFLAGS = -g -O2 -fpic
CCC = gcc
LIB = ../build/camflow-provenance-lib/src/provenancelib.a /usr/local/lib/libpaho-mqtt3c.so -lpthread ../build/camflow-provenance-lib/simplelogger/simplog.a
.SUFFIXES: .c

all:
	$(CCC) $(INCLUDES) publisher.c $(LIB) -o publisher.o $(CCFLAGS)

debug:
	$(CCC) $(INCLUDES) publisher.c $(LIB) -o publisher.o $(CCFLAGS) -g

install:
	cp --force ./publisher.o /usr/bin/camflow-provenance
	mkdir -p /var/relay
	cp --force ./camflow-provenance.service /etc/systemd/system/camflow-provenance.service
	systemctl enable camflow-provenance.service

restart:
	systemctl restart camflow-provenance.service

clean:
	rm -f config.o
