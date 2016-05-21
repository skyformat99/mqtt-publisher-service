#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "MQTTClient.h"
#include "provenancelib.h"
#include "simplog.h"

#define	LOG_FILE "/tmp/audit.log"
#define gettid() syscall(SYS_gettid)

#define ADDRESS         "tcp://m12.cloudmqtt.com:17065"
#define CLIENTID        "ExampleClientPub"
#define USERNAME        "camflow"
#define PASSWORD        "test"
#define TOPIC           "camflow/test"
#define PAYLOAD         "Ok and now?"
#define QOS             1
#define TIMEOUT         10000L

MQTTClient client;

void mqqt_publish(char* topic, char* payload, int qos){
  int rc;
  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  MQTTClient_deliveryToken token;
  pubmsg.payload = payload;
  pubmsg.payloadlen = strlen(payload);
  pubmsg.qos = qos;
  pubmsg.retained = 0;
  MQTTClient_publishMessage(client, topic, &pubmsg, &token);
  rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
  simplog.writeLog(SIMPLOG_INFO, "%d %s : %s", rc, topic, payload);
}

void _init_logs( void ){
  simplog.setLogFile(LOG_FILE);
  simplog.setLineWrap(false);
  simplog.setLogSilentMode(true);
  simplog.setLogDebugLevel(SIMPLOG_VERBOSE);
}

static __thread char buffer[10192]; // check the size

void init( void ){
  pid_t tid = gettid();
  simplog.writeLog(SIMPLOG_INFO, "audit writer thread, tid:%ld", tid);
}

void log_str(struct str_struct* data){
  simplog.writeLog(SIMPLOG_INFO, str_msg_to_json(buffer, data));
}

void log_link(struct link_struct* link){
  simplog.writeLog(SIMPLOG_INFO, link_to_json(buffer, link));
}

void log_unlink(struct unlink_struct* unlink){
  simplog.writeLog(SIMPLOG_INFO, unlink_to_json(buffer, unlink));
}

void log_edge(struct edge_struct* edge){
  mqqt_publish("camflow/edges", edge_to_json(buffer, edge), QOS);
}

void log_task(struct task_prov_struct* task){
  mqqt_publish("camflow/activities", task_to_json(buffer, task), QOS);
}

void log_inode(struct inode_prov_struct* inode){
  mqqt_publish("camflow/entities", inode_to_json(buffer, inode), QOS);
}

void log_disc(struct disc_node_struct* node){
  // TODO allow disclosing process to set type
  mqqt_publish("camflow/entities", disc_to_json(buffer, node), QOS);
}

void log_msg(struct msg_msg_struct* msg){
  mqqt_publish("camflow/entities", msg_to_json(buffer, msg), QOS);
}

void log_shm(struct shm_struct* shm){
  mqqt_publish("camflow/entities", shm_to_json(buffer, shm), QOS);
}


void log_sock(struct sock_struct* sock){
  mqqt_publish("camflow/entities", sock_to_json(buffer, sock), QOS);
}

void log_address(struct address_struct* address){
  simplog.writeLog(SIMPLOG_INFO, addr_to_json(buffer, address));
}

void log_file_name(struct file_name_struct* f_name){
  simplog.writeLog(SIMPLOG_INFO, pathname_to_json(buffer, f_name));
}

void log_ifc(struct ifc_context_struct* ifc){
  simplog.writeLog(SIMPLOG_INFO, ifc_to_json(buffer, ifc));
}

struct provenance_ops ops = {
  .init=init,
  .log_edge=log_edge,
  .log_task=log_task,
  .log_inode=log_inode,
  .log_str=log_str,
  .log_link=log_link,
  .log_unlink=log_unlink,
  .log_disc=log_disc,
  .log_msg=log_msg,
  .log_shm=log_shm,
  .log_sock=log_sock,
  .log_address=log_address,
  .log_file_name=log_file_name,
  .log_ifc=log_ifc
};

int main(int argc, char* argv[])
{
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    _init_logs();
    simplog.writeLog(SIMPLOG_INFO, "audit process pid: %ld", getpid());

    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        simplog.writeLog(SIMPLOG_ERROR, "Failed to connect %d", rc);
        exit(-1);
    }
    mqqt_publish(TOPIC, PAYLOAD, QOS);

    rc = provenance_register(&ops);
    if(rc){
      simplog.writeLog(SIMPLOG_ERROR, "Failed registering audit operation.");
      exit(rc);
    }
    sleep(2);
    while(1) sleep(60);

    // never reached
    provenance_stop();
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return 0;
}
