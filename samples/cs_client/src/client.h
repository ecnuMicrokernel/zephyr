#include "common.h"
#include <misc/printk.h>
struct client{
	struct callback cb;
	int server_port;
	int IP;
	struct k_thread threads[1];			//listen线程和recv线程
	char thread_stacks[1][STACKSIZE];	//两个线程的stack
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
			int server_port,
			connect_cb_t connect_cb,
			recv_cb_t recv_cb,
			send_cb_t send_cb,
			close_cb_t close_cb,
			int client_IP ){
	if(!server_port||server_port<=0){
		printk("INVALID PORT\n" );
		return FAIL;
	}
	printk("VALID PORT\n" );
	client->server_port=server_port;
	client->IP=client_IP;
	set_cb(&client->cb,connect_cb,recv_cb,send_cb,close_cb);

	//启动接收消息线程
	k_thread_create(&(client->threads[0]), &(client->thread_stacks[0][0]), STACKSIZE,
		client_threads_listen, 0, 0, 0,
		K_PRIO_COOP(4), 0, 0);
	return SUCCESS;
}
