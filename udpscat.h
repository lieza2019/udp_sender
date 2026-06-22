#define BOOL int
#define TRUE 1
#define FALSE 0
#define FOREVER for(;;)

#define DEFALUT_SEND_INTERVAL (500 * 1000)
#define UDP_PAYLOAD_EDITBUF 65535

typedef struct ipaddr {
  int fst_oct;
  int snd_oct;
  int thr_oct;
  int for_oct;
} IPADDR, *IPADDR_PTR;

BOOL par_ipaddr ( IPADDR_PTR pipaddr, const char *pstr_ipaddr, const char *prefix_errmsg );
BOOL par_portnum ( unsigned short *pportnum, const char *pstr_portnum, const char *prefix_errmsg );
