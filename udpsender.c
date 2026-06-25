#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include "udpsender.h"

#define TERM_NORMALLY 0
#define TERM_ILLFORMED_CMDPARAMS 1
#define TERM_CANNOT_CREATE_SOCKET 2

static struct {
  useconds_t cast_interval;
  UDPSENDER_OPEMODE mode;  
  in_addr_t dst_ipaddr;
  in_addr_t emit_nicaddr;
  unsigned short dstport;
  unsigned int seq;
} udpsender_cond = { DEFALUT_SEND_INTERVAL };

static void show_banner ( void ) {
  printf( "usage: udpsender [-u] DST_HOST_IPADDR UDP_SEND_DSTPORT [CAST_INTERVAL]\n" );
  printf( " DST_HOST_IPADDR: The IP addr assigned to the NIC on target host.\n" );
  printf( " UDP_CAST_DSTPORT: The UDP desination port to be unicasted by this program.\n" );
  printf( " [CAST_INTERVAL]: Interval for UDP unicast in msec, greater_than/equal_to 100 & lesser_than 1000.\n" );
  printf( "\n" );
  printf( "usage: udpsender -m EMIT_NIC_IPADDR DST_MCAST_IPADDR UDP_DESTPORT [CAST_INTERVAL]\n" );
  printf( " EMIT_NIC_IPADDR: The IP addr assigned on the NIC for UDP MULTI-cast packets emission.\n" );
  printf( " DST_MCAST_IPADDR: The detination MULTI-cast addr for emission.\n" );
  printf( " UDP_DESTPORT: The desination UDP/IP port to be casted.\n" );
  printf( " [CAST_INTERVAL]: Interval for MULTI-cast in msec, greater_than/equal_to 100 & lesser_than 1000.\n" );
}

static BOOL exam_cmdopts ( int argc, char **argv ) {
  BOOL r = FALSE;
  
  if( (argc >= 3) && (argc <= 6) ) {
    if( strcmp( argv[1], "-m" ) == 0 ) { /* MODE_MULTICAST */
      if( argc >= 5 ) {
	udpsender_cond.mode = MODE_MULTICAST;
	udpsender_cond.emit_nicaddr = inet_addr( argv[2] );
	if( !(udpsender_cond.emit_nicaddr < 0) ) {
	  udpsender_cond.dst_ipaddr = inet_addr( argv[3] );
	  if( !(udpsender_cond.dst_ipaddr < 0) ) {
	    BOOL acc_dstport = FALSE;
	    acc_dstport = par_portnum( &udpsender_cond.dstport, argv[4], "UDP_MULTICAST_DSTPORT:" );
	    if( acc_dstport ) {	      
	      if( argc > 5 ) {
		int interval = -1;
		interval = atoi( argv[5] );
		if( (interval >= 100) && (interval < 1000) ) {
		  udpsender_cond.cast_interval = interval * 1000;
		  r = TRUE;
		} else
		  printf( "CAST_INTERVAL excesses range.\n" );
	      } else
		r = TRUE;
	    }
	  } else
	    printf( "invalid DST_MCAST_IPADDR.\n" );
	} else
	  printf( "invalid EMIT_NIC_IPADDR.\n" );
      } else
	show_banner();
    } else { /* MODE_UDP_UNICAST */
      char *arg_1 = NULL; // DST_HOST_IPADDR
      char *arg_2 = NULL; // UDP_CAST_DSTPORT
      char *arg_3 = NULL; // CAST_INTERVAL
      if( strcmp( argv[1], "-u" ) == 0 ) {
	if( argc <= 5 ) {
	  if( argc >= 4 ) {
	    arg_1 = argv[2];
	    arg_2 = argv[3];
	    if( argc > 4 ) {
	      assert( argc == 5 );
	      arg_3 = argv[4];
	    }
	    goto mode_udp_unicast;
	  } else
	    show_banner();
	} else
	  show_banner();
      } else {
	if( argc <= 4 ) {
	  arg_1 = argv[1];
	  arg_2 = argv[2];
	  if( argc == 4 )
	    arg_3 = argv[3];
	mode_udp_unicast:
	  udpsender_cond.mode = MODE_UDP_UNICAST;
	  udpsender_cond.dst_ipaddr = inet_addr( arg_1 );
	  if( !(udpsender_cond.dst_ipaddr < 0) ) {
	    BOOL acc_dstport = FALSE;
	    acc_dstport = par_portnum( &udpsender_cond.dstport, arg_2, "UDP_UNICAST_DSTPORT:" );
	    if( acc_dstport ) {
	      if( arg_3 ) {
		int interval = -1;
		interval = atoi( arg_3 );
		if( (interval >= 100) && (interval < 1000) ) {
		  udpsender_cond.cast_interval = interval * 1000;
		  r = TRUE;
		} else
		  printf( "CAST_INTERVAL excesses range.\n" );
	      } else
		r = TRUE;
	    }
	  } else
	    printf( "invalid DST_HOST_IPADDR.\n" );
	} else
	  show_banner();
      }
    }
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
  assert( udpsender_cond.mode != MODE_UNKNOWN );
  
  sock = socket( AF_INET, SOCK_DGRAM, 0 );
  if( sock < 0 ) {
    printf( "failed to create socket for sent, giving-up.\n" );
    exit( TERM_CANNOT_CREATE_SOCKET );
  }
  assert( udpsender_cond.dstport > 0 );
  assert( udpsender_cond.dst_ipaddr != 0 );
  addr.sin_family = AF_INET;
  addr.sin_port = htons( udpsender_cond.dstport );
  addr.sin_addr.s_addr = udpsender_cond.dst_ipaddr;
  if( udpsender_cond.mode == MODE_MULTICAST ) {
    int r_optmod = -1;
    assert( udpsender_cond.emit_nicaddr != 0 );
    r_optmod = setsockopt( sock, IPPROTO_IP, IP_MULTICAST_IF,
			   (char *)&udpsender_cond.emit_nicaddr, sizeof(udpsender_cond.emit_nicaddr) );
    if( r_optmod < 0 ) {
      printf( "failed to specify the NIC for UDP MULTI-casting, giving up.\n" );
      exit( -1 );
    }
  }
  
  FOREVER {
    int data_size = -1;
    int sent_bytes = -1;
    data_size = edit_sentdata();
    assert( data_size > -1 );
    sent_bytes = sendto( sock, payload_buf, data_size, 0, (struct sockaddr *)&addr, sizeof(addr) );
    if( sent_bytes < 0 )
      printf( "faled to send the UDP packet, :-P\n" );
    else
      printf( "seq:%u, sent %d [bytes].\n", udpsender_cond.seq, sent_bytes );
    udpsender_cond.seq++;
    usleep( udpsender_cond.cast_interval );
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
    *p = htonl( udpsender_cond.seq );
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
