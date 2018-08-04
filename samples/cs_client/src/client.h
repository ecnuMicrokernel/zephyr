#include "common.h"
#include <misc/printk.h>
#include <misc/slist.h>
#include <zephyr.h>
struct client{
	struct callback cb;
	int server_port;
	int IP;
	struct k_thread threads[1];			//listen线程和recv线程
	char thread_stacks[1][STACKSIZE];	//两个线程的stack
	sys_snode_t node;
};

/*
 * 监听连接线程，在server_init处启动
 */
void client_threads_listen(){
	while (1) {
		printk("client's threads_listen\n");
		k_sleep(300);
	}
}
/*
 * API : initial a client, set client's server_port,IP and all callback
  *
  */
static int client_init(	struct  client* client ,
			int client_IP,
			connect_cb_t connect_cb,
			recv_cb_t recv_cb,
			send_cb_t send_cb,
			close_cb_t close_cb){
	/*if(!server_port||server_port<=0){
		printk("INVALID PORT\n" );
		return FAIL;
	}*/
	//printk("VALID PORT\n" );
	client->server_port=-1;
	client->IP=client_IP;
	set_cb(&client->cb,connect_cb,recv_cb,send_cb,close_cb);

	//启动接收消息线程
	k_thread_create(&(client->threads[0]), &(client->thread_stacks[0][0]), STACKSIZE,
		client_threads_listen, 0, 0, 0,
		K_PRIO_COOP(4), 0, 0);
	return SUCCESS;
}


       
static int client_api_connnect(struct  client* client,struct  server* server)
{
	  if(client->server_port==-1)
	  {
	      printk( "client connect request\n" ); 
	      
	      struct data_item_t msg;
	
	      build_MSG(&msg,MSG_CONNECT,"request connect",NULL,client);
	      /* send msg to server */
	      while (k_msgq_put(&(server->lisen_msgq), &msg, K_NO_WAIT)!= 0) {
	            /* message queue is full: purge old data & try again */
	                k_msgq_purge(&(server->lisen_msgq));
	            }/* data item was successfully added to message queue */

	    /*wait server msg to confirm connect*/  
		   
		   struct data_item_t data;
		   if(k_msgq_get(&(server->lisen_msgq), &data, K_FOREVER)==0)
		   {
		       if(data.flag==MSG_CONNECT)
		       {
		       	  printk( "MSG_CONNECT:%s\n", data.data );
		          client->server_port=data.server->port;
		          printk( "%d\n", client->server_port );
		          client->cb.connect_cb;
		          return SUCCESS;
		       }
		       else{
		          return FAIL;
		       }
		    }
	  
	   }
	   else{
	      client->cb.connect_cb;
	      return FAIL;//already connected
	    }

}


