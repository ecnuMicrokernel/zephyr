#include "common.h"
#include <misc/printk.h>
struct server{
	struct callback cb;
	int port;
};

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
	return SUCCESS;
}
