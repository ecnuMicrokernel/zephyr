#ifndef _HEADERNAME_H
#define _HEADERNAME_H

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


#endif




