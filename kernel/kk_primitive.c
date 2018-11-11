#include <k5_esb.h>
#include <string.h>
#include <misc/printk.h>
//-------------------------------------------------------------------------
//同步等待接收原语（原RS)，等待接收服务请求或服务确认;
//-------------------------------------------------------------------------

tU4  kk_wait  ( 
     tK5_esb       *esb,  //ESB总线数据结构，2018-10-03修改加*；
     tK5_net      *from,  //指定期望的端口号及网络地址 
     tU4          w_len,  //接收缓冲区长度，使用零拷贝时不用
     tU1         *w_buf   //接收缓冲区地址，使用零拷贝时不用
     ) 
{

if ( esb == NULL  )  return ( -1 );

memset(esb,0,K5_ESB_PAGE);   //清零ESB帧结构,整页

esb->primitive = K5_WAIT;              //设置服务原语
int i,j;
if ( from != NULL )                    //为空表示等待任意地址端口
{
     esb->head      = from->net_level;   //设置帧扩展长度
     esb->dst_port  = from->dst_port;    //设置等待特定目的端口
     esb->src_port  = from->src_port;    //设置源端口

     tK5_ehn  ehn; 
     
     if (from->net_level >= K5_N1  && from->net_level <= K5_N6) 
     {
        for(i=0,j=0;i<6;i++,j++) 
        {   
           ehn.dst_addr = from->hn[i].dst_addr; //设置等待第i级网络目的地址
           ehn.src_addr = from->hn[i].src_addr; //第i级网络源地址
           memcpy ( &esb->body[j], &ehn , sizeof(tU8));   //cast映射到body[j]
        }; //结束for 
     };    
};         //结束if from

// 直接切换到相应的服务线程!!!!!
//kk_switch_to ( esb, current, next );

while(k_msgq_get(&my_msgq,esb,K_FOREVER)==0){
   for(j=0;j<511;j++){
   k_msgq_get(&my_msgq,&esb->body[j],K_FOREVER);
   }
  int num=k_msgq_num_used_get(&my_msgq);
	printk("从消息队列中取出ESB帧结构数据\n");
	printk("取出后消息队列里数据组数:%d\n",num);
// while(esb_bus!=NULL){
// 	memcpy(esb,esb_bus,sizeof(tK5_esb));
	tU4 service=esb->service;
	tK5_svc  *svc=(tK5_svc *)&service;  
	printk("传到esb_bus_server中的param:\nservice服务号:%d\n",svc->svc_type*16+svc->svc_func);
	printk("body[0](address):%lld\nbody[510]:%lld\n",esb->body[0],esb->body[510]);
 // esb_bus=NULL;
  break;
}

                                             //2018-10-03增加并修改
if (w_len>=esb->size && w_buf!=NULL)       //若指定缓冲区且足够大
{
	memcpy(w_buf,esb,esb->size); //则连头拷贝到用户缓冲区
    return (esb->size);                      //给用户返回接收到帧长度
}
else return (1) ;                      //否则返回1，内容在ESB中

};   //end of kk_wait

//-------------------------------------------------------------------------
//异步应答原语（原SA,ack+nak)，确认服务请求，并返回服务结果，然后继续等待;
//-------------------------------------------------------------------------
tU4  kk_reply  (     //reply原语不将目的端口地址与源端口地址对调！！！
     tK5_esb  *esb,  //ESB总线帧数据结构
     tU2   ack_err,  //为正数表示确认序列号，为负数表示错误编号；
     tU4     s_len,  //发送缓冲区长度,按8字节计，含帧头；
     tU1    *s_buf   //发送缓冲区地址,带服务结果数据；
     )
{

tK5_eh1   *eh1;    //扩展头结构，8字节

if ( esb == NULL  )  return ( -1 );

esb->primitive = K5_REPLY;         //设置服务原语
esb->head = K5_H1;                 //设置扩展头部H1

eh1 = (tK5_eh1 *)&esb->body[0];  //获取并展开扩展头部
eh1->ack_seq  = ack_err;            //确认已收到的帧序号
memcpy(&esb->body[0],eh1,8);      //设置扩展头部 

if (s_len > 0 && s_buf != NULL )  //若有服务结果数据送回
{
    memcpy (&esb->body[esb->head], s_buf, s_len);
    esb->size = (esb->head+1)*8+s_len;   //设置帧长度 = 头长+体长
};

// 直接切换到相应的服务线程!!!!!
//kk_switch_to ( esb, current, next );
  for(int j=0;j<512;j++){
      if(j==0){k_msgq_put(&my_msgq_back,&esb,K_NO_WAIT);}
        k_msgq_put(&my_msgq_back,&esb->body[j-1],K_NO_WAIT);
    }

    int num=k_msgq_num_used_get(&my_msgq_back);
    printk("傳回消息队列里数据组数:%d\n",num);

   
  return (esb->size);  //不等待软中断处理结束，返回发送长度；

}; // 结束原语kk_reply