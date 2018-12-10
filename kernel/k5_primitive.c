//#include <k5_esb.h>
#include <string.h>
#include <misc/printk.h>
#include <kernel.h>
#include <k5_shared_memory.h>
//-------------------------------------------------------------------------
// 同步调用请求服务原语（原SS），一直等待对方应答后，才释放发送缓冲区
//-------------------------------------------------------------------------

tU4  k5_call (   
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
if ( svc_num <= 0 || svc_num > K5_LAST_SVC  )  return ( -1 );
if ( esb == NULL  || to == NULL         )  return ( -2 );

 memset(esb, 0, K5_MAX_BUF);  //清零ESB帧结构,整页

 esb->primitive = K5_CALL;              //设置服务原语
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
      memcpy ( &esb->body[j], &ehn , sizeof(tU8));     //cast映射到body[j]
    } //结束for 
}       //结束if to



//若为输出型服务，且指定缓冲区非空,则将其拷贝到ESB

if ( svc->svc_inout==1 && c_len>0 && c_buf!=NULL ) 
    {memcpy ( &esb->body[0], c_buf , c_len);}



esb->size = (esb->head+1)*8 + c_len;         //帧总长 = 头长+体长

// 已经准备好ESB帧头和帧体，可以通过软中断陷入内核了!!!!!!

__asm__ volatile (
                  "mov r12 ,%[esb]\n\t" \   
                  "svc %[id]"             
                  :
                  : [id] "i" (1),[esb]"X"(esb)
                  : "memory");
int esb1;
k_msgq_get(&my_msgq_callback,&esb1,K_FOREVER);
memcpy(esb,(tK5_esb *)esb1,sizeof(tK5_esb));
printk("-----------------------------------\ncall原语取出传回消息队列里ESB帧结构地址\n");



     
//若为输入型服务且指定缓冲区足够大, 则将ESB内容拷贝给用户
if (svc->svc_inout==0 && c_len>=esb->size && c_buf!=NULL) 
{  
    memcpy (c_buf,esb ,esb->size); //则连头拷贝到用户缓冲区
    return (esb->size);                      //给用户返回接收到帧长度
}
else return (1) ;              //否则返回1，内容在ESB中

}


//-------------------------------------------------------------------------
//同步等待接收原语（原RS)，等待接收服务请求或服务确认;
//-------------------------------------------------------------------------

tU4  k5_wait  ( 
     tK5_esb       *esb,  //ESB总线数据结构，
     tK5_net      *from,  //指定期望的端口号及网络地址 
     tU4          w_len,  //接收缓冲区长度，使用零拷贝时不用
     tU4         *w_buf   //接收缓冲区地址，使用零拷贝时不用
     ) 
{

if ( esb == NULL  )  return ( -1 );

memset(esb, 0, K5_MAX_BUF); //清零ESB帧结构,整页

esb->primitive = K5_WAIT;              //设置服务原语
esb->dst_port  = k_current_get(); //自己添加的

tK5_ehn  ehn; 
tU1      i, j;   
if ( from != NULL )                    //为空表示等待任意地址端口
{
     esb->head      = from->net_level;   //设置帧扩展长度
     esb->src_port  = from->src_port;    //设置等待特定源端口

     if (from->net_level >= K5_N1 && from->net_level <= K5_N6 ) 
     {
        for( i=0, j=0; i<6; i++,j++) 
        {   
           ehn.src_addr = from->hn[i].src_addr; //设置等待第i级网络源地址
           memcpy(&esb->body[j], &ehn ,sizeof(tU8));           //cast映射到body[j]
        }; //结束for 
     };    //结束if
};         //结束if from

// 已经准备好ESB帧头和帧体，可以通过软中断陷入内核了!!!!!!

__asm__ volatile (
                  "mov r12 ,%[esb]\n\t" \   
                  "svc %[id]"             
                  :
                  : [id] "i" (1),[esb]"X"(esb)
                  : "memory");

int esb1;
k_msgq_get(&my_msgq_callback,&esb1,K_FOREVER);
memcpy(esb,(tK5_esb *)esb1,sizeof(tK5_esb));
printk("-----------------------------------\nwait原语取出传回消息队列里ESB帧结构地址\n");

                           
if (w_len>=esb->size && w_buf!=NULL )       //若指定缓冲区且足够大
   {
    memcpy (w_buf,esb ,esb->size);  //则连头拷贝到用户缓冲区
    return (esb->size);                      //给用户返回接收到帧长
    } else return (1) ;                      //否则返回1，内容在ESB中

};   //结束 k5_wait

//-------------------------------------------------------------------------
//异步应答原语（原SA,ack+nak)，确认服务请求，并返回服务结果，然后继续等待;
//-------------------------------------------------------------------------

tU4  k5_reply  (     //reply原语不将目的端口地址与源端口地址对调！！！
     tK5_esb  *esb,  //ESB总线帧数据结构
     tU2   ack_err,  //为正数表示确认序列号，为负数表示错误编号；
     tU4     s_len,  //发送缓冲区长度,按8字节计，含帧头；
     tU4    *s_buf   //发送缓冲区地址,带服务结果数据；
     )
{

tK5_eh1  * eh1;    //扩展头结构，8字节

if ( esb == NULL )  return ( -1 );

esb->primitive = K5_REPLY;         //设置服务原语
esb->head = K5_H1;                 //设置扩展头部H1

eh1 = (tK5_eh1 *)&esb->body[0];  //获取并展开扩展头部
eh1->ack_seq == ack_err;  //确认已收到的帧序号
memcpy(&esb->body[0],eh1,sizeof(tU8));       //设置扩展头部 

if (s_len > 0 && s_buf != NULL )  //若有服务结果数据送回
{
    memcpy(&esb->body[esb->head], s_buf, s_len);
    esb->size = (esb->head+1)*8 +s_len;   //设置帧长度 = 头长+体长
};

// 已经准备好ESB帧头和帧体，可以通过软中断陷入内核了!!!!!!
__asm__ volatile (
                  "mov r12 ,%[esb]\n\t" \   
                  "svc %[id]"             
                  :
                  : [id] "i" (1),[esb]"X"(esb)
                  : "memory");
  
  return (esb->size);  //不等待软中断处理结束，返回发送长度；

}; // 结束原语k5_reply

//-------------------------------------------------------------------------
//异步发送原语（原SA)，主动发送数据或通知信号，然后继续等待;
//-------------------------------------------------------------------------

tU4  k5_send  ( 
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
if ( svc_num <= 0 || svc_num > K5_LAST_SVC  )  return ( -1 );
if ( esb == NULL  || to == NULL         )  return ( -2 );

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
      memcpy ( &esb->body[j], &ehn , sizeof(tU8));     //cast映射到body[j]
    } //结束for 
}       //结束if to

if (s_len > 0 && s_buf != NULL )         //若有数据要发送
{                                         //拷贝到ESB
    memcpy (&esb->body[esb->head], s_buf,s_len);
    esb->size = (esb->head+1)*8 + s_len;        //帧总长=头长+体长
} //结束if 

// 已经准备好ESB帧头和帧体，可以通过软中断陷入内核了!!!!!!

 __asm__ volatile (
                  "mov r12 ,%[esb]\n\t" \   
                  "svc %[id]"             
                  :
                  : [id] "i" (1),[esb]"X"(esb)
                  : "memory");
 
return (esb->size);  //不等待软中断处理结束，返回发送长度；

} // end of k5_send