#include <k5_esb.h>
#include <string.h>
#include <misc/printk.h>
#include <kernel.h>
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

 memset(esb, 0, K5_ESB_PAGE);  //清零ESB帧结构,整页

 esb->primitive = K5_CALL;              //设置服务原语
 esb->service   = service;              //设置服务编码
 esb->head      = to->net_level;        //设置帧扩展长度
 esb->dst_port  = to->dst_port;         //设置目的端口
 esb->src_port  = to->src_port;         //设置源端口,2018-10-03



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



//若为输出型服务，且指定缓冲区非空,则将其拷贝到ESB，2018-10-04增加;

if ( svc->svc_inout==1 && c_len>0 && c_buf!=NULL ) 
    {memcpy ( &esb->body[0], c_buf , c_len);}





esb->size = (esb->head+1)*8 + c_len;         //帧总长 = 头长+体长

// 已经准备好ESB帧头和帧体，可以通过软中断陷入内核了!!!!!!
//以下与处理器架构密切相关，由各家自行实现！此处仅示意！

__asm__ volatile (
                  "mov r12 ,%[esb]\n\t" \   
                  "svc %[id]"             
                  :
                  : [id] "i" (1),[esb]"X"(esb)
                  : "memory");

for(int n=0;n<512;n++){
  if(n==0){k_msgq_get(&my_msgq_back,esb,K_FOREVER);}
  k_msgq_get(&my_msgq_back,&esb->body[n-1],K_FOREVER);
}
int num=k_msgq_num_used_get(&my_msgq_back);
printk("-------------------------\n取出后消息队列里数据组数:%d\n",num);
printk("得到传回的数据:%lld\n",esb->body[510]);


     
//若为输入型服务且指定缓冲区足够大, 则将ESB内容拷贝给用户，2018-10-03增加

if (svc->svc_inout==0 && c_len>=esb->size && c_buf!=NULL) 
{  
    memcpy (c_buf,esb ,esb->size); //则连头拷贝到用户缓冲区
    return (esb->size);                      //给用户返回接收到帧长度
}
else return (1) ;                        //否则返回1，内容在ESB中

}