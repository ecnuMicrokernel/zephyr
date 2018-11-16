/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <misc/printk.h>
#include <k5_esb.h>
#include <kernel.h>

#define STACKSIZE 6000
#define SLEEPTIME 50


char client_stack[STACKSIZE];
char server_stack[STACKSIZE];

/*创建线程的入口函数*/
void test(void)
{
    printk("Create the thread successfully!!\n");  
}

/*cortex_m结构下显示寄存器的值*/
void show_regs(){
  printk("-----------------------------------\n此时CPU模式和权限级别相关寄存器值\n");
    int ipsr,control;
     __asm__ volatile(
      "mrs %[ipsr], ipsr\n\t" 
      "mrs %[control], control\n\t"
      : [ipsr] "=r" (ipsr),[control] "=r" (control)
    );
    printk("ipsr:%d\ncontrol:%d\n",ipsr,control);
}


void thread_client(){
    k_sleep(SLEEPTIME);
    printk("-----------------------------------\nStart client thread\n");
   // show_regs();
    
    printk("-----------------------------------\n创建线程所需10个参数值如下:\n");
    tU8 buffer[10];
    struct k_thread my_thread;
    char thread_stack[500];
    void *param0=0;
    void *param1=0;
    void *param2=0;
    int prio=1;
    u32_t options=0;
    s32_t delay=K_NO_WAIT;
    buffer[0]=(tU8)&my_thread;
    buffer[1]=(tU8)thread_stack;
    buffer[2]=(tU8)500;
    buffer[3]=(tU8)test;
    buffer[4]=(tU8)param0;
    buffer[5]=(tU8)param1;
    buffer[6]=(tU8)param2;
    buffer[7]=(tU8)prio;
    buffer[8]=(tU8)options;
    buffer[9]=(tU8)delay;
    for(int i=0;i<10;i++){
       printk("%lld\n",buffer[i]);
    }

    printk("-----------------------------------\n设置参数:\n");
    tK5_esb     esb;
    tK5_net     to;
    tU4    service;
    tK5_svc * serv = (tK5_svc *)&service;  
    serv->svc_func=1;
    serv->svc_type=3;
    serv->svc_inout=1; //设置操作拷贝buffer到esb帧结构
    serv->svc_space=0;
    to.dst_port=&server;//应该是server_id,目标线程!!左右数据大小不匹配
    
    printk("serv->svc_type:%d\nserv->svc_func:%d\nserv->svc_inout:%d\n",serv->svc_type,serv->svc_func,serv->svc_inout);
    printk("dst_port(访问的server线程id):%d\n",to.dst_port);

    printk("-----------------------------------\n调用ESB k5_call通信原语\n");
    k5_call(&esb,service,&to,80,buffer);
    //serv->svc_func=0;
    //k5_call(&esb,service,&to,0,NULL);
    
   // show_regs();
    printk("client得到传回的数据esb.body[510]:%lld\n",esb.body[510]);
    printk("client thread end!\n-----------------------------------\n");
}

void thread_server(){
    printk("-----------------------------------\nStart server thread\n");
   // show_regs();
    printk("-----------------------------------\n设置参数:\n");
    tK5_esb     esb;
    tK5_net     from;
    from.src_port=&client;//应该是client_id,源线程
    printk("src_port(允许访问的client线程id):%d\n",from.src_port);
    printk("-----------------------------------\n调用ESB k5_wait通信原语\n");
    k5_wait(&esb,&from,0,NULL);
    printk("server得到传回的数据esb.body[510]:%lld\n",esb.body[510]);
    printk("server得到数据后做相应处理,若是网络通信还要检测帧序号,再设置reply的esb回复数据\n");
    esb.src_port=1;
    /*网络通信*/
    //if clinet 发送来的snd_seq=server 原先希望接受的ack.seq
      // tU2 ack_err=1;
    //else ack_err=-1;
    printk("-----------------------------------\n调用ESB k5_reply通信原语\n");
    k5_reply(&esb,1,0,NULL);
   // show_regs();
    printk("server thread end!\n-----------------------------------\n");
}


void main(void){
  printk("-----------------------------------\nARCH: %s\n", CONFIG_ARCH);

   k_thread_create(&server,server_stack,STACKSIZE,thread_server,0,0,0,1,0,K_NO_WAIT);
   k_thread_create(&client,client_stack,STACKSIZE,thread_client,0,0,0,1,0,K_NO_WAIT);
   
}

