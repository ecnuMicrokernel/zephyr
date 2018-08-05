#ifndef _HEADERNAME_H
#define _HEADERNAME_H

struct  server* first_server;

#include <misc/printk.h>
#include <misc/dlist.h>
#include <zephyr.h>

#ifndef SUCCESS
	#define SUCCESS                  0
#endif
#ifndef FAIL
	#define FAIL                              -1
#endif
#define STACKSIZE 1000


/**
 * @typedef server_recv_cb_t
 * @brief Network data receive callback.
 * @details The recv callback is called after a  data packet is received successed
 * @param ctx The context to use（no use).
 * @param status Value is set to 0 if some data or the connection is
 * at EOF, <0 if there was an error receiving data, in this case the
 * pkt parameter is set to NULL.
 * @param user_data The user data given in net_recv() call.
 */
typedef void (*recv_cb_t)(	int ctx,
				int status,
				void *user_data);
/**
 * @typedef net_context_connect_cb_t
 * @brief Connection callback.
 * @details The connect callback is called after a connection is being
 * established.
 * @param ctx The context to use（no use).
 * @param status Status of the connection establishment. This is 0
 * if the connection was established successfully, <0 if there was an
 * error.
 * @param user_data The user data given in server_connect() call.
 */
typedef void (*connect_cb_t)(	int ctx,
				int status,
				void *user_data);
/**
 * @typedef net_context_send_cb_t
 * @brief Network data send callback.
 * @details The send callback is called after a network data packet is sent.
 * @param ctx The context to use（no use).
 * @param status Value is set to 0 if all data was sent ok, <0 if
 * there was an error sending data. >0 amount of data that was
 * sent when not all data was sent ok.
 * @param user_data The user data given in net_send() call.
 */
typedef void (*send_cb_t)(	int ctx,
				int status,
				void *user_data);
/**
 * @typedef server_close_cb_t
 * @brief Close callback.
 * @details The close callback is called after a connection is being
 * shutdown.
 * @param ctx The context to use（no use).
 * @param status Error code for the closing.
 * @param user_data The user data given in init call.
 */
typedef void (*close_cb_t)(	int ctx,
				 int status,
				 void *user_data);

struct  callback {
	connect_cb_t connect_cb;
	recv_cb_t recv_cb;
	send_cb_t send_cb;
	close_cb_t close_cb;
};

// callback implement example
static void  defult_cb(	int ctx,
			int status,
			void *user_data){
	printk("recv_cb\n");
}

/*
 * API
 * TODO: set cb callback
 * 
 */
 static int set_cb(	struct  callback* cb,
		   connect_cb_t connect_cb,
		   recv_cb_t recv_cb,
		   send_cb_t send_cb,
		   close_cb_t close_cb){
	if (!cb) {
		return FAIL;
	}
	cb->connect_cb = connect_cb;
	cb->recv_cb = recv_cb;
	cb->send_cb = send_cb;
	cb->close_cb = close_cb;
	return SUCCESS;
}









#define MSG_CONNECT		 	1	//FLAG 建立连接
#define MSG_DISCONN			-1	//FLAG 断开连接
#define MSG_DATA			2	//FLAG 数据传输
#define DATA_MAX_LEN 		50	//消息长度限制
#define DATA_ITEM_T_SIZE 	sizeof(struct data_item_t)

struct data_item_t
{
	int 	flag;
	char*	data;
	struct server* 	server;
	struct client* 	client;
};

/*
 * API
 * TODO: build a data_item_t
 * 		 if set fail return FAIL else return SUCCESS	
 * 		 msg will be change here
 */
static int build_MSG(struct data_item_t* msg,int flag,
	char*data,struct server* server,struct client* client){
	if(sizeof(data)>DATA_MAX_LEN)
		return FAIL;

	msg->flag=flag;
	msg->data=data;
	msg->server=server;
	msg->client=client;
	return SUCCESS;
}

//client
struct client{
	sys_dnode_t node;
	struct callback cb;
	struct server *server;
	int IP;
	struct k_thread threads[1];			
	char thread_stacks[1][STACKSIZE];
	char __aligned(4) 
	msgq_buf[2][10 * DATA_ITEM_T_SIZE]; //定义了recv消息队列的buffer
	struct k_msgq recv_msgq;			//定义了recv消息队列	
	struct k_msgq listen_msgq;			//接受确认连接信息

};

void client_threads_listen();
int client_init(	struct  client* client ,
			int client_IP,
			connect_cb_t connect_cb,
			recv_cb_t recv_cb,
			send_cb_t send_cb,
			close_cb_t close_cb);
int client_connect(int port,struct  client* client);
int api_client_release(struct  client* client_ptr);


//SERVER
struct server{
	struct callback cb;					//callback函数集合
	int port;							//server的ID
	struct k_thread threads[2];			//listen线程和recv线程
	char thread_stacks[2][STACKSIZE];	//两个线程的stack
	char __aligned(4) 
	msgq_buf[2][10 * DATA_ITEM_T_SIZE]; //定义了listen和recv对应消息队列的buffer
	struct k_msgq recv_msgq;			//定义了recv对应消息队列
	struct k_msgq listen_msgq;
	sys_dlist_t client_list;			//存放连接的client指针
};
void PrintList(sys_dlist_t *list) ;
void server_threads_listen(struct  server *server );
void server_threads_recv( struct  server *server  );
int server_init(	struct  server* server ,
			int port,
			connect_cb_t connect_cb,
			recv_cb_t recv_cb,
			send_cb_t send_cb,
			close_cb_t close_cb);
int api_server_release(struct  server* server_ptr);
#endif




