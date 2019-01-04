/*
**************************************************************
* 内核内部的通信原语函数定义
*  同步:
*     kk_call()
*     kk_wait()
*  异步:
*     kk_send()
*     kk_reply()
**************************************************************
*/

#include <kernel.h>
#include <string.h>
#include <misc/printk.h>
#include <k5_shared_memory.h>

/**
 *功能： 内核内部的同步等待接收原语，等待接收服务请求或服务确认;
 *输入参数:
 *    tK5_esb *esb : 包含服务id和待处理数据的事件帧结构，该参数不为NULL
 *    tK5_net      *from : 指定期望的端口号及网络地址
 *    tU4          w_len : 接收缓冲区长度，使用零拷贝时不用
 *    tU4         *w_buf : 接收缓冲区地址，使用零拷贝时不用
 *输出参数：
 *　　　　KK_NO_ACCESS : 传入的事件帧或指定期望的端口号或网络地址不存在
 *    esb->size : 原语接收成功,且指定缓冲区足够大时,返回收到的帧结构大小
 *    KK_COMPLETE : 原语接收成功,但指定缓冲区不够大时
 */
tU4  kk_wait  ( 
     tK5_esb       *esb,  //ESB总线数据结构
     tK5_net      *from,  //指定期望的端口号及网络地址 
     tU4          w_len,  //接收缓冲区长度，使用零拷贝时不用
     tU4         *w_buf   //接收缓冲区地址，使用零拷贝时不用
     ) 
{

    if (esb == NULL)  return KK_NO_ACCESS;

    memset(esb ,0 ,K5_MAX_BUF);   //清零ESB帧结构,整页

    esb->primitive = K5_WAIT;              //设置服务原语
    int  i ,j;
    if (from != NULL)                    //为空表示等待任意地址端口
    {
       esb->head      = from->net_level;   //设置帧扩展长度
       esb->dst_port  = from->dst_port;    //设置等待特定目的端口
       esb->src_port  = from->src_port;    //设置源端口

       tK5_ehn  ehn; 
       if (from->net_level >= K5_N1  && from->net_level <= K5_N6) 
       {
          for(i=0 ,j=0 ;i<6 ;i++ ,j++) 
          {   
             ehn.dst_addr = from->hn[i].dst_addr; //设置等待第i级网络目的地址
             ehn.src_addr = from->hn[i].src_addr; //第i级网络源地址
             memcpy (&esb->body[j] , &ehn , sizeof(tU8));   //cast映射到body[j]
          }; //结束for 
       };    
    };         //结束if from


    // 直接切换到相应的服务线程!!!!!
    int  param;
    while(k_msgq_get(&my_msgq ,&param ,K_FOREVER) == 0){
      memcpy(esb ,(tK5_esb *)param ,sizeof(tK5_esb));
      *w_buf = param;
    	tU4 service = esb->service;
    	tK5_svc  *svc = (tK5_svc *)&service;  
    	printk("传到esb_bus_server中的esb的param:\nservice服务号:%d\n" ,svc->svc_type *16 + svc->svc_func);
    	printk("body[0]:%lld\n",esb->body[0]);

      break;
    }

    if (w_len>esb->size && w_buf != NULL)       //w_len>=esb->size 若指定缓冲区且足够大
    {
    	memcpy(w_buf ,esb ,esb->size); //则连头拷贝到用户缓冲区
      return (esb->size);            //给用户返回接收到帧长度
    }
    else 
      return KK_COMPLETE ;          //否则返回1，内容在ESB中

};   //end of kk_wait


/**
 *功能： 内核内部的异步应答原语（ack+nak)，确认服务请求，并返回服务结果，然后继续运行;
 *输入参数:
 *    tK5_esb *esb : 包含服务id和待处理数据的事件帧结构，该参数不为NULL
 *    U2   ack_err : 为正数表示确认序列号，为负数表示错误编号；
 *    tU4    s_len : 发送缓冲区长度,按8字节计，含帧头；
 *    tU4   *s_buf : 发送缓冲区地址,带服务结果数据；
 *输出参数：
 *　　　　KK_NO_ACCESS : 传入的事件帧为NULL
 *    esb->size : 原语调用成功,返回帧结构大小
 *   
 */
tU4  kk_reply  (     //reply原语不将目的端口地址与源端口地址对调！！！
     tK5_esb  *esb,  //ESB总线帧数据结构
     tU2   ack_err,  //为正数表示确认序列号，为负数表示错误编号；
     tU4     s_len,  //发送缓冲区长度,按8字节计，含帧头；
     tU4    *s_buf   //发送缓冲区地址,带服务结果数据；
     )
{

    tK5_eh1   *eh1;    //扩展头结构，8字节

    if ( esb == NULL  )  return KK_NO_ACCESS;

    esb->primitive = K5_REPLY;         //设置服务原语
    esb->head = K5_H1;                 //设置扩展头部H1

    eh1 = (tK5_eh1 *)&esb->body[0];  //获取并展开扩展头部
    eh1->ack_seq  = ack_err;            //确认已收到的帧序号
    memcpy(&esb->body[0] ,eh1 ,8);      //设置扩展头部 

    esb->size = (esb->head +1) * 8;

    if (s_len > 0 && s_buf != NULL)  //若有服务结果数据送回
    {
        memcpy (&esb->body[esb->head] ,s_buf, s_len);
        esb->size = esb->size + s_len;   //设置帧长度 = 头长+体长
    };

    // 直接切换到相应的服务线程!!!!!
    //kk_switch_to ( esb, current, next );

    /*将传回的ESB帧地址放入传回消息队列里*/
    int param = (int)esb;
    k_msgq_put(&my_msgq_callback ,&param ,K_NO_WAIT);
    return (esb->size);  //不等待软中断处理结束，返回发送长度；

  }; // 结束原语kk_reply


/**
 *功能： 内核内部的异步发送原语，主动发送数据或通知信号，然后继续运行;
 *输入参数:
 *    tK5_esb *esb : 包含服务id和待处理数据的事件帧结构，该参数不为NULL
 *    tU2      service : 服务编码,该参数不能越界
 *    tK5_net      *to : 目的端口号（pid,fd,sock）及网络地址,该参数不为NULL
 *    tU4        s_len : 同步调用原语的缓冲区长度(长字)
 *    tU4       *s_buf : 同步调用原语的缓冲区地址 
 *输出参数：
 *    KK_NO_SERVER : 传入的系统服务id信息越界
 *　　　　KK_NO_ACCESS : 传入的事件帧或目的信息或网络地址不存在
 *    esb->size : 原语调用成功,返回发送的帧结构大小
 */
tU4  kk_send  ( 
     tK5_esb     *esb,  //ESB总线数据结构   
     tU2      service,  //服务编码
     tK5_net      *to,  //目的端口号（pid,fd,sock）及网络地址  
     tU4        s_len,  //发送数据长度（按8个字节长字计数）
     tU4       *s_buf   //发送数据缓冲区地址 
     )
{

    tK5_svc  *svc;    //服务编码的展开结构，14比特展开4字节
    tK5_ehn  ehn;    //ESB扩展网络地址，8字节
    tU1      i, j;   //循环临时变量，2018-10-03增加；

    svc = (tK5_svc *)&service;                 //展开14比特的服务编码
    int svc_num=svc->svc_type*16+svc->svc_func;
    if (svc_num <= 0 || svc_num > K5_LAST_SVC)  return KK_NO_SERVER;
    if (esb == NULL  || to == NULL)  return KK_NO_ACCESS;

    memset(esb, 0, K5_MAX_BUF);  //清零ESB帧结构,整页

    esb->primitive = K5_SEND;              //设置服务原语
    esb->service   = service;              //设置服务编码
    esb->head      = to->net_level;        //设置帧扩展长度
    esb->dst_port  = to->dst_port;         //设置目的端口
    esb->src_port  = to->src_port;         //设置源端口

    if(to->net_level >= K5_H1 ) {   }      //设置扩展头body[0],已清零

    if(to->net_level >= K5_N1 && to->net_level <= K5_N6 ) 
    {
       for(i=0,j=0; i<6; i++,j++) 
       {
          ehn.dst_addr = to->hn[i].dst_addr; //第i级网络目的地址
          ehn.src_addr = to->hn[i].src_addr; //第i级网络源地址
          //esb->body[j] = (tU8)ehn;   error trans
          memcpy ( &esb->body[j] ,&ehn ,sizeof(tU8));     //cast映射到body[j]
        } //结束for 
    }       //结束if to

    if (s_len > 0 && s_buf != NULL )         //若有数据要发送
    {                                         //拷贝到ESB
        memcpy (&esb->body[esb->head], s_buf,s_len);
        esb->size = (esb->head +1) * 8 + s_len;        //帧总长=头长+体长
    } //结束if 

    // 直接切换到相应的服务线程!!!!!
    //kk_switch_to ( esb, current, next );

    return (esb->size);  //不等待软中断处理结束，返回发送长度；

} // end of k5_send

/**
 *功能： 内核内部的同步调用请求服务原语，一直等待对方应答后，才释放发送缓冲区
 *输入参数:
 *    tK5_esb *esb : 包含服务id和待处理数据的事件帧结构，该参数不为NULL
 *    tU2      service : 服务编码,该参数不能越界
 *    tK5_net      *to : 目的端口号（pid,fd,sock）及网络地址,该参数不为NULL
 *    tU4        c_len : 同步调用原语的缓冲区长度(长字)
 *    tU4       *c_buf : 同步调用原语的缓冲区地址 
 *输出参数：
 *    KK_NO_SERVER : 传入的系统服务id信息越界
 *　　　　KK_NO_ACCESS : 传入的事件帧或目的信息或网络地址不存在
 *    esb->size : 原语调用成功,且为输入型服务且指定缓冲区足够大时,返回收到的帧结构大小
 *    KK_COMPLETE : 原语调用成功,但不为输入型服务或指定缓冲区不够大时
 */
tU4  kk_call (   
     tK5_esb     *esb,  //ESB总线数据结构；            
     tU2      service,  //服务编码
     tK5_net      *to,  //目的端口号（pid,fd,sock）及网络地址  
     tU4        c_len,  //同步调用原语的缓冲区长度(长字)
     tU4       *c_buf   //同步调用原语的缓冲区地址 
     )
{

    tK5_svc  *svc;    //服务编码的展开结构，14比特展开4字节
    tK5_ehn  ehn;    //ESB扩展网络地址，8字节
    tU1      i, j;   //循环临时变量，2018-10-03增加；

    svc = (tK5_svc *)&service;                 //展开14比特的服务编码
    int svc_num=svc->svc_type*16+svc->svc_func;
    if (svc_num <= 0 || svc_num > K5_LAST_SVC)  return KK_NO_SERVER;
    if (esb == NULL  || to == NULL)  return KK_NO_ACCESS;

    memset(esb, 0, K5_MAX_BUF);  //清零ESB帧结构,整页

    esb->primitive = K5_CALL;              //设置服务原语
    esb->service   = service;              //设置服务编码
    esb->head      = to->net_level;        //设置帧扩展长度
    esb->dst_port  = to->dst_port;         //设置目的端口
    esb->src_port  = to->src_port;         //设置源端口,2018-10-03

    if(to->net_level >= K5_H1 ) {   }      //设置扩展头body[0],已清零

    if(to->net_level >= K5_N1 && to->net_level <= K5_N6 ) 
    {
       for(i=0 ,j=0 ;i<6 ;i++ ,j++) 
       {
          ehn.dst_addr = to->hn[i].dst_addr; //第i级网络目的地址
          ehn.src_addr = to->hn[i].src_addr; //第i级网络源地址
          //esb->body[j] = (tU8)ehn;   error trans
          memcpy ( &esb->body[j] ,&ehn ,sizeof(tU8));     //cast映射到body[j]
        } //结束for 
    }       //结束if to

    //若为输出型服务，且指定缓冲区非空,则将其拷贝到ESB

    if ( svc->svc_inout==1 && c_len>0 && c_buf!=NULL ) 
        {memcpy ( &esb->body[0], c_buf , c_len);}

    esb->size = (esb->head+1)*8 + c_len;         //帧总长 = 头长+体长

    // 直接切换到相应的服务线程!!!!!
    //kk_switch_to ( esb, current, next );
    

         
    //若为输入型服务且指定缓冲区足够大, 则将ESB内容拷贝给用户

    if (svc->svc_inout==0 && c_len>=esb->size && c_buf!=NULL) 
    {  
        memcpy (c_buf,esb ,esb->size); //则连头拷贝到用户缓冲区
        return (esb->size);                      //给用户返回接收到帧长度
    }
    else return (1) ;              //否则返回1，内容在ESB中

}
