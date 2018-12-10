//-----------------------------------------------------------------
// 传输ESB帧结构的消息队列定义
//-----------------------------------------------------------------
#include <toolchain.h>
#include <kernel.h>
struct k_msgq my_msgq;
char __aligned(4) msgq_buf[8*500];
struct k_msgq my_msgq_callback;
char __aligned(4) msgq_buf_callback[8*500];

struct k_thread client;
struct k_thread server;
tK5_esb esb1;
tK5_esb esb2;

