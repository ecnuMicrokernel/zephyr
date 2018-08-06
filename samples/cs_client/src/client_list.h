



/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.

 * SPDX-License-Identifier: Apache-2.0
 */

#include "common.h"
#include <zephyr.h>
#include <misc/printk.h>
#include <misc/slist.h>
#include <stdlib.h>
//static struct clients client;
//static sys_slist_t client_list;

 /*static  struct server* find_client_by_ip(int ip){
 	 struct server *tmp_server;
 	 struct server *result=NULL;
	 SYS_SLIST_FOR_EACH_CONTAINER( &server_list, tmp_server, node)
	    {
	    	if(port==tmp_server->port){
	    		result=tmp_server;
	    		printk("find_server_by_port:%d\n",result);
	    		break;
	    	}
	    }
	    return result;
 }*/

 static inline void add_clients(struct server *server,struct client *client,int IP){

 		//struct clients new_clients[2];
	 	//new_clients[0].ip=12;
	 	//new_clients[1].ip=15;
 		client->IP=IP;
	 	sys_slist_append(&(server->client_list),&(client->node));
	 	//sys_slist_append(&client_list,&new_clients[1].node);
	 	printk("add_clients IP:%d\n",IP);
 
 }

 //初始化client_list
 static  void init_client_list(struct server *server){
  	 sys_slist_init(&(server->client_list));
  	  printk("init_client_list\n");
 }
 
 /*static  void remove_server(int port){
 	 struct server *tmp_server;
 	 struct server *result=find_server_by_port(port);
 	 sys_slist_find_and_remove(&server_list, &(result->node));
	  return result;
 }*/

static  struct client* find_client_by_ip(struct server *server,int IP){
 	 struct client *tmp_client;
 	 struct client *result=NULL;
	 SYS_SLIST_FOR_EACH_CONTAINER(&(server->client_list), tmp_client, node)
	    {
	    	
	    	if(IP==tmp_client->IP){
	    		result=tmp_client;
	    		printk("找到的ip为:%d\n",tmp_client->IP);
	    		break;
	    	}
	    	
	    	//printk("找到的ip为:%d\n",tmp_client->ip);
	    }
 	 
 	 
	  return result;
 }


 static void remove_client(struct server *server,struct client *client){

 		//struct clients new_client;
	 	//struct client *result=find_client_by_ip(server,IP);
	 	sys_slist_find_and_remove(&(server->client_list), &(client->node));
	 	printk("删除的ip为:%d\n",client->IP);
	 	//return result;
 
 }

 static  void print_client_list(sys_dlist_t *list){
 	printk("print_client_list:\n");
 	struct client *tmp_clients;

  	SYS_SLIST_FOR_EACH_CONTAINER(list, tmp_clients, node)
	{
	    printk("ip:%d\n",tmp_clients->IP);
	}
 }



// void main(void)
// {   
//    	init_client_list();
//    //	int num[3];
//    	//num[0] = 12;
//    //num[1] = 15;
//    	for(int i=0; i<1; i++){
//    		struct clients new_clients;
// 	 	new_clients.ip=num[i];
// 	 	sys_slist_append(&client_list,&new_clients.node);
// 	 	printk("add_clients ip:%d\n",num[i]);
//    	}

//    	add_clients(12);
//     //add_clients(15);
//    //printk("要删除的客户端IP为：");
//    	//remove_client(12);
// 	print_client_list();
// }
