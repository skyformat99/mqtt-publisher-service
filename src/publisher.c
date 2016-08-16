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
#include "ini.h"

#define	LOG_PATH "/tmp/audit.log"
#define CONFIG_PATH "/etc/camflow-mqtt.ini"
#define gettid() syscall(SYS_gettid)
#define TIMEOUT         10000L

MQTTClient client;

typedef struct{
  char address[PATH_MAX]; // assuming we could use unix socket
  char client_id[1024];
  char username[1024];
  char password[1024];
  int qos;
} configuration;

configuration config;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if(MATCH("mqtt", "qos")) {
      pconfig->qos = atoi(value);
      simplog.writeLog(SIMPLOG_INFO, "MQTT QOS %d", pconfig->qos);
    } else if (MATCH("mqtt", "address")) {
      strncpy(pconfig->address, value, PATH_MAX);
      simplog.writeLog(SIMPLOG_INFO, "MQTT address %s", pconfig->address);
    } else if(MATCH("mqtt", "client_id")) {
      strncpy(pconfig->client_id, value, 1024);
      simplog.writeLog(SIMPLOG_INFO, "MQTT client id %s", pconfig->client_id);
    }else if(MATCH("mqtt", "username")){
      strncpy(pconfig->username, value, 1024);
      simplog.writeLog(SIMPLOG_INFO, "MQTT username %s", pconfig->username);
    }else if(MATCH("mqtt", "password")){
      strncpy(pconfig->password, value, 1024);
      simplog.writeLog(SIMPLOG_INFO, "MQTT password %s", pconfig->password);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

void mqtt_connect(void){
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  int rc;
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  conn_opts.username = config.username;
  conn_opts.password = config.password;

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
  simplog.setLogFile(LOG_PATH);
  simplog.setLineWrap(false);
  simplog.setLogSilentMode(true);
  simplog.setLogDebugLevel(SIMPLOG_VERBOSE);
}

void init( void ){
  pid_t tid = gettid();
  simplog.writeLog(SIMPLOG_INFO, "audit writer thread, tid:%ld", tid);
}

void log_str(struct str_struct* data){
  append_message(str_msg_to_json(data));
}

void log_relation(struct relation_struct* relation){
  switch(relation->type){
    case RL_NAMED:
    case RL_OPEN:
    case RL_READ:
    case RL_EXEC:
    case RL_SEARCH:
      append_used( used_to_json(relation) );
      break;
    case RL_CREATE:
      append_generated( generated_to_json(relation) );
      break;
    case RL_FORK:
    case RL_VERSION_PROCESS:
      append_informed( informed_to_json(relation) );
      break;
    case RL_WRITE:
    case RL_VERSION:
      append_derived( derived_to_json(relation) );
      break;
    default:
      append_relation( relation_to_json(relation) );
      break;
  }
}

void log_task(struct task_prov_struct* task){
  append_activity(task_to_json(task));
}

void log_inode(struct inode_prov_struct* inode){
  append_entity(inode_to_json(inode));
}

void log_disc(struct disc_node_struct* node){
  switch(node->identifier.node_id.type){
    case MSG_DISC_ACTIVITY:
      append_activity(disc_to_json(node));
      break;
    case MSG_DISC_AGENT:
      append_agent(disc_to_json(node));
      break;
    case MSG_DISC_ENTITY:
    case MSG_DISC_NODE:
    default:
      append_entity(disc_to_json(node));
      break;
  }
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
  //append_entity(ifc_to_json(ifc));
}

struct provenance_ops ops = {
  .init=init,
  .log_relation=log_relation,
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
  if(strlen(json)>100){
    mqqt_publish("camflow", json, config.qos);
  }
}

int main(int argc, char* argv[])
{
    int rc;

    _init_logs();
    simplog.writeLog(SIMPLOG_INFO, "MQTT Provenance service");
    simplog.writeLog(SIMPLOG_INFO, "Main process pid: %ld", getpid());

    memset(&config, 0, sizeof(configuration));

    if (ini_parse(CONFIG_PATH, handler, &config) < 0) {
        simplog.writeLog(SIMPLOG_ERROR, "Can't load '%s'", CONFIG_PATH);
        exit(-1);
    }

    MQTTClient_create(&client, config.address, config.client_id,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);

    rc = provenance_register(&ops);
    if(rc){
      simplog.writeLog(SIMPLOG_ERROR, "Failed registering audit operation.");
      exit(rc);
    }
    set_ProvJSON_callback(print_json);
    while(1){
      sleep(10);
      flush_json();
    }

    // never reached
    provenance_stop();
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return 0;
}
