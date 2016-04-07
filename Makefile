prepare:
	mkdir -p build
	cd ./build && git clone https://github.com/tfjmp/camflow-audit-lib.git
	cd ./build/camflow-audit-lib && $(MAKE) prepare
	cd ./build && git clone https://github.com/eclipse/paho.mqtt.c.git
	cd ./build/paho.mqtt.c && git checkout tags/v1.0.3
