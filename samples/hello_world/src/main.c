/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <misc/printk.h>
//#include <k5_esb.h>
#include <kernel.h>
#include <k5_shared_memory.h>

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

//------------------------------*两个线程之间单个事件的通信原语测试*--------------------------/

/*同步通信原语测试:server先wait,client再call*/
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
    // tK5_esb     esb1;   //放到共享内存区
    tK5_net     to;
    tU4    service;
    tK5_svc * serv = (tK5_svc *)&service;  
    serv->svc_func=1;
    serv->svc_type=3;
    serv->svc_inout=1; //设置操作拷贝buffer到esb帧结构
    serv->svc_space=0;
    to.dst_port=&server;//server_id,目标线程
    to.src_port=&client;
    printk("serv->svc_type:%d\nserv->svc_func:%d\nserv->svc_inout:%d\n",serv->svc_type,serv->svc_func,serv->svc_inout);
    printk("-----------------------------------\n调用ESB k5_call通信原语\n");
    k5_call(&esb1,service,&to,80,buffer);
    //serv->svc_func=0;
    //k5_call(&esb,service,&to,0,NULL);
    
    // show_regs();
    printk("client得到传回的数据esb.body[510]:%lld\n",esb1.body[510]);

    /*------异步通信------*/
    // tK5_net     from;
    // from.src_port=&client;
    // printk("-----------------------------------\n调用ESB k5_wait通信原语\n");
    // k5_wait(&esb1,&from,0,NULL);
    //  改变esb内数据
    // esb1.body[0]=123;
    // printk("-----------------------------------\n调用ESB k5_reply通信原语\n");
    // esb1.src_port=NULL;
    // k5_reply(&esb1,1,0,NULL);
    //printk("%d\n",client.base.list);
    printk("client thread end!\n-----------------------------------\n");
}

void thread_server(){
    printk("-----------------------------------\nStart server thread\n");
    // show_regs();
    printk("-----------------------------------\n设置参数:\n");
    // tK5_esb     esb2;   //放到共享内存区
    tK5_net     from;
    from.src_port=&client;
    printk("-----------------------------------\n调用ESB k5_wait通信原语\n");
    k5_wait(&esb2,&from,0,NULL);
    printk("server得到传回的数据esb.body[510]:%lld\n",esb2.body[510]);
    printk("server得到数据后做相应处理,若是网络通信还要检测帧序号,再设置reply的esb回复数据\n");
    esb2.src_port=&client;
    /*网络通信*/
    //if clinet 发送来的snd_seq=server 原先希望接受的ack.seq
      // tU2 ack_err=1;
    //else ack_err=-1;
    printk("-----------------------------------\n调用ESB k5_reply通信原语\n");
    k5_reply(&esb2,1,0,NULL);
    // show_regs();
  
    /*------异步通信------*/
    //k_sleep(500);
    /*设置参数*/
    // tK5_net     to;
    // to.dst_port=&client;
    // tU4    service;
    // tK5_svc * serv = (tK5_svc *)&service;  
    // serv->svc_func=0;
    // serv->svc_type=3;
    // printk("-----------------------------------\n调用ESB k5_send通信原语\n");
    // k5_send(&esb2,service,&to,0,NULL);
    printk("server thread end!\n-----------------------------------\n");
}


/*同步原语通信测试:client先call,server再wait*/
void thread_client1(){
    printk("-----------------------------------\nStart client thread\n");
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
    // tK5_esb     esb1;   //放到共享内存区
    tK5_net     to;
    tU4    service;
    tK5_svc * serv = (tK5_svc *)&service;  
    serv->svc_func=1;
    serv->svc_type=3;
    serv->svc_inout=1; //设置操作拷贝buffer到esb帧结构
    serv->svc_space=0;
    to.dst_port=&server;//server_id,目标线程
    to.src_port=&client;
    printk("serv->svc_type:%d\nserv->svc_func:%d\nserv->svc_inout:%d\n",serv->svc_type,serv->svc_func,serv->svc_inout);

    printk("-----------------------------------\n调用ESB k5_call通信原语\n");
    k5_call(&esb1,service,&to,80,buffer);
    printk("client thread end!\n-----------------------------------\n");
}

void thread_server1(){
    k_sleep(SLEEPTIME);
    printk("\n-----------------------------------\nStart server thread\n");
    printk("-----------------------------------\n设置参数:\n");
    // tK5_esb     esb2;   //放到共享内存区
    tK5_net     from;
    from.src_port=&client;
    printk("-----------------------------------\n调用ESB k5_wait通信原语\n");
    k5_wait(&esb2,&from,0,NULL);

    printk("server得到传回的数据esb.body[510]:%lld\n",esb2.body[510]);
    printk("-----------------------------------\n调用ESB k5_reply通信原语\n");
    esb2.src_port=&client;
    k5_reply(&esb2,1,0,NULL);
    printk("server thread end!\n-----------------------------------\n");
}



/*-----------------------------------主测试函数-----------------------------------*/
void main(void){
   printk("-----------------------------------\nARCH: %s\n", CONFIG_ARCH);
   k_thread_create(&server,server_stack,STACKSIZE,thread_server1,0,0,0,1,0,K_NO_WAIT);
   k_thread_create(&client,client_stack,STACKSIZE,thread_client1,0,0,0,1,0,K_NO_WAIT);
   
}

