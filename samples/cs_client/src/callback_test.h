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


/**
 *
 * 测试接收消息
 */
void send_msg_test( struct  server *server )
{
      printk( "enter server_threads_listen()\n" ); 
      /* send data to consumers */ 
      struct data_item_t msg; 
      build_MSG( &msg, MSG_DATA, "hello world!", 8080, 1080 );    
      k_msgq_put( &server->recv_msgq, &msg, K_NO_WAIT );
      build_MSG( &msg, MSG_CONNECT, "hello world!", 8080, 1080 );    
      k_msgq_put( &server->recv_msgq, &msg, K_NO_WAIT );
      
}
//必须全局变量，否则线程创建会出问题
struct server test_server;
struct client test_client;
static void callback_test(){

   	server_init(&test_server,80,connect_cb,recv_cb,NULL,NULL);
      send_msg_test(&test_server);
   	/*test_server.cb.recv_cb(NULL,NULL,NULL);
   	test_server.cb.connect_cb(NULL,NULL,NULL);*/

   	//client_init(&test_client,80,connect_cb,recv_cb,NULL,NULL,100);
   	//printk("client_init\n");
   	//test_client.cb.recv_cb(NULL,NULL,NULL);
   	//test_client.cb.connect_cb(NULL,NULL,NULL);
   	//testserver.cb.send_cb(NULL,NULL,NULL);
}