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
    printk("Start client thread\n");
    show_regs();
    
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
    to.dst_port=1;//应该是server_id,目标线程
    esb.body[510]=433;
    printk("serv->svc_type:%d\nserv->svc_func:%d\nserv->svc_inout:%d\n",serv->svc_type,serv->svc_func,serv->svc_inout);
    printk("esb->body[510]:%lld\n",esb.body[510]);


    printk("-----------------------------------\n调用ESB k5_call通信原语\n");
    k5_call(&esb,service,&to,80,buffer);
    //serv->svc_func=0;
    //tK5_esb     esb1;
    //k5_call(&esb1,service,&to,0,NULL);

    show_regs();
    printk("client thread end!\n");
}

void thread_server(){
    printk("Start server thread\n");
    show_regs();
    printk("-----------------------------------\n设置参数:\n");
    tK5_esb     esb;
    tK5_net     from;
    from.dst_port=1;//应该是client_id,源线程
    esb.body[510]=334;
    printk("esb->body[510]:%lld\n",esb.body[510]);
    printk("-----------------------------------\n调用ESB k5_wait通信原语\n");
    k5_wait(&esb,&from,0,NULL);

    show_regs();
    printk("server thread end!\n");
}


void main(void){
  printk("-----------------------------------\nARCH: %s\n", CONFIG_ARCH);

   k_thread_create(&server,server_stack,STACKSIZE,thread_server,0,0,0,1,0,K_NO_WAIT);
   k_thread_create(&client,client_stack,STACKSIZE,thread_client,0,0,0,1,0,K_NO_WAIT);
   
}

