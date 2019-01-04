/**
*******************************************************************
*  对事件队列结构进行操作的相关函数声明;
*  包括:
*     搜索search_list
*     添加insert_event
*     删除delete_event
*
*  在PCB中添加的等待事件队列数据结构声明;
*******************************************************************
*/

/**
 *事件队列数据结构
 */
typedef struct EventNode
{
    tK5_esb *esb;  //存放的数据即事件帧结构地址
    
    struct EventNode *next;  //指向下个事件结点的指针

}EventNode,*EventLinkList;


/**
 *功能: 在相应的事件队列中查找对应目标进程的事件
 *输入参数:
 *    tK5_esb *esb : 提取查找信息的事件帧地址,该参数不能为空
 *    int list_type : 所需查找的事件队列类型,仅为0(代表等待发送队列),或2(代表等待接收队列)  
 *输出参数:
 *    NULL : 输入参数有误或查找不成功
 *    event->esb : 查找成功,返回查找到的对应事件
 */
tK5_esb * search_list(tK5_esb *esb,int list_type);


/**
 *功能: 在对应目标进程的指定类型的事件队列中插入相应事件帧
 *输入参数:
 *    tK5_esb *esb : 待插入的事件帧地址,该参数不能为空
 *    tU4 list_port : 对应的目标进程id,该参数不能为空
 *    int list_type : 所需查找的事件队列类型,仅为0(代表等待发送队列),或2(代表等待接收队列)  
 *输出参数:
 *    false : 输入参数有误插入不成功
 *    true : 插入事件成功
 */
bool insert_event(tK5_esb *esb,tU4 list_port,int list_type);


/**
 *功能: 在对应目标进程的指定类型的事件队列中删除相应事件帧
 *输入参数:
 *    tK5_esb *esb : 待删除的事件帧地址,该参数不能为空
 *    tU4 list_port : 对应的目标进程id,该参数不能为空
 *    int list_type : 所需查找的事件队列类型,仅为0(代表等待发送队列),或2(代表等待接收队列)  
 *输出参数:
 *    false : 输入参数有误或待删除事件不存在使删除不成功
 *    true : 删除事件成功
 */
bool delete_event(tK5_esb *esb,tU4 list_port,int list_type);