//////////////////////////////////////////////////////////////////////////
//
// k5_esb.h
//
// Micro Kernel OS K5 微内核操作系统公用数据结构定义 
//
///////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// 事件服务总线基础常数定义
//-----------------------------------------------------------------
//常用数据类型定义；
typedef            char    tI1;   //有符号1字节整型数 
typedef  unsigned  char    tU1;	    //无符号1字节整型数 
typedef            short   tI2;   //有符号2字节整型数
typedef  unsigned  short   tU2;   //无符号2字节整型数
typedef            int     tI4;   //有符号4字节整型数
typedef  unsigned  int     tU4;   //无符号4字节整型数
typedef            long long   tI8;   //有符号8字节整型数
typedef  unsigned  long long   tU8;   //无符号8字节整型数
typedef            float   tF4;   //4字节浮点数
typedef            double  tF8;   //8字节浮点数

//事件服务总线帧头类型和6级网络定义（3比特）；2018-10-10
#define   K5_H0  0   //0型帧，头部1个八字节；
#define   K5_H1  1   //1型帧，头部2个八字节；
#define   K5_N1  2   //1级网，头部3个八字节；
#define   K5_N2  3   //2级网，头部4个八字节；
#define   K5_N3  4   //3级网，头部5个八字节；
#define   K5_N4  5   //4级网，头部6个八字节；  
#define   K5_N5  6   //5级网，头部7个八字节； 
#define   K5_N6  7   //6级网，头部8个八字节； 

//服务原语primitive代码定义（2比特）； 2018-09-05
#define   K5_CALL    0  //服务原语：00：同步请求调用；
#define   K5_REPLY   1  //服务原语：01：异步应答；
#define   K5_SEND    2  //服务原语：10：异步发送；
#define   K5_WAIT    3  //服务原语：11：同步等待接收；


//内存页面典型尺寸定义 （可以增加）；  2018-09-05
#define  PAGE_4KB    0X00000fff+1  //最小页面字节数(4KB,12bits)
#define  PAGE_64KB   0X0000ffff+1  //小页面字节数(64KB,16bits)
#define  PAGE_1MB    0X000fffff+1  //中页面字节数(1MB,20bits)
#define  PAGE_16MB   0X00ffffff+1  //大页面字节数(16MB,24bits)

#define   K5_BPN    1                   //发送缓冲区页面数  
#define   BUF_SIZE  K5_BPN*PAGE_4KB     //缺省缓冲区长度  
#define   DAT_SIZE  BUF_SIZE-8*8        //缺省数据区长度  

#define   K5_EH1      0                 //扩展头起始索引号  
#define   K5_EHN      1                 //扩展网络地址起始索引号  

#define   K5_MAX_BUF  4096              //最大缓冲区长度，1页  
#define   K5_MAX_BODY  511              //最大数据体长度，1页 
#define   K5_ESB_PAGE  512              //最大数据体长度，1页 

#define   K5_KERNEL    0                //微内核服务（虚拟）线程标识
#define   K5_PROXY     1                //网络代理服务线程标识
#define   K5_SYSTEM    2                //操作系统（虚拟）服务线程标识
#define   K5_USER     30                //用户服务代理进程（暂定30）                    

#define K5_RUNNING		3

#define K5_ERR_INIT		-1
#define K5_ERR_WAIT		-2
#define K5_NO_SERVER	-3
#define K5_NO_ACCESS	-4


#define KK_ERR_INIT     -1
#define KK_ERR_WAIT     -2
#define KK_NO_SERVER    -3
#define KK_NO_ACCESS    -4
 
//-----------------------------------------------------------------
// 事件服务总线头部结构和帧结构描述，建议本次测试以此为基础。
//-----------------------------------------------------------------


//事件服务总线帧结构描述：esb, 全帧；2018-09-08 （最基本最常用）
typedef struct {          
    tU4  head:3;        //帧头扩展长度，0~7：扩展的8字节长字个数;
    tU4  page:1;        //长度按页计数，0:按8字节，1:按4KB页面计
    tU4  size:12;       //帧总长度，以8字节或4KB页面为单位计数;
    tU4  service:14;    //服务编码，空间3b、类型4b、动作4b、参数3b; 
    tU4  primitive:2;   //服务原语，0:请求，1:确认，2:否认，3：等待
    tU2  dst_port;      //接收进程线程标识，或文件描述符，或网络套接字;
    tU2  src_port;      //发送进程线程标识，或文件描述符，或网络套接字;
    tU8  body[K5_MAX_BODY]; //帧体，扩展头部及所带数据，8字节数组；
}   tK5_esb;            //ESB帧结构


//事件服务总线帧头基本结构描述：eh0,仅基本8字节；2018-09-08（选用）
typedef struct {          
    tU4  head:3;      //帧头扩展长度，0~7：扩展的8字节长字个数;
    tU4  page:1;      //长度按页计数，0:按8字节，1:按4KB页面计
    tU4  size:12;     //帧总长度，以8字节或4KB页面为单位计数;
    tU4  service:14;  //服务编码，空间3b、类型4b、动作4b、参数3b; 
    tU4  primitive:2; //服务原语，0:请求，1:确认，2:否认，3：等待
    tU2  dst_port;    //接收进程线程标识，或文件描述符，或网络套接字;
    tU2  src_port;    //发送进程线程标识，或文件描述符，或网络套接字;
}   tK5_eh0;          //ESB帧头基本结构，用于映射事件服务寄存器；


//事件服务总线头部扩展结构：eh1, 仅扩展8字节；2018-09-08（推荐）
typedef struct {      
    tU2  snd_seq;     //发送帧序列号,与IPv4兼容；
    tU2  ack_seq;     //确认或否认收到的帧序列号，或错误码；
    tU4  hops:8;      //网络路由跳数，与IPv4和IPv6兼容；
    tU4  qos:8;       //服务质量标识，与IPv4和IPv6兼容；
    tU4  protocol:8;  //上层协议标识，与IPv4和IPv6兼容；
    tU4  endian:1;    //端点标识，0:小端点,1:大端点;仅用于网络;
    tU4  spare:7;     //留作扩展;
}   tK5_eh1;          //ESB帧头扩展结构，用于cast映射body[1];


//事件服务总线头部网络地址扩展：ehn，仅地址8字节；2018-09-08 （推荐）
typedef struct {      
    tU4  dst_addr;    //接收侧目的网络地址;
    tU4  src_addr;    //发送侧源端网络地址; 
}   tK5_ehn;          //扩展网络地址，用于cast映射body[2~7];


// 注：帧体body[]的其他长字，可携带服务参数或结果数据，由服务提供者定义；


//-----------------------------------------------------------------
// 服务原语相关参数描述，建议本次测试以此为基础。2018-09-06
//-----------------------------------------------------------------

#define  K5_MAX_NET_LEVEL  6   //最大网络级数
#define  K5_MAX_NET_NAME  64   //最大网络名称字符数

//事件服务总线网络地址描述 net, 2018-09-06
typedef struct {        
  tU1      net_level;              //网络级数，1为本处理器，直到7，共6级；
  tU1      cvt_level;              //已将名称转换为二进制的网络级数（按位）
  tU2       name_len;              //网络名称字符串总长度
  tU2       dst_port;              //目的端口号，可为：pid、fd、sock；
  tU2       src_port;              //源侧端口号，可为：pid、fd、sock；
  tK5_ehn   hn[K5_MAX_NET_LEVEL];  //ESB头部网络地址扩展结构，6*8字节
  tU1  net_name[K5_MAX_NET_NAME];  //字符串描述的网络名称，首次输入
} tK5_net;                         //ESB网络地址描述 

//------------------------------------------------------------
// 端口port类型：利用两个字节端口号的最高2比特(15和14)加以区分
//
//   bit 15  14
//        0   0  ：表示线程标识符, PID;  最多运行约1.5万个线程；
//        0   1  ：表示文件描述符，FD;   可同时打开约1.5万个文件；
//        1   0  ：表示网络套接字，SOCK; 可同时建立约1.5万个连接；
//        1   1  ：表示设备标识码，DEV;  
//
//------------------------------------------------------------
// k5服务编码的细部结构：  service总共占14比特，包括4个部分
//
// bit  13 12 11     10 9 8      7 6 5 4      3 2 1 0  
//      服务空间     参数个数    服务类型     服务动作    
//      space        pnum        type         act       
//      系统空间     服务定义    现在11类     每类16个    
//      用户空间     向量内含    需要优化     需要优化      
//------------------------------------------------------------

//服务编码展开结构：svc, 仅14比特；2018-09-08（可选）
typedef struct {          
    tU4  svc_func:4;   //服务功能：每类服务可有16个功能，需优化；
    tU4  svc_type:4;   //服务类型：目前定义11类，需优化；
    tU4  svc_pnum:3;   //服务参数个数： 在服务向量中定义；
    tU4  svc_inout:1;  //服务参数方向：0输入、1输出；2018-10-03增加； 
    tU4  svc_space:2;  //服务领域空间：0系统、1及以上用户； 
    tU4  primitive:2;  //服务原语，0:请求，1:确认，2:否认，3：等待
    tU4  spare:16;     //备用
}   tK5_svc;           //服务码展开，服务原语用；

// 原语k5_call中整型服务参数展开结构描述，pint（2018-09-09）未用？？
typedef struct {          
    tU4  pi4u;    //第一个通用4字节整型参数；
    tU2  p_num;   //该服务的参数个数；
    tU2  p_len;   //后面串型参数字符串的总字节数;
}   tK5_pint;     //整型参数展开结构，tk5_call原语用

// 原语k5_ack返送服务结果的通用动态数据结构，gdd（2018-09-08）
typedef struct {          
    tU1  type;    //通用数据类型；
    tU1  size;    //该类数据单元长度
    tU1  num;     //该类数据单元个数
    tU1  name;    //数据名称长度（按字节计数）;
    tU1  body[];  //数据名称按4字节对齐，填充0，后跟数据本体;
}   tK5_gdd;      //gdd总长度 = 单元长*个数 + 名长 + 头长4字节


//-------------------------------------------------------------------------
//系统服务原语函数接口原型（最新版）, 建议本次测试以此为基础。2018-09-06
//-------------------------------------------------------------------------
//几点说明：
//注1：原来的四个原语（同步发送SS、同步接收RS、异步发送SA、异步接收RA），
//     逻辑上是完备的，但名称太抽象，且原来四个参数不完备，需增数据体描述。
//注2：将原来的两个异步原语换为意义更为明确的异步确认和异步否认，主要是为了
//     配合零拷贝技术，需要尽快确认以便释放送端缓冲区，也使接收端继续等待。
//注3：该服务原语有两种用法，一是供新开发的应用程序直接调用，二是由新CLIB
//     中POSIX系统服务函数间接调用，原应用程序不需修改，但不支持K5新功能。
//注4  为四原语设置两个通用参数（一个整型、一串型），便于原语组装ESB帧，
//     若还需传递其他参数，可直接通过ESB数据结构传递。 2018-09-05增加的
//     各个系统服务的特定参数结构不再需要了，使用更为方便。2018-09-06.
//注5：在ESB帧结构统一改为8字节数组后（2018-09-08），所有原语（k5_call、
//      k5_ack和k5_nak）可以使用单一esb帧体结构，当需要网络地址、序列号、
//     错误号的时候，可直接用tK5_eh1和tK5_ehn映射（cast）到body[]的相应
//     位置，并通过frame头部扩展长度指明。（2018-09-08修改）
//注6：在服务编码中增加该服务的参数个数，占3比特，需要由服务提供者在服务注
//     册时提供，在服务向量表中保存。（2018-09-08）
//注7：服务原语为各类服务预留两个通用参数位置：1个整型参数和1个字符串型
//     参数；当参数为线程标识pid、文件描述符fd、网络套接字sock时，可直接
//     用ESB目的端口dst_port传递，不占用整型参数位置（8字节，好对齐）。
//    （2018-09-08）
//注8：当两个通用参数位置不够时，可以通过字符串型通用参数进行扩展，将多个
//     字符串合并为一字符串，之间用空格分隔；当用字符串表示十进制或十六
//     进制数时，其首为必须为数字；在一个字符串中描述的参数个数，须与服务
//     向量表中的参数个数相匹配。（2018-09-08）
//注9：服务结果及返回数据的结构格式，由服务提供方确定，可采用静态和动态
//     方式。静态方式需要预先定义数据结构，效率很高，但不灵活；动态方式
//     采用tK5_gdd动态描述复杂数据结构，很灵活，效率低。（2018-09-08）
//注10：将所有原语都需要的参数esb,固定为第1个参数，最后两个参数固定为
//      整型长度、缓冲区地址指针，便于记忆 。  （2018-09-11）
//-------------------------------------------------------------------------
//用户态服务原语，k5_*, 通过软中断陷入内核
//-------------------------------------------------------------------------
// 用户态同步调用请求原语（原SS），一直等待对方应答后，才释放发送缓冲区
tU4  k5_call (         //5个参数     
     tK5_esb   *esb,   //ESB总线数据结构（改为第1个参数）
     tU2    service,   //服务编码
     tK5_net    *to,   //目的端口号（pid,fd,sock）及网络地址  
     tU4      c_len,   //同步调用原语的缓冲区长度（按8字节）；
     tU4     *c_buf    //同步调用原语的缓冲区地址；
); 

//用户态同步等待接收原语（原RS)，等待接收服务请求或服务确认;
tU4  k5_wait  (       //4个参数，不需要等待特定服务编码 ；   
     tK5_esb   *esb,  //ESB总线帧数据结构（改为第1个参数）
     tK5_net  *from,  //期望等待的端口及网址，NULL为ANY 
     tU4      w_len,  //等待接收缓冲区长度，按8字节计，含帧头；
     tU4     *w_buf   //等待接收缓冲区地址，使用零拷贝时不用；
); 

//用户异步应答原语（原SA，ack+nak)，确认服务请求，返回服务结果，然后继续等待;
tU4  k5_reply  (     //4个参数，原路返回，不需要地址；
     tK5_esb  *esb,  //ESB总线帧数据结构（改为第1个参数）
     tI2   ack_err,  //正值时为确认序列号，负值表示错误编号；
     tU4     s_len,  //发送缓冲区长度,按8字节计，含帧头；
     tU4    *s_buf   //发送缓冲区地址,带服务结果数据；
); 


//用户态异步发送原语（原SA)，主动发送数据或通知，然后继续等待;
tU4  k5_send  (      //5个参数; 取消nak原语后，恢复异步发送原语send;
     tK5_esb   *esb, //ESB总线帧数据结构（改为第1个参数）
     tU2    service, //服务编码
     tK5_net    *to, //目的端口号（pid,fd,sock）及网络地址  
     tU4      s_len, //发送缓冲区长度,按8字节计，含帧头；
     tU4     *s_buf  //发送缓冲区地址,主动发送的数据或订阅的数据；
); 

//--------------------------------------------------------------
//内核态服务原语，kk_*, 直接切换到相应服务线程
//--------------------------------------------------------------

// 内核态同步调用请求原语（原SS），一直等待对方应答后，才释放发送缓冲区
tU4  kk_call (         //5个参数     
     tK5_esb   *esb,   //ESB总线数据结构（改为第1个参数）
     tU2    service,   //服务编码
     tK5_net    *to,   //目的端口号（pid,fd,sock）及网络地址  
     tU4      c_len,   //同步调用原语的缓冲区长度（按8字节）；
     tU1     *c_buf    //同步调用原语的缓冲区地址；
); 

//内核态同步等待接收原语（原RS)，等待接收服务请求或服务确认;
tU4  kk_wait  (       //4个参数，不需要等待特定服务编码 ；   
     tK5_esb   *esb,  //ESB总线帧数据结构（改为第1个参数）
     tK5_net  *from,  //期望等待的端口及网址，NULL为ANY 
     tU4      w_len,  //等待接收缓冲区长度，按8字节计，含帧头；
     tU1     *w_buf   //等待接收缓冲区地址，使用零拷贝时不用；
); 

//内核异步应答原语（原SA)，确认服务请求，返回服务结果，然后继续等待;
tU4  kk_reply  (     //4个参数，原路返回，不需要地址；
     tK5_esb  *esb,  //ESB总线帧数据结构（改为第1个参数）
     tU2   ack_err,  //正值时为确认序列号，负值表示错误编号；
     tU4     s_len,  //发送缓冲区长度,按8字节计，含帧头；
     tU1    *s_buf   //发送缓冲区地址,带服务结果数据；
); 


//内核态异步发送原语（原SA)，主动发送数据或通知，然后继续等待;
tU4  kk_send  (      //5个参数; 取消nak原语后，恢复异步发送原语send;
     tK5_esb   *esb, //ESB总线帧数据结构（改为第1个参数）
     tU2    service, //服务编码
     tK5_net    *to, //目的端口号（pid,fd,sock）及网络地址  
     tU4      s_len, //发送缓冲区长度,按8字节计，含帧头；
     tU1     *s_buf  //发送缓冲区地址,主动发送的数据或订阅的数据；
); 

//--------------------------------------------------------------
//内核态原语用的小工具，kk_*
//--------------------------------------------------------------
//内核态按八字节长字的内存设置，比按字节的内存设置memset快8倍;
void kk_mem_set  ( 
     tU8     memory,  //内存起始地址（八字节对齐）
     tU8      value,  //需要设置的整型数值
     tU4       size   //以8字节为单位的长度 
     );

//内核态按八字节长字的内存拷贝，比按字节的内存拷贝memncpy快8倍;
void kk_mem_copy  ( 
     tU8     to_mem,  //目的内存起始地址（八字节对齐）
     tU4       size, //以8字节为单位的长度 
     tU8   from_mem //源端内存起始地址（八字节对齐）
     );

//内核态查找参数字符串中的零分隔符位置
tU4  kk_find_zero  ( 
     tU1    *str,  //字符串起始地址
     tU4     len   //以字节为单位的长度范围 
 );


//内核态线程切换
void  kk_switch_to  ( 
     tU2    curr_pid,  //当前线程标识
     tU2    next_pid,  //后续线程标识
     tK5_esb    //传递参数
 );

//--------------------------------------------------------------
//用户态原语用的小工具，k5_*
//--------------------------------------------------------------

//用户态按八字节长字设置内存，比按字节的内存设置memset快8倍;
void k5_mem_set  ( 
     tU8    *memory,  //内存起始地址（八字节对齐）
     tU8      value,  //需要设置的整型数值
     tU4       size   //以8字节为单位的长度 
     );

//用户态按八字节长字的内存拷贝，比按字节的内存拷贝memncpy快8倍;
void k5_mem_copy  ( 
     tU8    *to_mem,  //目的内存起始地址（八字节对齐）
     tU4       size,  //以8字节为单位的长度 
     tU8  *from_mem  //源端内存起始地址（八字节对齐）
     );


//用户态查找参数字符串中的零分隔符位置
tU4  k5_find_zero  ( 
     tU1    *str,  //字符串起始地址
     tU4     len   //以字节为单位的长度范围 
 );

//用户态打印事件服务总线头部及数据
void k5_print_esb (
      tK5_esb *esb,  //事件服务总线帧结构
      tU2     deep   //打印深度，按八字节长字计数，单页最大512；
);

//-------------------------------------------------------------------------
//服务向量表数据结构和服务编码定义。待进一步讨论。 2018-05-23
//-------------------------------------------------------------------------

//服务组进程向量2维表,小表，从服务组号查找，用于线程快速切换；
typedef struct {          
    tU2   pid;         //提供服务的进程或线程的标识PID；
    tU1   state;       //该组服务进程运行状态;
    tU1   snum;        //每组内的服务器个数（第1列有效）;
}   tK5_ServiceType;   //服务类型组(STG)，服务类型X本类服务器个数；

#define  K5_MAX_STG 16//最大服务类组表，4比特  
#define  K5_NUM_STG 4 //同组最大服务器数，4比特  


//服务访问控制表,用于从服务号查找，用于每个服务的访问控制；
typedef struct {          
    tU1  subject;      //主体权限认证级别；
    tU1  object;       //客体访问控制级别；
}   tK5_ServiceVector; //服务向量表(SVC)；

#define  K5_MAX_SVC 256 //最大服务向量表，1个字节  

//服务向量配置表
//< K5_OS_Service  group=11    service=110    max_service=256    >
#define  sys_svc_num      0x0010     //系统System                13           
#define  sys_reboot       0x0011     //系统_重启                  1           
#define  sys_halt         0x0012     //系统_停机                  1           
#define  sys_start_vmm    0x0013     //系统_启动虚拟机            1           
#define  sys_stop_vmm     0x0014     //系统_停止虚拟机            1           
#define  sys_mount_fs     0x0015     //系统_安装文件系统          1           
#define  sys_unmount_fs   0x0016     //系统_卸载文件系统          1           
#define  sys_login        0x0017     //系统_登录系统              1           
#define  sys_logout       0x0018     //系统_退出系统              1           
#define  sys_svc_grp_reg  0x0019     //系统_注册服务组            1           
#define  sys_svc_grp_del  0x001a     //系统_删除服务组            1           
#define  sys_svc_reg      0x001b     //系统_注册服务              1           
#define  sys_svc_del      0x001c     //系统_删除服务              1           
#define  sys_svc_list     0x001d     //系统_服务列表              0           

#define  prc_svc_num      0x0020     //进程Process                8           
#define  prc_start        0x0021     //进程_启动                  1           
#define  prc_stop         0x0022     //进程_停止                  1           
#define  prc_fork         0x0023     //进程_分叉                  1           
#define  prc_exit         0x0024     //进程_退出                  1           
#define  prc_pause        0x0025     //进程_暂停                  1           
#define  prc_resume       0x0026     //进程_继续                  1           
#define  prc_sync         0x0027     //进程_同步                  1           
#define  prc_wait         0x0028     //进程_等待                  1           

#define  thr_svc_num      0x0030     //线程Thread                 7           
#define  thr_start        0x0031     //线程_启动                  1           
#define  thr_stop         0x0032     //线程_停止                  1           
#define  thr_exit         0x0033     //线程_退出                  1           
#define  thr_pause        0x0034     //线程_暂停                  1           
#define  thr_resume       0x0035     //线程_继续                  1           
#define  thr_sync         0x0036     //线程_同步                  1           
#define  thr_wait         0x0037     //线程_等待                  1           

#define  file_svc_num     0x0040     //文件File                  15           
#define  file_create      0x0041     //文件_创建                  1           
#define  file_delete      0x0042     //文件_删除                  1           
#define  file_open        0x0a43     //文件_打开                  1           
#define  file_close       0x0144     //文件_关闭                  1           
#define  file_read        0x0445     //文件_读出                  0           
#define  file_write       0x0c46     //文件_写入                  0           
#define  file_seek        0x0047     //文件_定位                  0           
#define  file_rewind      0x0048     //文件_倒回                  0           
#define  file_state       0x0049     //文件_状态                  0           
#define  file_rename      0x004a     //文件_改名                  0           
#define  file_link        0x004b     //文件_连接                  1           
#define  file_unlink      0x004c     //文件_解连                  1           
#define  file_set_mode    0x004d     //文件_设置访问模式          1           
#define  file_get_mode    0x004e     //文件_获取访问模式          0           
#define  file_copy        0x004f     //文件_拷贝，还有另存为、移动等 

#define  dir_svc_num      0x0050     //目录Director              11           
#define  dir_make         0x0051     //目录_创建                  1           
#define  dir_remove       0x0052     //目录_删除                  1           
#define  dir_list         0x0053     //目录_列表                  0           
#define  dir_change       0x0054     //目录_切换                  0           
#define  dir_rename       0x0055     //目录_改名                  1           
#define  dir_link         0x0056     //目录_连接                  1           
#define  dir_unlink       0x0057     //目录_解连                  1           
#define  dir_set_owner    0x0058     //目录_设置拥有者            1           
#define  dir_get_owner    0x0059     //目录_获取拥有者            0           
#define  dir_set_mode     0x005a     //目录_设置访问模式          1           
#define  dir_get_mode     0x005b     //目录_获取访问模式          0           

#define  usr_svc_num      0x0060     //用户User                   7           
#define  usr_create       0x0061     //用户_创建                  1           
#define  usr_delete       0x0062     //用户_删除                  1           
#define  usr_get          0x0063     //用户_获取                  1           
#define  usr_set          0x0064     //用户_设置                  1           
#define  usr_get_uid      0x0065     //用户_获取标识              0           
#define  usr_set_uid      0x0066     //用户_设置标识              1           
#define  usr_check        0x0067     //用户_检查                  1           

#define  grp_svc_num      0x0070     //用户组Group                7           
#define  grp_create       0x0071     //用户组_创建                1           
#define  grp_delete       0x0072     //用户组_删除                1           
#define  grp_get          0x0073     //用户组_获取                0           
#define  grp_set          0x0074     //用户组_设置                1           
#define  grp_get_gid      0x0075     //用户组_获取标识            0           
#define  grp_set_gip      0x0076     //用户组_设置标识            1           
#define  grp_check        0x0077     //用户组_检查                1           

#define  mem_svc_num      0x0080     //内存Memory                13           
#define  mem_map          0x0081     //内存_映射                  1           
#define  mem_unmap        0x0082     //内存_解除映射              1           
#define  mem_alloc        0x0083     //内存_申请                  0           
#define  mem_free         0x0084     //内存_释放                  0           
#define  mem_share        0x0085     //内存_共享                  1           
#define  mem_unshare      0x0086     //内存_解除共享              1           
#define  mem_lock         0x0087     //内存_锁定                  0           
#define  mem_unlock       0x0088     //内存_解除锁定              0           
#define  mem_sem_get      0x0089     //内存_获取信号灯            0           
#define  mem_sem_set      0x008a     //内存_设置信号灯            1           
#define  mem_sem_on       0x008b     //内存_开启信号灯            0           
#define  mem_sem_off      0x008c     //内存_关闭信号灯            0           
#define  mem_control      0x008d     //内存_控制                  1           

#define  time_svc_num     0x0090     //时间Time                   6           
#define  time_get         0x0091     //时间_获取                  0            
#define  time_set         0x0092     //时间_设置                  0           
#define  time_sleep       0x0093     //时间_休眠                  0           
#define  time_wait        0x0094     //时间_等待                  0           
#define  time_alarm       0x0096     //时间_设置告警              0           
#define  time_clear       0x0097     //时间_清除告警              0           
#define  time_sync        0x0098     //时间_对时同步              1           

#define  net_svc_num      0x00a0     //网络Network               11           
#define  net_socket       0x00a1     //网络_打开套接              1           
#define  net_close        0x00a2     //网络_关闭套接              1           
#define  net_bind         0x00a3     //网络_绑定                  0           
#define  net_control      0x00a4     //网络_控制                  1           
#define  net_connect      0x00a5     //网络_请求联接              1           
#define  net_accept       0x00a6     //网络_接受联接              1           
#define  net_send         0x00a7     //网络_发送                  0           
#define  net_recv         0x00a8     //网络_接收                  0           
#define  net_send_to      0x00a9     //网络_发送到                0           
#define  net_recv_from    0x00aa     //网络_接收自                0           
#define  net_select       0x00ab     //网络_选择                  0           

#define  dev_svc_num      0x00b0     //设备Device                12           
#define  dev_start        0x00b1     //设备_启动                  1           
#define  dev_stop         0x00b2     //设备_停止                  1           
#define  dev_open         0x00b3     //设备_打开                  1           
#define  dev_close        0x00b4     //设备_关闭                  1           
#define  dev_read         0x00b5     //设备_读出                  0           
#define  dev_write        0x00b6     //设备_写入                  0           
#define  dev_state        0x00b7     //设备_状态                  0           
#define  dev_control      0x00b8     //设备_控制                  1           
#define  dev_reset        0x00b9     //设备_复位                  1           
#define  dev_sleep        0x00ba     //设备_休眠                  0           
#define  dev_set          0x00bb     //设备_设置参数              1           
#define  dev_get          0x00bc     //设备_获得参数              0           

#define  K5_LAST_SVC      0x00bc     //实际最大服务编码  

//待增加常用设备服务，如打印机、显示器、键盘、鼠标、硬盘、U盘、照相机
//以太网、WiFi、无线通信、4G/5G等。

//-------------------------------------------------------------------------
//主体功能权能描述和课题访问分类权能描述，待讨论。 2018-06-24
//-------------------------------------------------------------------------

//主体功能权能描述：32比特；2018-06-24
typedef struct {          
    tU4  system:1;     //0：系统服务，需要特权认证;
    tU4  process:1;    //1：进程服务，需要特权认证;
    tU4  thread:1;     //2：线程服务，需要特权认证;
    tU4  fileCtl:1;    //3：文件控制服务，需要特权认证;
    tU4  fileOpr:1;    //4：文件操作服务，不需要认证;
    tU4  dirCtl:1;     //5：目录控制服务，需要特权认证;
    tU4  dirOpr:1;     //6：目录操作服务，不需要认证;
    tU4  user:1;       //7：用户服务，需要特权认证;
    tU4  group:1;      //8：组服务，需要特权认证;
    tU4  memCtl:1;     //9：内存控制服务，需要特权认证;
    tU4  memOpr:1;     //10：内存操作服务，不需要认证;
    tU4  timeCtl:1;    //11：时间控制服务，需要特权认证;
    tU4  timeOpr:1;    //12：时间操作服务，不需要认证;
    tU4  netCtl:1;     //13：网络控制服务，需要特权认证;
    tU4  netOpr:1;     //14：网络操作服务，不需要认证;
    tU4  devCtl:1;     //15：设备控制服务，需要特权认证;
    tU4  devOpr:1;     //16：设备操作服务，不需要认证;   
    tU4  spareC:7;     //备用扩展7比特;
    tU4  blp_lev:4;    //BLP主体权能等级;2018-09-05
    tU4  biba_lev:4;   //BIBA主体权能等级;2018-09-05
}  tK5_Capibility;     //主体功能分类权能控制。

//客体访问权能描述：32比特；2018-06-24
typedef struct {          
    tU4  kernel:2;     //0：内核态访问; 00无权，01只读，10读写，11执行；
    tU4  system:2;     //1：系统管理员访问;
    tU4  security:2;   //2：安全管理员访问;
    tU4  auction:2;    //3：审计管理员访问;
    tU4  ownor:2;      //4：客体拥有者自己访问;
    tU4  group:2;      //5：本地同组内的主体访问;
    tU4  local:2;      //6：本地非同组的主体访问;
    tU4  domain:2;     //7：远程同域的主体访问;
    tU4  remote:2;     //8：远程非同域的主体访问;
    tU4  spare:6;     //9: 备用
    tU4  blp_lev:4;    //10：BLP客体访问权能等级; 2018-09-05
    tU4  biba_aev:4;   //11：BIBA客体访问权能等级;2018-09-05
}  tK5_Accessbility;   //客体角色分类访问控制。


//-------------------------------------------------------------------------
//K5微内核基础数据结构（DEMO测试用），10张管理表。供参考，待讨论。2018-09-02
//-------------------------------------------------------------------------

//内核基础数据结构K5_base；2018-09-02
typedef struct {          
    tU4  k_version;    //内核版本标识;
    tU4  k_gen_time;   //内核生成时间;
    tU4  k_code_size;  //内核程序总字节数;
    tU4  k_data_size;  //内核数据总字节数;
    tU4  k_stack_size; //内核栈总字节数;
    tU4  k_heap_size;  //内核堆总字节数;
    tU8  k_boot_addr;  //内核启动地址;

    tU2  cpu_max;      //处理器最大数
    tU2  cpu_num;      //当前处理器总数
    tU4* cpu_tab;      //处理器表指针

    tU2  mem_max;      //内存分段最大数
    tU2  mem_num;      //内存分段总数
    tU4* mem_tab;      //内存分段表指针

    tU2  pdir_max;     //页表目录项目最大数
    tU2  pdir_num;     //页表目录项目数
    tU4* pdir_tab;     //页表目录指针

    tU2  ptab_max;     //页表项目最大数
    tU2  ptab_num;     //页表项目总数
    tU4* ptab_tab;     //首个页表指针

    tU2  irq_max;      //硬件中断向量最大数
    tU2  irq_num;      //硬件中断向量实际数
    tU4* irq_tab;      //硬件中断向量表指针

    tU1  stg_max;      //软中断服务类型组最大数
    tU1  stg_num;      //软中断服务类型组实际数
    tK5_ServiceType    stg_tab[K5_MAX_STG][K5_NUM_STG]; //软中断服务类组表

    tU2  svc_max;      //软中断服务向量最大数
    tU2  svc_num;      //软中断服务向量实际数
    tK5_ServiceVector  svc_tab[K5_MAX_SVC];      //软中断服务组表

    tU2  task_max;     //任务（进程和线程）最大数
    tU2  task_num;     //任务（进程和线程）实际数
    tU4* task_tab;     //任务表指针
    tU4* task_buf;     //任务缓冲区表指针（内核态，用于零拷贝）

    tU2  fd_max;       //文件描述符最大数
    tU2  fd_num;       //文件描述符实际数
    tU4* fd_tab;       //文件描述符表指针

    tU2  sock_max;     //网络套接字最大数
    tU2  sock_num;     //网络套接字实际数
    tU4* sock_tab;     //网络套接字表指针

    tU2  dev_max;      //设备最大数
    tU2  dev_num;      //设备实际数
    tU4* dev_tab;      //设备表指针
    tU4* dev_buf;      //设备缓冲区表指针（内核态，用于零拷贝）

}  tK5_base;           //K5微内核操作系统基础数据结构。





//-----------------------------------------------------------------
// 传输ESB帧结构的消息队列定义
//-----------------------------------------------------------------
#include <toolchain.h>
#include <kernel.h>
tK5_esb *esb_bus;
struct k_msgq my_msgq;
char __aligned(8) msgq_buf[8*1000];

struct k_msgq my_msgq_back;
char __aligned(4) msgq_buf_back[8*1000];


