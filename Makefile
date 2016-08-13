all:
	cd ./src && $(MAKE) all

prepare:
	mkdir -p build
	cd ./build && git clone https://github.com/benhoyt/inih.git
	cd ./build/inih/extra && $(MAKE) -f Makefile.static default
	cd ./build && git clone https://github.com/ntpeters/SimpleLogger.git
	cd ./build/SimpleLogger && $(MAKE) all
	cd ./build && git clone https://github.com/eclipse/paho.mqtt.c.git
	cd ./build/paho.mqtt.c && git checkout tags/v1.0.3
	cd ./build/paho.mqtt.c && sudo $(MAKE) install

install:
	cd ./src && sudo $(MAKE) install
	sudo cp --force ./camflow-mqtt.ini /etc/camflow-mqtt.ini

restart:
	cd ./src && sudo $(MAKE) restart

clean:
	cd ./src && $(MAKE) clean
