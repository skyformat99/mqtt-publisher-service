# MQTT publisher service

## Install

We assume CamFlow has been installed on your Linux machine, please see instruction [here](https://github.com/CamFlow/camflow-install), or [here](https://github.com/CamFlow/vagrant) to setup a VM through vagrant.

``` SHELL
sudo dnf install mosquitto
git clone https://github.com/tfjmp/mqtt-publisher-service.git
cd mqtt-publisher-service
make prepare
make all
make install
```

## Configuration

The file `/etc/camflow-mqtt.ini` allows to modify the configuration of the MQTT publisher service. The service need to be restarted for a new configuration to be applied (through `systemctl restart camflow-provenance.service`).

``` INI
[mqtt]
address=tcp://m12.cloudmqtt.com:17065
client_id=ExampleClientPub
username=camflow
password=test
; message delivered: 0 at most once, 1 at least once, 2 exactly once
qos=1
```

## Checking logs

```
cat /tmp/audit.log # CamFlow service logs
cat /tmp/mosquitto.log # Mosquitto logs
```
