// callback test
#include <misc/printk.h>
#include "common.h"
static void connect_cb(	int ctx,
			int status,
			void *user_data){
	printk("enter connect_cb_t\n");
}
static void  recv_cb(	int ctx,
			int status,
			void *user_data){
	printk("enter recv_cb\n");
}


//必须全局变量，否则线程创建会出问题
static struct server test_server;
static struct client test_client1;
static struct client test_client2;
static struct client test_client3;


// 测试client_send;

static void init_CS(){
    server_init(&test_server,80,connect_cb,recv_cb,NULL,NULL);
    client_init(&test_client1,100,connect_cb,recv_cb,NULL,NULL);
    client_init(&test_client2,101,connect_cb,recv_cb,NULL,NULL);
    client_init(&test_client3,102,connect_cb,recv_cb,NULL,NULL);
    k_sleep(300);

}

static void connect_server(){
  client_connect(80,&test_client1); 
  client_connect(80,&test_client2); 
  client_connect(80,&test_client3); 
  k_sleep(300);
}

static void release_CS(){
    api_client_release(&test_client1);
    api_client_release(&test_client2);
    api_client_release(&test_client3);
    api_server_release(&test_server);
    k_sleep(300);

}
static void send_client2server(){
  client_send(&test_client1);
  client_send(&test_client2);
  client_send(&test_client3);
  k_sleep(300);
}
static void disconn_client2server(){
  client_disconn(&test_client1);
}  

//main 入口
static void entry_test(){

  init_CS();
  connect_server();
  send_client2server();
  disconn_client2server();
  release_CS();
  printk("end\n");  

}

