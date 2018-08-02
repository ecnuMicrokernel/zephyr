#include "common.h"
#include <misc/printk.h>


struct server{
	struct callback cb;					//callback函数集合
	int port;							//server的ID
	struct k_thread threads[2];			//listen线程和recv线程
	char thread_stacks[2][STACKSIZE];	//两个线程的stack
};

/*
 * 接收消息线程，在server_init处启动
 */
void server_threads_recv(){
	while (1) {
		printk("server's threads_recv\n");
		k_sleep(300);
	}
}

/*
 * 监听连接线程，在server_init处启动
 */
void server_threads_listen(){
	while (1) {
		printk("server's threads_listen\n");
		k_sleep(300);
	}
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
	if(!port||port<=0){
		printk("INVALID PORT\n" );
		return FAIL;
	}
	printk("VALID PORT\n" );
	server->port=port;
	set_cb(&server->cb,connect_cb,recv_cb,send_cb,close_cb);
	//创建监听连接线程和接收消息线程
	k_thread_create(&(server->threads[0]), &(server->thread_stacks[0][0]), STACKSIZE,
			server_threads_listen, 0, 0, 0,
			K_PRIO_COOP(4), 0, 0);
	k_thread_create(&( server->threads[1]), &( server->thread_stacks[1][0]), STACKSIZE,
			server_threads_recv, 0, 0, 0,
			K_PRIO_COOP(4), 0, 0);
	return SUCCESS;
}
