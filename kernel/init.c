/*
 * Copyright (c) 2010-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Kernel initialization module
 *
 * This module contains routines that are used to initialize the kernel.
 */

#include <zephyr.h>
#include <offsets_short.h>
#include <kernel.h>
#include <misc/printk.h>
#include <misc/stack.h>
#include <random/rand32.h>
#include <linker/sections.h>
#include <toolchain.h>
#include <kernel_structs.h>
#include <device.h>
#include <init.h>
#include <linker/linker-defs.h>
#include <ksched.h>
#include <version.h>
#include <string.h>
#include <misc/dlist.h>
#include <kernel_internal.h>
#include <kswap.h>
#include <entropy.h>
#include <logging/log_ctrl.h>

/*设置ESB总线所需导入头文件*/
#include <irq_offload.h>
#include <k5_shared_memory.h>

/* kernel build timestamp items */
#define BUILD_TIMESTAMP "BUILD: " __DATE__ " " __TIME__

/* boot banner items */
#if defined(CONFIG_BOOT_DELAY) && CONFIG_BOOT_DELAY > 0
#define BOOT_DELAY_BANNER " (delayed boot "	\
	STRINGIFY(CONFIG_BOOT_DELAY) "ms)"
static const unsigned int boot_delay = CONFIG_BOOT_DELAY;
#else
#define BOOT_DELAY_BANNER ""
static const unsigned int boot_delay;
#endif

#ifdef BUILD_VERSION
#define BOOT_BANNER "Booting Zephyr OS "	\
	 STRINGIFY(BUILD_VERSION) BOOT_DELAY_BANNER
#else
#define BOOT_BANNER "Booting Zephyr OS "	\
	 KERNEL_VERSION_STRING BOOT_DELAY_BANNER
#endif

#if !defined(CONFIG_BOOT_BANNER)
#define PRINT_BOOT_BANNER() do { } while (0)
#else
#define PRINT_BOOT_BANNER() printk("***** " BOOT_BANNER " *****\n")
#endif

/* boot time measurement items */

#ifdef CONFIG_BOOT_TIME_MEASUREMENT
u64_t __noinit __start_time_stamp; /* timestamp when kernel starts */
u64_t __noinit __main_time_stamp;  /* timestamp when main task starts */
u64_t __noinit __idle_time_stamp;  /* timestamp when CPU goes idle */
#endif

/* init/main and idle threads */

#define IDLE_STACK_SIZE CONFIG_IDLE_STACK_SIZE
#define MAIN_STACK_SIZE CONFIG_MAIN_STACK_SIZE
#define  ESB_STACK_SIZE 8000  	 

K_THREAD_STACK_DEFINE(_main_stack, MAIN_STACK_SIZE);
K_THREAD_STACK_DEFINE(_idle_stack, IDLE_STACK_SIZE);
K_THREAD_STACK_DEFINE(_esb_stack, ESB_STACK_SIZE);

static struct k_thread _main_thread_s;
static struct k_thread _idle_thread_s;
static struct k_thread _esb_thread_s;

k_tid_t const _main_thread = (k_tid_t)&_main_thread_s;
k_tid_t const _idle_thread = (k_tid_t)&_idle_thread_s;
k_tid_t const _esb_thread = (k_tid_t)&_esb_thread_s;

/*
 * storage space for the interrupt stack
 *
 * Note: This area is used as the system stack during kernel initialization,
 * since the kernel hasn't yet set up its own stack areas. The dual purposing
 * of this area is safe since interrupts are disabled until the kernel context
 * switches to the init thread.
 */
K_THREAD_STACK_DEFINE(_interrupt_stack, CONFIG_ISR_STACK_SIZE);

/*
 * Similar idle thread & interrupt stack definitions for the
 * auxiliary CPUs.  The declaration macros aren't set up to define an
 * array, so do it with a simple test for up to 4 processors.  Should
 * clean this up in the future.
 */
#if defined(CONFIG_SMP) && CONFIG_MP_NUM_CPUS > 1
K_THREAD_STACK_DEFINE(_idle_stack1, IDLE_STACK_SIZE);
static struct k_thread _idle_thread1_s;
k_tid_t const _idle_thread1 = (k_tid_t)&_idle_thread1_s;
K_THREAD_STACK_DEFINE(_interrupt_stack1, CONFIG_ISR_STACK_SIZE);
#endif

#if defined(CONFIG_SMP) && CONFIG_MP_NUM_CPUS > 2
K_THREAD_STACK_DEFINE(_idle_stack2, IDLE_STACK_SIZE);
static struct k_thread _idle_thread2_s;
k_tid_t const _idle_thread2 = (k_tid_t)&_idle_thread2_s;
K_THREAD_STACK_DEFINE(_interrupt_stack2, CONFIG_ISR_STACK_SIZE);
#endif

#if defined(CONFIG_SMP) && CONFIG_MP_NUM_CPUS > 3
K_THREAD_STACK_DEFINE(_idle_stack3, IDLE_STACK_SIZE);
static struct k_thread _idle_thread3_s;
k_tid_t const _idle_thread3 = (k_tid_t)&_idle_thread3_s;
K_THREAD_STACK_DEFINE(_interrupt_stack3, CONFIG_ISR_STACK_SIZE);
#endif

#ifdef CONFIG_SYS_CLOCK_EXISTS
	#define initialize_timeouts() do { \
		sys_dlist_init(&_timeout_q); \
	} while ((0))
#else
	#define initialize_timeouts() do { } while ((0))
#endif

extern void idle(void *unused1, void *unused2, void *unused3);

/* LCOV_EXCL_START */
#if defined(CONFIG_INIT_STACKS) && defined(CONFIG_PRINTK)
extern K_THREAD_STACK_DEFINE(sys_work_q_stack,
			     CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE);


void k_call_stacks_analyze(void)
{
	printk("Kernel stacks:\n");
	STACK_ANALYZE("main     ", _main_stack);
	STACK_ANALYZE("idle     ", _idle_stack);
	STACK_ANALYZE("interrupt", _interrupt_stack);
	STACK_ANALYZE("workqueue", sys_work_q_stack);
}
#else
void k_call_stacks_analyze(void) { }
#endif
/* LCOV_EXCL_STOP */

/**
 *
 * @brief Clear BSS
 *
 * This routine clears the BSS region, so all bytes are 0.
 *
 * @return N/A
 */
void _bss_zero(void)
{
	memset(&__bss_start, 0,
		 ((u32_t) &__bss_end - (u32_t) &__bss_start));
#ifdef CONFIG_CCM_BASE_ADDRESS
	memset(&__ccm_bss_start, 0,
		((u32_t) &__ccm_bss_end - (u32_t) &__ccm_bss_start));
#endif
#ifdef CONFIG_APPLICATION_MEMORY
	memset(&__app_bss_start, 0,
		 ((u32_t) &__app_bss_end - (u32_t) &__app_bss_start));
#endif
}


#ifdef CONFIG_XIP
/**
 *
 * @brief Copy the data section from ROM to RAM
 *
 * This routine copies the data section from ROM to RAM.
 *
 * @return N/A
 */
void _data_copy(void)
{
	memcpy(&__data_ram_start, &__data_rom_start,
		 ((u32_t) &__data_ram_end - (u32_t) &__data_ram_start));
#ifdef CONFIG_CCM_BASE_ADDRESS
	memcpy(&__ccm_data_start, &__ccm_data_rom_start,
		 ((u32_t) &__ccm_data_end - (u32_t) &__ccm_data_start));
#endif
#ifdef CONFIG_APP_SHARED_MEM
	memcpy(&_app_smem_start, &_app_smem_rom_start,
		 ((u32_t) &_app_smem_end - (u32_t) &_app_smem_start));
#endif
#ifdef CONFIG_APPLICATION_MEMORY
	memcpy(&__app_data_ram_start, &__app_data_rom_start,
		 ((u32_t) &__app_data_ram_end - (u32_t) &__app_data_ram_start));
#endif
}
#endif



/**
 *功能：esb总线的主函数，接收事件并根据事件的类型做出相应的事件驱动操作
 *输入参数：
 *　　　　　　无
 *输出参数：
 *      KK_ERR_WAIT : 接收到的帧结构地址为NULL
 *      KK_NO_DST : 接收到的帧结构内无所需的源地址信息
 *      KK_NO_SRC : 接收到的帧结构内无所需的目标地址信息
 *		KK_NO_PRIMITIVE : 接收到的帧结构类型错误
 */
static int esb_server(){

    printk("-----------------------------------\n开启 esb_server \n-----------------------------------\n");
   
    tK5_esb esb;
    tU4 ac_addr;
    tK5_esb *ac_esb;
    tI4   ret = 0;  
    
    //tK5_ServiceType   *stg;  //服务组表(STG);
    //tK5_ServiceVector *svc;  //服务向量表(SVC);
    
    while(1){
        ret=kk_wait(&esb,  NULL, 0, &ac_addr);
        ac_esb = (tK5_esb *)ac_addr;
        if ( ret <= 0 || &esb == NULL ) 
	      {
	        printk("KK_ERR: kk_wait [%d]\n", ret);
	        return  KK_ERR_WAIT;     //-2, 等待失败，推出程序？ 再试？
	       };
	    if ( esb.head >= K5_N1 && esb.body[0] != 0 ) 
	      {
          	//kk_switch_to (current, K5_NET_PROXY,esb ); //切到网络代理  
		    continue;
	      };  
	    // if ( serv->svc_space != 0 ) {		      //如果服务空间不是内核空间
        //         kk_switch_to ( current, K5_USER_PROXY，esb ); //切到用户代理 
		   //  continue;
	    //  };
            // stg =(tK5_ServiceType*)&stg_tab[serv->type][0]; 
	    switch(esb.primitive)     //服务原语分支转移
		{
			case K5_CALL:{
                if(esb.dst_port != NULL){

                	if(search_list(&esb,0) != NULL){ 
                		search_server_num(&esb);//查找服务向量表,调用系统服务
						printk("完成服务后将事件插入客户端进程自身的等待发送事件队列\n");
						insert_event(ac_esb ,esb.src_port ,0);	
						printk("从服务器端的等待发送事件队列移除事件\n");
						delete_event(search_list(&esb ,0) ,esb.dst_port ,0);
						printk("恢复服务器端进程来接受响应\n"); 
						k_thread_resume(esb.dst_port); 
						kk_reply(&esb ,0 ,0 ,NULL);
                	}
                	else{
                		printk("直接插入客户端进程自身的等待发送事件队列\n");
                		insert_event(ac_esb ,esb.src_port ,0);		
                	}
                 }
                else{
                	printk("K_ERR: K5_CALL HAS NO DST_PORT\n"); 
                	return KK_NO_DST;
                }
				break;
			}//结束K5_CALL原语分支
	        case K5_WAIT:{
	        	if(esb.src_port != NULL){
	        		if(search_list(&esb ,1) != NULL){ //等待接受队列
	        			ac_esb = search_list(&esb,1);
	        			memcpy(&esb ,ac_esb ,sizeof(tK5_esb));
	        			printk("移出目标进程的等待接受队列中的结果事件并返回\n");
	        			delete_event(search_list(&esb ,1) ,esb.dst_port ,1);
	        			k_thread_resume(esb.dst_port); 
                        kk_reply(&esb ,0 ,0 ,NULL);
	        		}
	        		else{
	        			if(search_list(&esb ,0) != NULL){
		        			//search_server_num(&esb);
		        			printk("读出客户端等待发送事件队列中的事件内容并调用服务后返回\n");
	                        memcpy(&esb ,search_list(&esb ,0) ,sizeof(tK5_esb));
	                        search_server_num(&esb);
	                        printk("恢复服务器端进程来接受响应\n");
	                        k_thread_resume(esb.dst_port);  
	                        kk_reply(&esb ,0 ,0 ,NULL);
	        
		        		}
		        		else{
			        		search_server_num(&esb);	        
			        		printk("将事件插入服务器端进程自身的等待发送事件队列\n");	
			        		insert_event(ac_esb ,esb.dst_port ,0);	            
		        		}
	        		}		
	        	}
	        	else{
	        		printk("K_ERR: K5_wait HAS NO SRC_PORT\n"); 
	  	        	return KK_NO_SRC;
	        	}
	  	        break;
	        }//结束K5_WAIT原语分支
	        case K5_SEND:{
	        	if(esb.dst_port != NULL){
	        		if(search_list(&esb ,0) != NULL){
	        			search_server_num(&esb);
	        			k_thread_resume(esb.dst_port);  
						delete_event(search_list(&esb ,0) ,esb.dst_port ,0);
						kk_reply(&esb ,0 ,0 ,NULL);
	        		}
	        	    else{
	        	    	search_server_num(&esb);
	        	    	memcpy(ac_esb ,&esb ,sizeof(tK5_esb));
	        	    	printk("调用服务后将结果事件插入目标进程的等待接受队列\n");
	        	    	insert_event(ac_esb ,esb.dst_port ,1);
	        	    } 
	        	    
	  	        }
	  	        else{
                	printk("K_ERR: K5_SEND HAS NO DST_PORT\n"); 
                	return KK_NO_DST;
                }
                break;
	        }//结束K5_SEND原语分支
	        case K5_REPLY:{
	        	if(esb.src_port != NULL){
	        		if(search_list(&esb ,0) != NULL){
	        			search_server_num(&esb);
	        	        printk("从客户端的等待发送事件队列移除事件\n"); 
	        	        delete_event(search_list(&esb ,0) ,esb.src_port ,0);

	        	        //memcpy(ac_esb,&esb,sizeof(tK5_esb));
	        	        printk("恢复源客户端进程来接受响应\n");
	        	        k_thread_resume(esb.src_port);
	        	        kk_reply(&esb ,0 ,0 ,NULL);
	        		}
	        		else{
	        			search_server_num(&esb);
	        			memcpy(ac_esb ,&esb ,sizeof(tK5_esb));
	        			printk("调用服务并将返回结果事件插入源目的进程的等待接受队列\n");
	        	    	insert_event(ac_esb ,esb.src_port ,2);
	        		}
	        	    
	  	        }
	  	        else{
	  	        	printk("K_ERR: K5_REPLY HAS NO SRC_PORT\n"); 
	  	        	return KK_NO_SRC;
	  	        }
	  	        break;
	        }//结束K5_REPLY原语分支
	        default: 
	        {
			    printk("K_ERR: Unknown primitive\n"); 
			    return KK_NO_PRIMITIVE;
	            break; 	
	        }  //结束default未知系统服务
	    }//结束选择服务原语类型

        k_sleep(100);

    }  
}


/**
 *功能：通信原语软中断陷入内核后的执行的软中断服务子例程，切换权限后获得陷入传来的esb帧地址,再将esb帧传递给esb_server
 *输入参数：
 *      无
 *输出参数：
 *　　　　　　无
 **/
extern void show_regs();
static void svc_trap()
{	
	printk("陷入内核执行svc陷入指令的中断例程,将esb帧通过下消息队列传到ESB_server线程中\n");
	  /*提升权限
	  　　若不改变寄存器control[0]的值,异常返回后回到产生异常之前的特权级,
	    也可以改写control[0]的值,来改写退出异常处理事件后的权限级别,变成用户级.
	  */
  	//show_regs();

    int param;
    __asm__ volatile(
      "mov %[param],r12\n\t"\
      : [param]"=r"(param)
      );  

    //插入等待发送到总线的事件队列,调度等待事件队列????
    k_msgq_put(&my_msgq,&param,K_NO_WAIT);      
	return;	
}



/**
 *
 * @brief Mainline for kernel's background thread
 *
 * This routine completes kernel initialization by invoking the remaining
 * init functions, then invokes application's main() routine.
 *
 * @return N/A
 */
static void bg_thread_main(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	_sys_device_do_config_level(_SYS_INIT_LEVEL_POST_KERNEL);
#if CONFIG_STACK_POINTER_RANDOM
	z_stack_adjust_initialized = 1;
#endif
	if (boot_delay > 0) {
		printk("***** delaying boot " STRINGIFY(CONFIG_BOOT_DELAY)
		       "ms (per build configuration) *****\n");
		k_busy_wait(CONFIG_BOOT_DELAY * USEC_PER_MSEC);
	}
	PRINT_BOOT_BANNER();

	/* Final init level before app starts */
	_sys_device_do_config_level(_SYS_INIT_LEVEL_APPLICATION);

#ifdef CONFIG_CPLUSPLUS
	/* Process the .ctors and .init_array sections */
	extern void __do_global_ctors_aux(void);
	extern void __do_init_array_aux(void);
	__do_global_ctors_aux();
	__do_init_array_aux();
#endif

	_init_static_threads();

#ifdef CONFIG_SMP
	smp_init();
#endif

#ifdef CONFIG_BOOT_TIME_MEASUREMENT
	/* record timestamp for kernel's _main() function */
	extern u64_t __main_time_stamp;

	__main_time_stamp = (u64_t)k_cycle_get_32();
#endif


    /*ESB:开启esb_server线程*/
 	 _setup_new_thread(_esb_thread, _esb_stack,
			  ESB_STACK_SIZE, esb_server,
			  NULL, NULL, NULL,
			  1, K_NO_WAIT);
	_mark_thread_as_started(_esb_thread);
	_ready_thread(_esb_thread);  

    /*ESB:初始化esb_server线程与主线程的通信消息队列*/  
	k_msgq_init(&my_msgq,msgq_buf,4,500);
	k_msgq_init(&my_msgq_callback,msgq_buf_callback,4,500);
    
     /*ESB:设置中断处理函数*/
    offload_routine = svc_trap;

    
	extern void main(void);

	main();

	/* Terminate thread normally since it has no more work to do */
	_main_thread->base.user_options &= ~K_ESSENTIAL;
}

void __weak main(void)
{
	/* NOP default main() if the application does not provide one. */
}

#if defined(CONFIG_MULTITHREADING)
static void init_idle_thread(struct k_thread *thr, k_thread_stack_t *stack)
{
#ifdef CONFIG_SMP
	thr->base.is_idle = 1;
#endif

	_setup_new_thread(thr, stack,
			  IDLE_STACK_SIZE, idle, NULL, NULL, NULL,
			  K_LOWEST_THREAD_PRIO, K_ESSENTIAL);
	_mark_thread_as_started(thr);
}
#endif

/**
 *
 * @brief Initializes kernel data structures
 *
 * This routine initializes various kernel data structures, including
 * the init and idle threads and any architecture-specific initialization.
 *
 * Note that all fields of "_kernel" are set to zero on entry, which may
 * be all the initialization many of them require.
 *
 * @return N/A
 */
#ifdef CONFIG_MULTITHREADING
static void prepare_multithreading(struct k_thread *dummy_thread)
{
#ifdef CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN
	ARG_UNUSED(dummy_thread);
#else
	/*
	 * Initialize the current execution thread to permit a level of
	 * debugging output if an exception should happen during kernel
	 * initialization.  However, don't waste effort initializing the
	 * fields of the dummy thread beyond those needed to identify it as a
	 * dummy thread.
	 */

	_current = dummy_thread;

	dummy_thread->base.user_options = K_ESSENTIAL;
	dummy_thread->base.thread_state = _THREAD_DUMMY;
#ifdef CONFIG_THREAD_STACK_INFO
	dummy_thread->stack_info.start = 0;
	dummy_thread->stack_info.size = 0;
#endif
#ifdef CONFIG_USERSPACE
	dummy_thread->mem_domain_info.mem_domain = 0;
#endif
#endif

	/* _kernel.ready_q is all zeroes */
	_sched_init();

#ifndef CONFIG_SMP
	/*
	 * prime the cache with the main thread since:
	 *
	 * - the cache can never be NULL
	 * - the main thread will be the one to run first
	 * - no other thread is initialized yet and thus their priority fields
	 *   contain garbage, which would prevent the cache loading algorithm
	 *   to work as intended
	 */
	_ready_q.cache = _main_thread;
#endif

	_setup_new_thread(_main_thread, _main_stack,
			  MAIN_STACK_SIZE, bg_thread_main,
			  NULL, NULL, NULL,
			  CONFIG_MAIN_THREAD_PRIORITY, K_ESSENTIAL);
	_mark_thread_as_started(_main_thread);
	_ready_thread(_main_thread);

#ifdef CONFIG_MULTITHREADING
	init_idle_thread(_idle_thread, _idle_stack);
	_kernel.cpus[0].idle_thread = _idle_thread;
#endif

#if defined(CONFIG_SMP) && CONFIG_MP_NUM_CPUS > 1
	init_idle_thread(_idle_thread1, _idle_stack1);
	_kernel.cpus[1].idle_thread = _idle_thread1;
	_kernel.cpus[1].id = 1;
	_kernel.cpus[1].irq_stack = K_THREAD_STACK_BUFFER(_interrupt_stack1)
		+ CONFIG_ISR_STACK_SIZE;
#endif

#if defined(CONFIG_SMP) && CONFIG_MP_NUM_CPUS > 2
	init_idle_thread(_idle_thread2, _idle_stack2);
	_kernel.cpus[2].idle_thread = _idle_thread2;
	_kernel.cpus[2].id = 2;
	_kernel.cpus[2].irq_stack = K_THREAD_STACK_BUFFER(_interrupt_stack2)
		+ CONFIG_ISR_STACK_SIZE;
#endif

#if defined(CONFIG_SMP) && CONFIG_MP_NUM_CPUS > 3
	init_idle_thread(_idle_thread3, _idle_stack3);
	_kernel.cpus[3].idle_thread = _idle_thread3;
	_kernel.cpus[3].id = 3;
	_kernel.cpus[3].irq_stack = K_THREAD_STACK_BUFFER(_interrupt_stack3)
		+ CONFIG_ISR_STACK_SIZE;
#endif

	initialize_timeouts();

}

static void switch_to_main_thread(void)
{
#ifdef CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN
	_arch_switch_to_main_thread(_main_thread, _main_stack, MAIN_STACK_SIZE,
				    bg_thread_main);
#else
	/*
	 * Context switch to main task (entry function is _main()): the
	 * current fake thread is not on a wait queue or ready queue, so it
	 * will never be rescheduled in.
	 */

	_Swap(irq_lock());
#endif
}
#endif /* CONFIG_MULTITHREDING */

u32_t z_early_boot_rand32_get(void)
{
#ifdef CONFIG_ENTROPY_HAS_DRIVER
	struct device *entropy = device_get_binding(CONFIG_ENTROPY_NAME);
	int rc;
	u32_t retval;

	if (entropy == NULL) {
		goto sys_rand32_fallback;
	}

	/* Try to see if driver provides an ISR-specific API */
	rc = entropy_get_entropy_isr(entropy, (u8_t *)&retval,
				     sizeof(retval), ENTROPY_BUSYWAIT);
	if (rc == -ENOTSUP) {
		/* Driver does not provide an ISR-specific API, assume it can
		 * be called from ISR context
		 */
		rc = entropy_get_entropy(entropy, (u8_t *)&retval,
					 sizeof(retval));
	}

	if (rc >= 0) {
		return retval;
	}

	/* Fall through to fallback */

sys_rand32_fallback:
#endif

	/* FIXME: this assumes sys_rand32_get() won't use any synchronization
	 * primitive, like semaphores or mutexes.  It's too early in the boot
	 * process to use any of them.  Ideally, only the path where entropy
	 * devices are available should be built, this is only a fallback for
	 * those devices without a HWRNG entropy driver.
	 */
	return sys_rand32_get();
}

#ifdef CONFIG_STACK_CANARIES
extern uintptr_t __stack_chk_guard;
#endif /* CONFIG_STACK_CANARIES */

/**
 *
 * @brief Initialize kernel
 *
 * This routine is invoked when the system is ready to run C code. The
 * processor must be running in 32-bit mode, and the BSS must have been
 * cleared/zeroed.
 *
 * @return Does not return
 */

FUNC_NORETURN void _Cstart(void)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_ARCH_HAS_CUSTOM_SWAP_TO_MAIN
	struct k_thread *dummy_thread = NULL;
#else
	/* Normally, kernel objects are not allowed on the stack, special case
	 * here since this is just being used to bootstrap the first _Swap()
	 */
	char dummy_thread_memory[sizeof(struct k_thread)];
	struct k_thread *dummy_thread = (struct k_thread *)&dummy_thread_memory;

	memset(dummy_thread_memory, 0, sizeof(dummy_thread_memory));
#endif
#endif
	/*
	 * The interrupt library needs to be initialized early since a series
	 * of handlers are installed into the interrupt table to catch
	 * spurious interrupts. This must be performed before other kernel
	 * subsystems install bonafide handlers, or before hardware device
	 * drivers are initialized.
	 */

	_IntLibInit();

	if (IS_ENABLED(CONFIG_LOG)) {
		log_core_init();
	}

	/* perform any architecture-specific initialization */
	kernel_arch_init();

	/* perform basic hardware initialization */
	_sys_device_do_config_level(_SYS_INIT_LEVEL_PRE_KERNEL_1);
	_sys_device_do_config_level(_SYS_INIT_LEVEL_PRE_KERNEL_2);

#ifdef CONFIG_STACK_CANARIES
	__stack_chk_guard = z_early_boot_rand32_get();
#endif
   

#ifdef CONFIG_MULTITHREADING
 	prepare_multithreading(dummy_thread);

	switch_to_main_thread();


#else
	
	bg_thread_main(NULL, NULL, NULL);

	while (1) {
	}
#endif
     

	/*
	 * Compiler can't tell that the above routines won't return and issues
	 * a warning unless we explicitly tell it that control never gets this
	 * far.
	 */

 /*start esb_server thread*/
   

	CODE_UNREACHABLE;
}
