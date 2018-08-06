/* callback test */
#include <misc/printk.h>
#include "common.h"
#include "client_list.h"
static void connect_cb( int ctx,
      int status,
      void *user_data )
{
  printk( "enter connect_cb_t\n" );
}


static void  recv_cb( int ctx,
          int status,
          void *user_data )
{
  printk( "enter recv_cb\n" );
}


/* 必须全局变量，否则线程创建会出问题 */
static struct server  test_server;
static struct client  test_client1;
static struct client  test_client2;
static struct client  test_client3;


/* 测试client_send; */

static void init_CS()
{
  printk( "TEST\t init_CS():\n" );
  printk( "=============================\n\n" );

  server_init( &test_server, 80, connect_cb, recv_cb, NULL, NULL );
  client_init( &test_client1, 100, connect_cb, recv_cb, NULL, NULL );
  client_init( &test_client2, 101, connect_cb, recv_cb, NULL, NULL );
  client_init( &test_client3, 102, connect_cb, recv_cb, NULL, NULL );
  printk( "=============================\n\n" );

  k_sleep( 300 );
}


static void connect_server()
{
  printk( "TEST\t connect_server():\n" );
  printk( "=============================\n\n" );


  api_client_connect( 80, &test_client1 );
  api_client_connect( 80, &test_client2 );
  api_client_connect( 80, &test_client3 );
  printk( "=============================\n\n" );

  k_sleep( 300 );
}


static void release_CS()
{
  printk( "TEST\t release_CS():\n" );
  printk( "=============================\n\n" );

  api_client_release( &test_client1 );
  api_client_release( &test_client2 );
  api_client_release( &test_client3 );
  api_server_release( &test_server );
  printk( "=============================\n\n" );

  k_sleep( 300 );
}


static void send_client2server()
{
  printk( "TEST\t send_client2server():\n" );
  printk( "=============================\n\n" );

  api_client_send("hello1",&test_client1);
  api_client_send("hello2",&test_client2);
  api_client_send("hello3",&test_client3);
  printk( "=============================\n\n" );

  k_sleep( 300 );
}
static void send_server2client()
{
  printk( "TEST\t send_server2client():\n" );
  printk( "=============================\n\n" );
  add_clients(&test_server,&test_client1,100);
  add_clients(&test_server,&test_client2,101);
  //remove_client(&test_server,101);
  add_clients(&test_server,&test_client3,102);

  api_server_send(&test_server,100,MSG_DATA,"api_server_send100");
  api_server_send(&test_server,101,MSG_DATA,"api_server_send101");
  api_server_send(&test_server,102,MSG_DATA,"api_server_send102");
  struct data_item_t msg; 

  k_msgq_get( &(test_client1.recv_msgq), &msg, K_FOREVER );
  printk( "%s\n" ,msg.data); 
  k_msgq_get( &(test_client2.recv_msgq), &msg, K_FOREVER );
  printk( "%s\n" ,msg.data); 
  k_msgq_get( &(test_client3.recv_msgq), &msg, K_FOREVER );
  printk( "%s\n" ,msg.data); 


  k_sleep( 300 );
  printk( "=============================\n\n" );
}

static void disconn_client2server()
{
  printk( "TEST\t disconn_client2server():\n" );
  printk( "=============================\n\n" );

  api_client_disconn( &test_client1 );
  api_client_disconn( &test_client2 );
  printk( "=============================\n\n" );
}


/* main 入口 */
static void entry_test()
{
  init_CS();
  connect_server();
  send_client2server();

  send_server2client();
  
    
  disconn_client2server();
  release_CS();
  printk( "end\n" );
}


