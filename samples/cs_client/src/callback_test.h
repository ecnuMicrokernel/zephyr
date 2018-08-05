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





static void callback_test(){

   	server_init(&test_server,80,connect_cb,recv_cb,NULL,NULL);
    client_init(&test_client1,100,connect_cb,recv_cb,NULL,NULL);
    client_init(&test_client2,101,connect_cb,recv_cb,NULL,NULL);
    client_init(&test_client3,102,connect_cb,recv_cb,NULL,NULL);
    //send_msg_test(&test_server);
    k_sleep(1000);

    client_connect(80,&test_client1); 
    client_connect(80,&test_client2); 
    client_connect(80,&test_client3); 
    
    printk("end\n");  
      
   	/*test_server.cb.recv_cb(NULL,NULL,NULL);
   	test_server.cb.connect_cb(NULL,NULL,NULL);*/

   	//client_init(&test_client,80,connect_cb,recv_cb,NULL,NULL,100);
   	//printk("client_init\n");
   	//test_client.cb.recv_cb(NULL,NULL,NULL);
   	//test_client.cb.connect_cb(NULL,NULL,NULL);
   	//testserver.cb.send_cb(NULL,NULL,NULL);
}