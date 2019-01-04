/**
****************************************************************
*  对事件队列结构进行操作的相关函数定义;
*  包括:
*     搜索search_list
*     添加insert_event
*     删除delete_event
****************************************************************
*/
#include <kernel.h>

/**
 *功能: 在相应的事件队列中查找对应目标进程的事件
 *输入参数:
 *    tK5_esb *esb : 提取查找信息的事件帧地址,该参数不能为空
 *    int list_type : 所需查找的事件队列类型,仅为0(代表等待发送队列),或2(代表等待接收队列)  
 *输出参数:
 *    NULL : 输入参数有误或查找不成功
 *    event->esb : 查找成功,返回查找到的对应事件
 */
tK5_esb * search_list(tK5_esb *esb ,int list_type){
    if (esb == NULL || (list_type!=0 && list_type!=1)){
         printk("K_ERR: input parm error\n");
         return NULL;
    }
  	struct k_thread *src_port = (struct k_thread *)esb->src_port;
  	struct k_thread *dst_port = (struct k_thread *)esb->dst_port;
  	EventLinkList list;
  	EventNode *event;
    switch(esb->primitive){
      	case K5_WAIT :
      	case K5_REPLY:
      	{   
      		if(list_type == 0) list = src_port->base.wait_send_list;
      		if(list_type == 1) list = dst_port->base.wait_receive_list;
        		if(list != NULL){
                 event = list;
                 while(event != NULL && event->esb->dst_port != dst_port){
                     	event = event->next;
                   }
                   if(event != NULL){
                   	  return event->esb;
                   }
        		}
        		break;
      	}
      	case K5_CALL:
      	case K5_SEND:
      	{
      		if(list_type == 0) list=dst_port->base.wait_send_list;
      		if(list_type == 1) list=dst_port->base.wait_receive_list;
        		if(list != NULL){
                   event=list;
                   while(event != NULL && event->esb->src_port != src_port){
                   	   event = event->next;
                   }
                   if(event != NULL){
                   	   return event->esb;
                   }
        		}
        		break;
      	}
      	default:
      	{
      		  printk("K_ERR: Unknown service\n");
      	}
    }
    return NULL;
}

 /**
 *功能: 在对应目标进程的指定类型的事件队列中插入相应事件帧
 *输入参数:
 *    tK5_esb *esb : 待插入的事件帧地址,该参数不能为空
 *    tU4 list_port : 对应的目标进程id,该参数不能为空
 *    int list_type : 所需查找的事件队列类型,仅为0(代表等待发送队列),或2(代表等待接收队列)  
 *输出参数:
 *    false : 输入参数有误插入不成功
 *    true : 插入事件成功
 */
bool insert_event(tK5_esb *esb ,tU4 list_port ,int list_type){
    if (esb == NULL || (list_type != 0 && list_type != 1) || list_port == NULL){
         printk("K_ERR: input parm error\n");
         return false;
    }
    struct k_thread *port = (struct k_thread *)list_port;
    EventNode *event = k_malloc(sizeof(EventNode));
    event->esb = esb; 
  	if(list_type == 0){ 
  	    event->next = port->base.wait_send_list; 
      	port->base.wait_send_list = event; //头插法
  	}
  	if(list_type == 1){ 
  	    event->next = port->base.wait_receive_list; 
      	port->base.wait_receive_list = event; //头插法
  	}       

	return true;
}

 /**
 *功能: 在对应目标进程的指定类型的事件队列中删除相应事件帧
 *输入参数:
 *    tK5_esb *esb : 待删除的事件帧地址,该参数不能为空
 *    tU4 list_port : 对应的目标进程id,该参数不能为空
 *    int list_type : 所需查找的事件队列类型,仅为0(代表等待发送队列),或2(代表等待接收队列)  
 *输出参数:
 *    false : 输入参数有误或待删除事件不存在使删除不成功
 *    true : 删除事件成功
 */
bool delete_event(tK5_esb *esb ,tU4 list_port ,int list_type){
    if (esb == NULL || (list_type != 0 && list_type != 1) || list_port == NULL){
         printk("K_ERR: input parm error\n");
         return false;
    }
    struct k_thread *port = (struct k_thread *)list_port;
    EventNode *list;    
  	if(list_type == 0){ list = port->base.wait_send_list;}
  	if(list_type == 1){ list = port->base.wait_receive_list;}  	
      EventNode *pre_event = list;
      while(list->esb!=esb&&list != NULL){
         pre_event = list;
         list = list->next;
      }
      if(list != NULL){
        if(pre_event == list){
      		 list = list->next;
      		 k_free(port->base.wait_send_list);
      		 port->base.wait_send_list = list;
      		 
      	}
      	else{
          pre_event->next = list->next;
          k_free(list);
      	}
        return true;
      }
      else{
      	printk("KK_ERR:This event is not in the EventLinkList\n");
      	return false;
      }
 }	