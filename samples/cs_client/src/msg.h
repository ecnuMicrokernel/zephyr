#ifndef _HEADERNAME_H_MSG
#define _HEADERNAME_H_MSG

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
int build_MSG(struct data_item_t* msg,int flag,
	char*data,struct server* server,struct client* client){
	if(sizeof(data)>DATA_MAX_LEN)
		return FAIL;

	msg->flag=flag;
	msg->data=data;
	msg->server=server;
	msg->client=client;
	return SUCCESS;
}

#endif