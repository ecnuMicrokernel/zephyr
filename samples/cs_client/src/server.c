#include "common.h"


void PrintList(sys_dlist_t *list) //依次打印所有节点
{
    struct client *container;
    printk("print list node:\n");
    //注意，下面的参数node是container_node结构体中的字段名
    SYS_DLIST_FOR_EACH_CONTAINER(list, container, node)
    {
        printk("node%d   ", container->IP);
    }
    printk("\n\n");
}


/*
 * 监听连接线程，在server_init处启动
 */
void server_threads_listen(struct  server *server ){
  printk("server's threads_listen\n");
  sys_dlist_init(&server->client_list);
   while (1) {
   		struct data_item_t msg; 
    	k_msgq_get( &(server->listen_msgq), &msg, K_FOREVER );
    	if(msg.flag==MSG_CONNECT){
      		struct client*  client_ptr=msg.client;
      		printk( "\tMSG_CONNECT IP:%d\n", client_ptr->IP );

      		sys_dlist_append(&server->client_list, &(client_ptr->node));
      		PrintList(&server->client_list);
      		build_MSG(&msg,MSG_CONNECT,"accept connect",server,client_ptr);
	     	 /* send msg to server */
	      	while (k_msgq_put(&(client_ptr->listen_msgq), &msg, K_NO_WAIT)!= 0) {
	            /* message queue is full: purge old data & try again */
	                k_msgq_purge(&(client_ptr->listen_msgq));
	            }/* data item was successfully added to message queue */

    	}
    	else if(msg.flag==MSG_DISCONN){
     		printk( "MSG_DISCONN client%d:%s\n",msg.client->IP, msg.data );
      		if (server->cb.close_cb)
        	server->cb.close_cb(NULL,NULL,NULL);
      
    	}
   }
}



/*
 * 服务器接收消息，监听对应的server->recv_msgq消息队列
 * 如果收到的消息是MSG_DATA就调用recv回调函数，否则输出错误信息NODEFINE
 */
void server_threads_recv( struct  server *server  )
{
	printk( "enter server_threads_recv()\n");
	while ( 1 )
	{
		struct data_item_t msg; 
		k_msgq_get( &(server->recv_msgq), &msg, K_FOREVER ); 
		printk( "recv ");
		if(msg.flag==MSG_DATA){
			printk( "MSG_DATA client%d:%s\n", msg.client->IP,msg.data );
			if (server->cb.recv_cb){
				server->cb.recv_cb(NULL,NULL,NULL);
			}
		}else{
			printk( "MSG_CONNECT:%s\n", msg.data );

			struct data_item_t msg1; 
            build_MSG( &msg1, MSG_CONNECT, "connect success", server, NULL );    
      
            while (k_msgq_put(&server->recv_msgq, &msg1, K_NO_WAIT) != 0) {
               k_msgq_purge(&server->recv_msgq);
             }

			if (server->cb.recv_cb){
				server->cb.recv_cb(NULL,NULL,NULL);
			}

		}

	}
}

/*
 * API : initial a server, set server's port and all callback
 *
 */
int server_init(	struct  server* server ,
			int port,
			connect_cb_t connect_cb,
			recv_cb_t recv_cb,
			send_cb_t send_cb,
			close_cb_t close_cb){
	printk("enter server_init\n");
	if(!port||port<=0){
		printk("INVALID PORT\n" );
		return FAIL;
	}
	printk("VALID PORT\n" );
	first_server=server;
	server->port=port;
	set_cb(&server->cb,connect_cb,recv_cb,send_cb,close_cb);

	k_msgq_init(&(server->recv_msgq), server->msgq_buf[0], DATA_ITEM_T_SIZE, 10); 
	k_msgq_init(&(server->listen_msgq), server->msgq_buf[1], DATA_ITEM_T_SIZE, 10); 
	//创建监听连接线程和接收消息线程
	k_thread_create(&(server->threads[0]), &(server->thread_stacks[0][0]), STACKSIZE,
			server_threads_listen, server, 0, 0,
			K_PRIO_COOP(4), 0, 0);
	k_thread_create(&( server->threads[1]), &( server->thread_stacks[1][0]), STACKSIZE,
			server_threads_recv, server, 0, 0,
			K_PRIO_COOP(4), 0, 0);

	return SUCCESS;
}

int api_server_release(struct  server* server_ptr)
{
	printk("enter api_server_release\n" ); 

	//如果server的client列表不为空的话，逐个断连
	struct client *client_ptr=sys_dlist_peek_head(&server_ptr->client_list);
	while(client_ptr)
	{
		//发送消息给client断连，把client移出client_list
	  	//server_disconn(client_ptr);
	  	printk("disconnect client %d\n",client_ptr->IP); 
	  	server2client_disconnect(server_ptr,client_ptr);
		client_ptr=sys_dlist_peek_head(&server_ptr->client_list);

	} 
	 
	//结束client的接收消息线程和监听连接线程
	k_thread_abort(&server_ptr->threads[0]);
	k_thread_abort(&server_ptr->threads[1]);
	printk("server %d release\n",server_ptr->port); 

	return SUCCESS;
}

//server端发送消息给client端断开连接，并调用deal_disconnect
int server2client_disconnect(struct  server* server_ptr,struct client *client_ptr){
	printk("enter api_server_disconnect\n");
   	printk("server %d| client %d\n",server_ptr->port,client_ptr->IP);
   	struct container_node *container;
   	int try=30;//尝试30次操作，如果都不成功则不管发送
   	struct data_item_t msg; 
    build_MSG( &msg, MSG_DISCONN, "server disconnect", server_ptr, client_ptr );    
   	while (k_msgq_put(&(client_ptr->recv_msgq), &msg, K_NO_WAIT)!= 0&&try-->0) {
	 	k_msgq_purge(&(client_ptr->recv_msgq));
	}
   deal_disconnect(server_ptr,client_ptr);
   return SUCCESS;
}

//如果client_IP的client在server->client_list里面，则返回该client的地址，否则返回NULL
struct client* client_in_list(struct  server* server_ptr,int client_IP){
	 //注意，下面的参数node是container_node结构体中的字段名
	//sys_dlist_t list=server_ptr->client_list;
	struct client *client_ptr=NULL;
    SYS_DLIST_FOR_EACH_CONTAINER(&server_ptr->client_list, client_ptr, node)
    {
    	if(client_ptr->IP==client_IP){
    		break;
    	}
    }
    return client_ptr;
}
//把client从server的client_list里面删除
void deal_disconnect(struct  server* server_ptr,struct client *client_ptr){
	printf("enter deal_disconnect server:%d client:%d\n",server_ptr->port,client_ptr->IP);
	sys_dlist_remove(&client_ptr->node);
}