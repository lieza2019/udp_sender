#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include "udpscat.h"

#define TERM_NORMALLY 0
#define TERM_ILLFORMED_CMDPARAMS 1
#define TERM_CANNOT_CREATE_SOCKET 2

static struct {
  useconds_t cast_interval;
  in_addr_t dst_ipaddr;
  unsigned short dstport;
  unsigned int seq;
} udpscat_cond = { DEFALUT_SEND_INTERVAL };

static void show_banner ( void ) {
  printf( "usage: udpscat DST_HOST_IPADDR UDP_SEND_DSTPORT [CAST_INTERVAL]\n" );
  printf( " DST_HOST_IPADDR: The IP addr assigned to the NIC on target host.\n" );
  printf( " UDP_CAST_DSTPORT: The UDP desination port to be unicasted by this program.\n" );
  printf( " [CAST_INTERVAL]: Interval for UDP unicast in msec, greater_than/equal_to 100 & lesser_than 1000.\n" );
}

static BOOL exam_cmdopts ( int argc, char **argv ) {
  BOOL r = FALSE;
  
  if( (argc >= 3) && (argc <= 4) ) {
    udpscat_cond.dst_ipaddr = inet_addr( argv[1] );
    if( !(udpscat_cond.dst_ipaddr < 0) ) {
      BOOL acc_dstport = FALSE;
      acc_dstport = par_portnum( &udpscat_cond.dstport, argv[2], "UDP_UNICAST_DSTPORT:" );
      if( acc_dstport ) {
	if( argc > 3 ) {
	  int interval = -1;
	  assert( argc == 4 );
	  interval = atoi( argv[3] );
	  if( (interval >= 100) && (interval < 1000) ) {
	    udpscat_cond.cast_interval = interval * 1000;
	    r = TRUE;
	  } else
	    printf( "CAST_INTERVAL excesses range.\n" );
	} else {
	  assert( argc == 3 );
	  r = TRUE;
	}
      }
    } else
      printf( "invalid DST_HOST_IPADDR.\n" );
  } else
    show_banner();
  return r;
}

static unsigned char payload_buf[UDP_PAYLOAD_EDITBUF];
static int edit_sentdata ( void );
int main ( int argc, char **argv ) {
  struct sockaddr_in addr;
  int sock;
  
  if( ! exam_cmdopts( argc, argv ) )
    exit( TERM_ILLFORMED_CMDPARAMS );
  
  sock = socket( AF_INET, SOCK_DGRAM, 0 );
  if( sock < 0 ) {
    printf( "failed to create socket for sent, giving-up.\n" );
    exit( TERM_CANNOT_CREATE_SOCKET );
  }
  assert( udpscat_cond.dst_ipaddr != 0 );
  assert( udpscat_cond.dstport > 0 );
  addr.sin_family = AF_INET;
  addr.sin_port = htons( udpscat_cond.dstport );
  addr.sin_addr.s_addr = udpscat_cond.dst_ipaddr;
  
  FOREVER {
    int data_size = -1;
    int sent_bytes = -1;
    data_size = edit_sentdata();
    assert( data_size > -1 );
    sent_bytes = sendto( sock, payload_buf, data_size, 0, (struct sockaddr *)&addr, sizeof(addr) );
    if( sent_bytes < 0 )
      printf( "faled to send the UDP packet, :-P\n" );
    else
      printf( "seq:%u, sent %d [bytes].\n", udpscat_cond.seq, sent_bytes );
    udpscat_cond.seq++;
    usleep( udpscat_cond.cast_interval );
  }
  close( sock );
  return TERM_NORMALLY;
}

static int edit_sentdata ( void ) {
  unsigned char *plim = &payload_buf[UDP_PAYLOAD_EDITBUF];
  unsigned char *pw = payload_buf;
  int wrote_len = 0;
  
  {
    in_addr_t *p = (in_addr_t *)pw;
    *p = htonl( udpscat_cond.seq );
    pw += sizeof( in_addr_t );
  }
  while( pw < plim ) {
    *pw = 0xFF;
    pw++;
  }
  assert( pw == plim );
  wrote_len = UDP_PAYLOAD_EDITBUF;
  return wrote_len;
}
