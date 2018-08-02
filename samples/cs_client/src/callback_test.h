// callback test
#include <misc/printk.h>
#include "server.h"
#include "client.h"
static void connect_cb(	int ctx,
			int status,
			void *user_data){
	printk("connect_cb_t\n");
}
static void  recv_cb(	int ctx,
			int status,
			void *user_data){
	printk("recv_cb\n");
}

//必须全局变量，否则线程创建会出问题
struct server test_server;
struct client test_client;
static void callback_test(){

   	server_init(&test_server,80,connect_cb,recv_cb,NULL,NULL);
   	printk("server_init\n");
   	test_server.cb.recv_cb(NULL,NULL,NULL);
   	test_server.cb.connect_cb(NULL,NULL,NULL);

   	client_init(&test_client,80,connect_cb,recv_cb,NULL,NULL,100);
   	printk("client_init\n");
   	test_client.cb.recv_cb(NULL,NULL,NULL);
   	test_client.cb.connect_cb(NULL,NULL,NULL);
   	//testserver.cb.send_cb(NULL,NULL,NULL);
}