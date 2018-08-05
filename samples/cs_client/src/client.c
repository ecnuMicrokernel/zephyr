#include "common.h"



/*
 * 监听连接线程，在server_init处启动
 */
void client_threads_listen(struct  client* client_ptr){
	while (1) {
		printk("client %d threads_listen\n",client_ptr->IP);
	
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
		client_threads_listen, client, 0, 0,
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



       
int api_client_release(struct  client* client_ptr)
{
	printk("enter api_client_release\n" ); 

	//如果client的server不为空，则证明client还有连接server，需要disconnect再释放	
	if(client_ptr->server)
	{
	  	//apt_client_disconn(client_ptr);
	} 
	 
	//结束client的接收消息线程
	k_thread_abort(&client_ptr->threads[0]);
	printk("Client %d release\n",client_ptr->IP); 

	return SUCCESS;
}