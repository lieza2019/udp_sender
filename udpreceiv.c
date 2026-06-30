#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#define UDP_MSG_LENGTH 520
#define UDP_UNICAST_RECVPORT 50001
#define uSLEEP_RECV_INTERVAL (20 * 1000)
#define UDP_RECV_BUFSIZ 1024

#define TERM_NORMALLY 0
#define TERM_ILLFORMED_CMDPARAMS 1
#define TERM_CANNOT_CREATE_SOCKET 2
#define TERM_CANNOT_BIND_SOCKET 3

static char recv_buf[UDP_RECV_BUFSIZ];
int main ( int argc, char *argv[] ) {
  struct sockaddr_in addr;
  int sock;
  
  sock = socket( AF_INET, SOCK_DGRAM, 0 );
  if( sock < 0 ) {
    printf( "failed to create socket for reveival, giving-up.\n" );
    exit( TERM_CANNOT_CREATE_SOCKET );
  }
  addr.sin_family = AF_INET;
  addr.sin_port = htons( UDP_UNICAST_RECVPORT );
  addr.sin_addr.s_addr = INADDR_ANY;
  {
    int r_bind = -1;
    r_bind = bind( sock, (struct sockaddr *)&addr, sizeof(addr) );
    if( r_bind < 0 ) {
      printf( "failed to bind socket for reveival, giving-up.\n" );
      exit( TERM_CANNOT_BIND_SOCKET );
    }
  }
  
  memset( recv_buf, 0, sizeof(recv_buf) );
  for(;;) {
    const char *plim = &recv_buf[UDP_RECV_BUFSIZ];
    char *pw = recv_buf;
    if( (plim - pw) > UDP_MSG_LENGTH ) {
      int n = -1;
      //recv( sock, recv_buf, sizeof(recv_buf), 0 );
      n = recv( sock, pw, UDP_MSG_LENGTH, 0 );
      if( n < 0 )
	printf( "failed to receive UDP datagram.\n" );
      else
	printf( "reveived %d bytes.\n", n );
    } else {
      printf( "recv buffer lacked, flushed themall and standing-by again.\n" );
      pw = recv_buf;
    }
    usleep( uSLEEP_RECV_INTERVAL );
  }
  close( sock );
  return TERM_NORMALLY;
}
