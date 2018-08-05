#include "common.h"



/*
 * 监听连接线程，在server_init处启动
 */
void client_threads_listen(){
	while (1) {
		printk("client's threads_listen\n");
		return;
		k_sleep(300);
	}
}
/*
 * API : initial a client, set client's server_port,IP and all callback
  *
  */
int client_init(	struct  client* client ,
			int client_IP,
			connect_cb_t connect_cb,
			recv_cb_t recv_cb,
			send_cb_t send_cb,
			close_cb_t close_cb){
	printk("enter client_init\n");  

	client->server=NULL;

	client->IP=client_IP;
	set_cb(&client->cb,connect_cb,recv_cb,send_cb,close_cb);

	//接收线程队列信息
	k_msgq_init(&(client->recv_msgq), client->msgq_buf[0], DATA_ITEM_T_SIZE, 10); 
	k_msgq_init(&(client->listen_msgq), client->msgq_buf[1], DATA_ITEM_T_SIZE, 10); 

	//启动接收消息线程
	k_thread_create(&(client->threads[0]), &(client->thread_stacks[0][0]), STACKSIZE,
		client_threads_listen, 0, 0, 0,
		K_PRIO_COOP(4), 0, 0);
	return SUCCESS;
}


       
int client_connect(int port,struct  client* client)
{
	//printk("enter client_connnect :%d\n",client->server); 
	struct server* server=first_server;
	if(!client->server)
	  {
	      
	      struct data_item_t msg;
	
	      build_MSG(&msg,MSG_CONNECT,"request connect",first_server,client);
	      /* send msg to server */
	      while (k_msgq_put(&(server->listen_msgq), &msg, K_NO_WAIT)!= 0) {
	            /* message queue is full: purge old data & try again */
	                k_msgq_purge(&(server->listen_msgq));
	            }/* data item was successfully added to message queue */

	    /*wait server msg to confirm connect*/  
		   
		   if(k_msgq_get(&(client->listen_msgq), &msg, K_FOREVER)==0)
		   {
		   		//确认连接
		       if(msg.flag==MSG_CONNECT&&msg.client==client&&msg.server==server)
		       {
		       	  printk( "\tCLIENT CONNECT INFO IP:%d\n",msg.server->port );
		          client->server=server;
		          client->cb.connect_cb;
		          return SUCCESS;
		       }
		       else{
		          return FAIL;
		       }
		    }
	  
	   }
	   else{
	   	  printk( "connect err\n" ); 

	      client->cb.connect_cb;
	      return FAIL;//already connected
	    }
	return SUCCESS;
}


void client_send(struct  client* client)
{
      struct data_item_t msg;
	
	  build_MSG(&msg,MSG_DATA,"hello world",client->server,client);
	 
	  while (k_msgq_put(&(client->server->recv_msgq), &msg, K_NO_WAIT)!= 0) {
	        
	            k_msgq_purge(&(client->server->recv_msgq));
	        }

	  client->cb.send_cb;

}


void client_disconn(struct  client* client)
{
      struct data_item_t msg;
	
	  build_MSG(&msg,MSG_DISCONN,"request disconnect",client->server,client);
	 
	  while (k_msgq_put(&(client->server->listen_msgq), &msg, K_NO_WAIT)!= 0) {
	        
	            k_msgq_purge(&(client->server->listen_msgq));
	        }

	 deal_disconn( client);


}


void deal_disconn(struct  client* client)
{
	 client->server=NULL;        
	 client->cb.close_cb;
}