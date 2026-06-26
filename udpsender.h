#define BOOL int
#define TRUE 1
#define FALSE 0
#define FOREVER for(;;)

#define DEFALUT_SEND_INTERVAL (999 * 1000) // in micro-seconds.
#define UDP_PAYLOAD_EDITBUF 520 // in bytes.

typedef enum ope_mode {
  MODE_UNKNOWN = 0,
  MODE_MULTICAST,
  MODE_UDP_UNICAST
} UDPSENDER_OPEMODE;

typedef struct ipaddr {
  int fst_oct;
  int snd_oct;
  int thr_oct;
  int for_oct;
} IPADDR, *IPADDR_PTR;

BOOL par_ipaddr ( IPADDR_PTR pipaddr, const char *pstr_ipaddr, const char *prefix_errmsg );
BOOL par_portnum ( unsigned short *pportnum, const char *pstr_portnum, const char *prefix_errmsg );
