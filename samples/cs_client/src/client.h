#include "common.h"
#include <misc/printk.h>
struct client{
	struct callback cb;
	int server_port;
	int IP;
};

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
	return SUCCESS;
}
