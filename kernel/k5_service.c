/**
**************************************************************************
*  查找系统服务向量表调用系统服务函数定义
* 
**************************************************************************
*/

#include <kernel.h>

/**
 *功能： 根据帧结构中的服务id信息查找系统服务向量表并调用系统服务接口，根据帧结构传入数据完成相应系统服务
 *输入参数：
 *    tK5_esb *esb : 包含服务id和待处理数据的事件帧结构，该参数不为NULL
 *输出参数：
 *    KK_COMPLETE : 完成相应的系统服务调用
 *　　　　KK_NO_SERVER : 传入的系统服务id错误
 */
int search_server_num(tK5_esb *esb){
	tU4 service = esb->service;
    tK5_svc  *serv = (tK5_svc *)&service;  
	tU4 serv_num = serv->svc_type*16 + serv->svc_func;
    switch(serv_num)
	 {
	 	case thr_start  :
	 	  {
	 	  	printk("调用创建线程服务\n");
	 	  	struct k_thread *my_thread = (struct k_thread *)esb->body[0];
			char *thread_stack = (char *)esb->body[1];
			int stacksize = (int)esb->body[2];
			void *entry = (void *)esb->body[3];
			void *param0 = (void *)esb->body[4];
			void *param1 = (void *)esb->body[5];
			void *param2 = (void *)esb->body[6];
			int prio = (int)esb->body[7];
			u32_t options = (u32_t)esb->body[8];
			s32_t delay = (s32_t)esb->body[9];

			_impl_k_thread_create(my_thread,thread_stack,stacksize,entry,param0,param1,param2,prio,options,delay);
			k_sleep(100);

	        esb->body[510] = 666;
	        printk("设置ESB的body[510]返回测试数据:%lld\n",esb->body[510]);
			
	 	  	break;
	 	  }
	    default: //未知系统服务
	      {
			 printk("K_ERR: Unknown service [%x]\n",serv_num );
			 esb->body[510] = 333;
	         printk("设置ESB的body[510]返回测试数据:%lld\n",esb->body[510]);
	         return KK_NO_SERVER; 	
	      }  //结束default

         return KK_COMPLETE ;
	 }//结束查找服务向量表
}

