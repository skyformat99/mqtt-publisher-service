all:
	cd ./build/camflow-provenance-lib && $(MAKE) all
	cd ./src && $(MAKE) all

prepare:
	mkdir -p build
	cd ./build && git clone https://github.com/CamFlow/camflow-provenance-lib.git
	cd ./build/camflow-provenance-lib && $(MAKE) prepare
	cd ./build && git clone https://github.com/eclipse/paho.mqtt.c.git
	cd ./build/paho.mqtt.c && git checkout tags/v1.0.3
	cd ./build/paho.mqtt.c && sudo $(MAKE) install
