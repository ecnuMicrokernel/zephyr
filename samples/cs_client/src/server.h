#include "common.h"
#include "msg.h"
#include <misc/printk.h>


struct server{
	struct callback cb;					//callback函数集合
	int port;							//server的ID
	struct k_thread threads[2];			//listen线程和recv线程
	char thread_stacks[2][STACKSIZE];	//两个线程的stack
	char __aligned(4) 
	msgq_buf[2][10 * DATA_ITEM_T_SIZE]; //定义了listen和recv对应消息队列的buffer
	struct k_msgq recv_msgq;			//定义了recv对应消息队列
	struct k_msgq listen_msgq;
};

/*
 * 监听连接线程，在server_init处启动
 */
void server_threads_listen(struct  server *server ){
	while (1) {
		printk("server's threads_listen\n");
        struct data_item_t msg; 
		k_msgq_get( &(server->listen_msgq), &msg, K_FOREVER ); 
		printk( "lisen ");
		if(msg.flag==MSG_DISCONN){
			printk( "MSG_DISCONN:%s\n", msg.data );
			if (server->cb.connect_cb){
				server->cb.connect_cb(NULL,NULL,NULL);
			}
		}
		if(msg.flag==MSG_CONNECT){
			printk( "MSG_CONNECT:%s\n", msg.data );

			struct data_item_t msg1; 
            build_MSG( &msg1, MSG_CONNECT, "connect success", server, NULL );    
      
            while (k_msgq_put(&server->listen_msgq, &msg1, K_NO_WAIT) != 0) {
               k_msgq_purge(&server->listen_msgq);
             }

			if (server->cb.connect_cb){
				server->cb.connect_cb(NULL,NULL,NULL);
			}

		}
		k_sleep(1);
	}
}


/*
 * 服务器接收消息，监听对应的server->recv_msgq消息队列
 * 如果收到的消息是MSG_DATA就调用recv回调函数，否则输出错误信息NODEFINE
 */
void server_threads_recv( struct  server *server  )
{
	printk( "enter server_threads_recv()\n");
	//while ( 1 )
	//{
		struct data_item_t msg; 
		k_msgq_get( &(server->recv_msgq), &msg, K_FOREVER ); 
		printk( "recv ");
		if(msg.flag==MSG_DATA){
			printk( "MSG_DATA:%s\n", msg.data );
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

	//}
}

/*
 * API : initial a server, set server's port and all callback
 *
 */
static int server_init(	struct  server* server ,
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
