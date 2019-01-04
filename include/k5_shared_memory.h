/**
*******************************************************************
*  用户应用共享数据 (暂放此文件)
*******************************************************************
*/
#include <toolchain.h>
#include <kernel.h>
/*传输ESB帧结构的消息队列定义*/
struct k_msgq my_msgq;
char __aligned(4) msgq_buf[4*500];
struct k_msgq my_msgq_callback;
char __aligned(4) msgq_buf_callback[4*500];

/*用户线程pcb*/
struct k_thread client;
struct k_thread server;

/*用户事件内容*/
tK5_esb esb1;
tK5_esb esb2;

