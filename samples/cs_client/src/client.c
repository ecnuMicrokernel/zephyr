#include "common.h"



/*
 * 接受消息
 */
void client_threads_recv(struct  client* client_ptr){
	while (1) {
		printk("client %d threads_recv\n",client_ptr->IP);

		struct data_item_t msg; 
		k_msgq_get( &(client_ptr->recv_msgq), &msg, K_FOREVER ); 
		printk( "recv ");
		if(msg.flag==MSG_DATA){
			printk( "MSG_DATA client%d:%s\n", msg.client->IP,msg.data );
			if (client_ptr->cb.recv_cb){
				client_ptr->cb.recv_cb(NULL,NULL,NULL);
			}
		}

		if(msg.flag==MSG_DISCONN){
			printk( "MSG_DISCONN client%d:%s\n", msg.client->IP,msg.data );
			client_ptr->server=NULL;
			if (client_ptr->cb.recv_cb){
				client_ptr->cb.recv_cb(NULL,NULL,NULL);
			}
		}
        //确认连接
       if(msg.flag==MSG_CONNECT&&msg.client==client_ptr)
       {
       	  printk( "\tCLIENT CONNECT INFO IP:%d\n",msg.server->port );
          client_ptr->server=msg.server;
          client_ptr->cb.connect_cb;
          return SUCCESS;
       }
       else{
          return FAIL;
       }
		   		
		    
	
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
	
	//启动接收消息线程
	k_thread_create(&(client->threads[0]), &(client->thread_stacks[0][0]), STACKSIZE,
		client_threads_recv, client, 0, 0,
		K_PRIO_COOP(4), 0, 0);
	return SUCCESS;
}


       
int api_client_connect(int port,struct  client* client)
{
	printk("enter client_connnect :%d\n",client->server); 
	struct server* server=first_server;

	if(1)
	  {
	      
	      struct data_item_t msg;
	
	      build_MSG(&msg,MSG_CONNECT,"request connect",first_server,client);
	      /* send msg to server */
	      while (k_msgq_put(&(server->listen_msgq), &msg, K_NO_WAIT)!= 0) {
	            /* message queue is full: purge old data & try again */
	                k_msgq_purge(&(server->listen_msgq));
	            }/* data item was successfully added to message queue */

	    /*wait server msg to confirm connect*/  
		   
		   
	  
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
	  	api_client_disconn(client_ptr);
	} 
	 
	//结束client的接收消息线程
	k_thread_abort(&client_ptr->threads[0]);
	printk("Client %d release\n",client_ptr->IP); 

	return SUCCESS;
}

void api_client_send(char *data,struct  client* client)
{
      struct data_item_t msg;
	
	  build_MSG(&msg,MSG_DATA,data,client->server,client);
	 
	  while (k_msgq_put(&(client->server->recv_msgq), &msg, K_NO_WAIT)!= 0) {
	        
	            k_msgq_purge(&(client->server->recv_msgq));
	        }

	  client->cb.send_cb;

}


void api_client_disconn(struct  client* client)
{
      struct data_item_t msg;
	
	  build_MSG(&msg,MSG_DISCONN,"request disconnect",client->server,client);
	 
	  while (k_msgq_put(&(client->server->listen_msgq), &msg, K_NO_WAIT)!= 0) {
	        
	            k_msgq_purge(&(client->server->listen_msgq));
	        }

	 api_deal_disconn( client);


}


void api_deal_disconn(struct  client* client)
{
	 client->server=NULL;        
	 client->cb.close_cb;
}