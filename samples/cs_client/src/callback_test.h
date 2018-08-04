// callback test
#include <misc/printk.h>
#include "server.h"
#include "client.h"
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
struct server test_server;
struct client test_client;


/*client request to connect*/
int client_connect(int port)
{
      if(test_server.port!=port){
          printk("port error!");  
          return FAIL; 
        }
        else{
           client_api_connnect(&test_client,&test_server);
           return SUCCESS;
        }
           
}


static void callback_test(){

   	server_init(&test_server,80,connect_cb,recv_cb,NULL,NULL);
      client_init(&test_client,100,connect_cb,recv_cb,NULL,NULL);
      //send_msg_test(&test_server);
      client_connect(80);  
      printk("end\n");  
      
   	/*test_server.cb.recv_cb(NULL,NULL,NULL);
   	test_server.cb.connect_cb(NULL,NULL,NULL);*/

   	//client_init(&test_client,80,connect_cb,recv_cb,NULL,NULL,100);
   	//printk("client_init\n");
   	//test_client.cb.recv_cb(NULL,NULL,NULL);
   	//test_client.cb.connect_cb(NULL,NULL,NULL);
   	//testserver.cb.send_cb(NULL,NULL,NULL);
}