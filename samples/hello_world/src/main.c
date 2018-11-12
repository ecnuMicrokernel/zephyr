/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <misc/printk.h>
#include <k5_esb.h>

#define STACKSIZE 500

/*创建线程的入口函数*/
void test(void)
{
    printk("Create the thread successfully!!\n");  
}


void main(void)
{
  
  printk("-----------------------------------\nARCH: %s\n", CONFIG_ARCH);

  printk("-----------------------------------\n用户线程模式和权限级别相关寄存器值\n");
  int ipsr,control;
   __asm__ volatile(
    "mrs %[ipsr], ipsr\n\t" 
    "mrs %[control], control\n\t"
    : [ipsr] "=r" (ipsr),[control] "=r" (control)
  );
  printk("ipsr:%d\ncontrol:%d\n",ipsr,control);


  printk("-----------------------------------\n创建线程所需10个参数值如下:\n");
  tU8 buffer[10];
  struct k_thread my_thread;
  char thread_stack[STACKSIZE];
  void *param0=0;
  void *param1=0;
  void *param2=0;
  int prio=1;
  u32_t options=0;
  s32_t delay=K_NO_WAIT;
  buffer[0]=(tU8)&my_thread;
  buffer[1]=(tU8)thread_stack;
  buffer[2]=(tU8)STACKSIZE;
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
  tK5_esb     esb;
  tK5_net     to;
  tU4    service;
  tK5_svc * serv = (tK5_svc *)&service;  
  serv->svc_func=1;
  serv->svc_type=3;
  serv->svc_inout=1;
  serv->svc_space=0;
  esb.body[510]=433;

  printk("-----------------------------------\n设置参数:\n");
  printk("serv->svc_type:%d\nserv->svc_func:%d\nserv->svc_inout:%d\n",serv->svc_type,serv->svc_func,serv->svc_inout);
  printk("esb->body[510]:%lld\n",esb.body[510]);


  printk("-----------------------------------\n调用ESB通信原语\n");
  k5_call(&esb,service,&to,sizeof(buffer),buffer);
  serv->svc_func=0;
  tK5_esb     esb1;
  k5_call(&esb1,service,&to,0,NULL);



 
  printk("-----------------------------------\n返回线程模式后的相关寄存器值\n");
  __asm__ volatile(
    "mrs %[ipsr], ipsr\n\t" \
    "mrs %[control], control\n\t"\
    : [ipsr] "=r" (ipsr),[control] "=r" (control)
  );
  printk("ipsr:%d\ncontrol:%x\n",ipsr,control);


  printk("用户应用结束!!\n");


}


