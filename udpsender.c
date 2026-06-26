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
#define TERM_CANNOT_MODIFY_SOCKETOPT 3

static struct {
  useconds_t cast_interval;
  int blk_packetsnum;
  UDPSENDER_OPEMODE mode;
#if 0
  in_addr_t dst_ipaddr;
#else
  struct {
    IPADDR dest_ip;
    in_addr_t dst_addr;
  } dest;
#endif
#if 0
  in_addr_t emit_nicaddr;
#else
  struct {
    IPADDR emit_nic;
    in_addr_t emit_addr;
  } emit_mcast;
#endif
  unsigned short dstport;
  unsigned int seq;
} udpsender_cond = { DEFALUT_SEND_INTERVAL, 1 };

static void show_banner ( void ) {
  printf( "usage: udpsender [-u] DST_HOST_IPADDR UDP_SENT_DSTPORT [BULKSENT_PACKETSNUM]\n" );
  printf( "       udpsender [-u] DST_HOST_IPADDR UDP_SENT_DSTPORT BULKSENT_PACKETSNUM [CAST_INTERVAL]\n" );
  printf( " DST_HOST_IPADDR: The IP addr assigned to the NIC on target host.\n" );
  printf( " UDP_SENT_DSTPORT: The UDP desination port to be unicasted by this program.\n" );
  printf( " BULKSENT_PACKETSNUM: num of packets sent in bulk, from 1 to 10000.\n" );
  printf( " CAST_INTERVAL: Interval for UDP unicast in msec, from 100 to 999.\n" );
  printf( "\n" );
  printf( "usage: udpsender -m EMIT_NIC_IPADDR DST_MCAST_IPADDR UDP_SENT_DSTPORT [BULKSENT_PACKETSNUM]\n" );
  printf( "       udpsender -m EMIT_NIC_IPADDR DST_MCAST_IPADDR UDP_SENT_DSTPORT BULKSENT_PACKETSNUM [CAST_INTERVAL]\n" );
  printf( " EMIT_NIC_IPADDR: The IP addr assigned on the NIC for UDP MULTI-cast packets emission.\n" );
  printf( " DST_MCAST_IPADDR: The detination MULTI-cast addr for sent.\n" );
  printf( " UDP_SENT_DSTPORT: The desination UDP/IP port to be casted.\n" );
  printf( " BULKSENT_PACKETSNUM: num of packets sent in bulk, from 1 to 10000.\n" );
  printf( " CAST_INTERVAL: Interval for MULTI-cast in msec, from 100 to 999.\n" );
}

static BOOL exam_cmdopts ( int argc, char **argv ) {
  BOOL r = FALSE;
  assert( (argc > 0) && argv[0] );
  
  if( (argc >= 3) && (argc <= 7) ) {
    if( strcmp( argv[1], "-m" ) == 0 ) { /* MODE_MULTICAST */
      // udpsender -m EMIT_NIC_IPADDR DST_MCAST_IPADDR UDP_SENT_DSTPORT [BULKSENT_PACKETSNUM] [CAST_INTERVAL]
      if( argc >= 5 ) {
	udpsender_cond.mode = MODE_MULTICAST;
#if 0
	IPADDR emit_nic = {};
	par_ipaddr( &emit_nic, argv[2], "EMIT_NIC_IPADDR: " );
#else
	BOOL emit_nic_ip = FALSE;
	emit_nic_ip = par_ipaddr( &udpsender_cond.emit_mcast.emit_nic, argv[2], "EMIT_NIC_IPADDR: " );
#endif
	udpsender_cond.emit_mcast.emit_addr = inet_addr( argv[2] );
	if( emit_nic_ip && !(udpsender_cond.emit_mcast.emit_addr < 0) ) {
#if 1
	  BOOL mcast_grpaddr = FALSE;
	  mcast_grpaddr = par_ipaddr( &udpsender_cond.dest.dest_ip, argv[3], "DST_MCAST_IPADDR: " );
#endif
	  udpsender_cond.dest.dst_addr = inet_addr( argv[3] );
	  if( mcast_grpaddr && !(udpsender_cond.dest.dst_addr < 0) ) {
	    BOOL acc_dstport = FALSE;
	    acc_dstport = par_portnum( &udpsender_cond.dstport, argv[4], "UDP_SENT_DSTPORT: " );
	    if( acc_dstport ) {
	      if( argc > 5 ) {
		int nblkpackets = -1;
		nblkpackets = atoi( argv[5] );
		if( (nblkpackets > 0) && (nblkpackets <= 10000) ) {
		  udpsender_cond.blk_packetsnum = nblkpackets;
		  if( argc > 6 ) {		    
		    int interval = -1;
		    assert( argc == 7 );
		    interval = atoi( argv[6] );
		    if( (interval >= 100) && (interval < 1000) ) {
		      udpsender_cond.cast_interval = interval * 1000;
		      r = TRUE;
		    } else
		      printf( "CAST_INTERVAL excesses range.\n" );
		  } else
		    r = TRUE;
		} else
		  printf( "BULKSENT_PACKETSNUM excesses range.\n" );
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
      char *opt_1 = NULL; // DST_HOST_IPADDR
      char *opt_2 = NULL; // UDP_SENT_DSTPORT
      char *opt_3 = NULL; // BULKSENT_PACKETSNUM
      char *opt_4 = NULL; // CAST_INTERVAL
      if( strcmp( argv[1], "-u" ) == 0 ) {
	// udpsender -u DST_HOST_IPADDR UDP_SENT_DSTPORT [BULKSENT_PACKETSNUM] [CAST_INTERVAL]
	if( argc <= 6 ) {
	  if( argc >= 4 ) {
	    opt_1 = argv[2];
	    opt_2 = argv[3];
	    if( argc > 4 ) {
	      assert( (argc >= 5) && (argc <= 6) );
	      opt_3 = argv[4];
	      if( argc > 5 )
		opt_4 = argv[5];
	    }
	    goto mode_udp_unicast;
	  } else
	    show_banner();
	} else
	  show_banner();
      } else {
	// udpsender DST_HOST_IPADDR UDP_SENT_DSTPORT [BULKSENT_PACKETSNUM] [CAST_INTERVAL]
	BOOL acc_dest = FALSE;
	assert( argc >= 3 );
	if( argc <= 5 ) {
	  opt_1 = argv[1];
	  opt_2 = argv[2];
	  if( argc > 3 ) {
	    assert( (argc >= 4) && (argc <= 5) );
	    opt_3 = argv[3];
	    if( argc > 4 )
	      opt_4 = argv[4];
	  } else
	    show_banner();
	mode_udp_unicast:
	  udpsender_cond.mode = MODE_UDP_UNICAST;
	  acc_dest = par_ipaddr( &udpsender_cond.dest.dest_ip, opt_1, "DST_HOST_IPADDR: " );
	  udpsender_cond.dest.dst_addr = inet_addr( opt_1 );
	  if( acc_dest && !(udpsender_cond.dest.dst_addr < 0) ) {
	    BOOL acc_dstport = FALSE;
	    acc_dstport = par_portnum( &udpsender_cond.dstport, opt_2, "UDP_SENT_DSTPORT: " );
	    if( acc_dstport ) {
	      if( opt_3 ) {
		int nblkpackets = -1;
		nblkpackets = atoi( opt_3 );
		if( (nblkpackets > 0) && (nblkpackets <= 10000) ) {
		  udpsender_cond.blk_packetsnum = nblkpackets;
		  if( opt_4 ) {
		    int interval = -1;
		    interval = atoi( opt_4 );
		    if( (interval >= 100) && (interval < 1000) ) {
		      udpsender_cond.cast_interval = interval * 1000;
		      r = TRUE;
		    } else
		      printf( "CAST_INTERVAL excesses range.\n" );
		  } else
		    r = TRUE;
		} else
		  printf( "BULKSENT_PACKETSNUM excesses range.\n" );
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
  assert( udpsender_cond.dest.dst_addr != 0 );
  addr.sin_family = AF_INET;
  addr.sin_port = htons( udpsender_cond.dstport );
  addr.sin_addr.s_addr = udpsender_cond.dest.dst_addr;
  if( udpsender_cond.mode == MODE_MULTICAST ) {
    int r_optmod = -1;
    assert( udpsender_cond.emit_mcast.emit_addr != 0 );
    r_optmod = setsockopt( sock, IPPROTO_IP, IP_MULTICAST_IF,
			   (char *)&udpsender_cond.emit_mcast.emit_addr,
			   sizeof(udpsender_cond.emit_mcast.emit_addr) );
    if( r_optmod < 0 ) {
      printf( "failed to specify the NIC for UDP MULTI-casting, giving up.\n" );
      exit( TERM_CANNOT_MODIFY_SOCKETOPT );
    }
  }
  
  FOREVER {
    int data_size = -1;
    int sent_bytes = -1;
    int i;
    data_size = edit_sentdata();
    assert( data_size > -1 );
    for( i = 0; i < udpsender_cond.blk_packetsnum; i++ ) {
      sent_bytes = sendto( sock, payload_buf, data_size, 0, (struct sockaddr *)&addr, sizeof(addr) );
      if( sent_bytes < 0 )
	printf( "faled to send the UDP packet, :-P\n" );
      else
	printf( "seq:%u, sent %d [bytes].\n", udpsender_cond.seq, sent_bytes );
      udpsender_cond.seq++;
    }
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
