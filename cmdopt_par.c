#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include "udpsender.h"

typedef enum ipaddr_par_ph {
  FST_OCT = 1,
  SND_OCT,
  THR_OCT,
  FOR_OCT,
  END_OF_PAR_PH
} IPADDR_PAR_PH;
BOOL par_ipaddr ( IPADDR_PTR pipaddr, const char *pstr_ipaddr, const char *prefix_errmsg ) {
  assert( pipaddr );
  assert( pstr_ipaddr );
  assert( prefix_errmsg );
  BOOL r = FALSE;
  char oct_digs[3 + 1] = {};
  
  IPADDR_PAR_PH ph = FST_OCT;
  int cnt = 0;
  int i = 0;
  pipaddr->fst_oct = -1;
  pipaddr->snd_oct = -1;
  pipaddr->thr_oct = -1;
  pipaddr->for_oct = -1;
  while( pstr_ipaddr[i] ) {
    if( isdigit( (int)pstr_ipaddr[i] ) ) {
      if( cnt < 3 ) {
	oct_digs[cnt] = pstr_ipaddr[i];
	i++;
	if( (pstr_ipaddr[i] == '.') && (ph < FOR_OCT) ) {
	  i++;
	fill_octet:
	  oct_digs[cnt + 1] = 0;
	  switch( ph ) {
	  case FST_OCT:
	    pipaddr->fst_oct = atoi( oct_digs );
	    ph = SND_OCT;
	    break;
	  case SND_OCT:
	    pipaddr->snd_oct = atoi( oct_digs );
	    ph = THR_OCT;
	    break;
	  case THR_OCT:
	    pipaddr->thr_oct = atoi( oct_digs );
	    ph = FOR_OCT;
	    break;
	  case FOR_OCT:
	    pipaddr->for_oct = atoi( oct_digs );
	    ph = END_OF_PAR_PH;
	    r = TRUE;
	    break;
	  case END_OF_PAR_PH:	    
	    printf( "%s", prefix_errmsg );
	    printf( "excessive %dth octet.\n", ph );
	    return FALSE;
	  default:
	    assert( FALSE );
	    printf( "internal error in par_ipaddr.\n" );
	    return FALSE;
	  }
	  cnt = 0;
	} else if( !pstr_ipaddr[i] && (ph >= FOR_OCT) ) {
	  goto fill_octet;
	} else
	  cnt++;
      } else {
	printf( "%s", prefix_errmsg );
	printf( "%dth octet has excessive digits.\n", ph );
	ph = END_OF_PAR_PH;
	r = FALSE;
	break;
      }
    } else {
      printf( "%s", prefix_errmsg );
      printf( "%dth octet has invaid characters.\n", ph );
      ph = END_OF_PAR_PH;
      r = FALSE;
      break;
    }
  }
  if( ph != END_OF_PAR_PH )
    if( !r ) {
      printf( "%dth octet is corrputed.\n", ph );
      r = FALSE;
    }
  if( r ) {
    assert( ph == END_OF_PAR_PH );
    assert( pipaddr->fst_oct > -1 );
    assert( pipaddr->snd_oct > -1 );
    assert( pipaddr->thr_oct > -1 );
    assert( pipaddr->for_oct > -1 );    
    BOOL acc[4] = { FALSE, FALSE, FALSE, FALSE };    
    acc[0] = (pipaddr->fst_oct >= 0) && (pipaddr->fst_oct < 256);
    acc[1] = (pipaddr->snd_oct >= 0) && (pipaddr->snd_oct < 256);
    acc[2] = (pipaddr->thr_oct >= 0) && (pipaddr->thr_oct < 256);
    acc[3] = (pipaddr->for_oct >= 0) && (pipaddr->for_oct < 256);
    if( !(acc[0] && acc[1] && acc[2] && acc[3]) ) {
      BOOL dirty = FALSE;
      int i;
      printf( "%s", prefix_errmsg );
      for( i = 0; i < 4; i++ ) {
	if( dirty )
	  printf( ", " );
	dirty = FALSE;
	if( acc[i] ) {
	  printf( "%dth", (i + 1) );
	  dirty = TRUE;
	}
      }
      printf( " octets are invalid.\n" );
      r = FALSE;
    }
  }
  return r;
}

BOOL par_portnum ( unsigned short *pportnum, const char *pstr_portnum, const char *prefix_errmsg ) {
  assert( pportnum );
  assert( pstr_portnum );
  assert( prefix_errmsg );
  BOOL r = FALSE;;  
  char digs[5 + 1] = "";
  
  int i = 0;
  *pportnum = -1;
  while( pstr_portnum[i] ) {
    if( i < 5 ) {
      if( isdigit( (int)pstr_portnum[i] ) ) {
	digs[i] = pstr_portnum[i];
	digs[i + 1] = 0;
	r = TRUE;
      } else {
	printf( "%s", prefix_errmsg );
	printf( "invalid character, NOT DIGITS are included in %s.\n", pstr_portnum );
	r = FALSE;
	break;
      }
    } else {
      printf( "%s", prefix_errmsg );
      printf( "%s excesses the range of UDP/IP port number.\n", pstr_portnum );
      r = FALSE;
      break;
    }
    i++;
  }
  if( r ) {
    int n = -1;
    n = atoi( digs );
    if( (n > 0) && (n < 65536) ) {
      *pportnum = n;
    } else {
      printf( "%s", prefix_errmsg );
      printf( "%s is out of range over UDP/IP port number.\n", pstr_portnum );
      r = FALSE;
    }
  }
  return r;
}

#if 0
int main ( void ) {
  BOOL r = FALSE;
#if 0
  IPADDR ipaddr = {};
  r = par_ipaddr( &ipaddr, "192.168.0.243", "" );
  if( r ) {
    printf( "%d.%d.%d.%d\n",
	    ipaddr.fst_oct, ipaddr.snd_oct, ipaddr.thr_oct, ipaddr.for_oct );
  }
#endif
  unsigned short dst_portnum = -1;
  r = par_portnum( &dst_portnum, "-1024", "" );
  if( r ) {
    printf( "%d is in range of valid UDP/IP dest. port number.\n", dst_portnum );
  }
  return r;
}
#endif
