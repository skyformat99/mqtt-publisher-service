#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "MQTTClient.h"
#include "provenancelib.h"
#include "provenancePovJSON.h"
#include "simplog.h"

#define	LOG_FILE "/tmp/audit.log"
#define gettid() syscall(SYS_gettid)

#define ADDRESS         "tcp://m12.cloudmqtt.com:17065"
#define CLIENTID        "ExampleClientPub"
#define USERNAME        "camflow"
#define PASSWORD        "test"
#define QOS             1
#define TIMEOUT         10000L

MQTTClient client;

void mqtt_connect(void){
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  int rc;
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  conn_opts.username = USERNAME;
  conn_opts.password = PASSWORD;

  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
  {
      simplog.writeLog(SIMPLOG_ERROR, "Failed to connect, return code %d\n", rc, rc);
      exit(-1);
  }
}

void mqqt_publish(char* topic, char* payload, int qos){
  int rc;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token;
  pubmsg.payload = payload;
  pubmsg.payloadlen = strlen(payload);
  pubmsg.qos = qos;
  pubmsg.retained = 0;
  do{
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    if(rc == MQTTCLIENT_DISCONNECTED)
      mqtt_connect();
  }while(rc == MQTTCLIENT_DISCONNECTED);
  simplog.writeLog(SIMPLOG_INFO, "%d %s : %s", rc, topic, payload);
}

void _init_logs( void ){
  simplog.setLogFile(LOG_FILE);
  simplog.setLineWrap(false);
  simplog.setLogSilentMode(true);
  simplog.setLogDebugLevel(SIMPLOG_VERBOSE);
}

void init( void ){
  pid_t tid = gettid();
  simplog.writeLog(SIMPLOG_INFO, "audit writer thread, tid:%ld", tid);
}

void log_str(struct str_struct* data){
  //append_entity(str_msg_to_json(data));
}

void log_edge(struct edge_struct* edge){
  append_edge(edge_to_json(edge));
}

void log_task(struct task_prov_struct* task){
  append_activity(task_to_json(task));
}

void log_inode(struct inode_prov_struct* inode){
  append_entity(inode_to_json(inode));
}

void log_disc(struct disc_node_struct* node){
  append_entity(disc_to_json(node));
}

void log_msg(struct msg_msg_struct* msg){
  append_entity(msg_to_json(msg));
}

void log_shm(struct shm_struct* shm){
  append_entity(shm_to_json(shm));
}


void log_sock(struct sock_struct* sock){
  append_entity(sock_to_json(sock));
}

void log_address(struct address_struct* address){
  append_entity(addr_to_json(address));
}

void log_file_name(struct file_name_struct* f_name){
  append_entity(pathname_to_json(f_name));
}

void log_ifc(struct ifc_context_struct* ifc){
  append_entity(ifc_to_json(ifc));
}

struct provenance_ops ops = {
  .init=init,
  .log_edge=log_edge,
  .log_task=log_task,
  .log_inode=log_inode,
  .log_str=log_str,
  .log_disc=log_disc,
  .log_msg=log_msg,
  .log_shm=log_shm,
  .log_sock=log_sock,
  .log_address=log_address,
  .log_file_name=log_file_name,
  .log_ifc=log_ifc
};

void print_json(char* json){
  sleep(1); // demo use free version we don't want to go over bandwith limit
  mqqt_publish("camflow", json, QOS);
}

int main(int argc, char* argv[])
{
    int rc;

    _init_logs();
    simplog.writeLog(SIMPLOG_INFO, "audit process pid: %ld", getpid());

    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);

    rc = provenance_register(&ops);
    if(rc){
      simplog.writeLog(SIMPLOG_ERROR, "Failed registering audit operation.");
      exit(rc);
    }
    set_ProvJSON_callback(print_json);
    while(1) sleep(60);

    // never reached
    provenance_stop();
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return 0;
}
