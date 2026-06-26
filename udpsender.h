#define BOOL int
#define TRUE 1
#define FALSE 0
#define FOREVER for(;;)

#define TERM_NORMALLY 0
#define TERM_ILLFORMED_CMDPARAMS 1
#define TERM_CANNOT_CREATE_SOCKET 2
#define TERM_CANNOT_MODIFY_SOCKETOPT 3

#define DEFALUT_SEND_INTERVAL (999 * 1000) // in micro-seconds.
#define UDP_PAYLOAD_EDITBUF 520 // in bytes.

typedef struct ipaddr {
  int fst_oct;
  int snd_oct;
  int thr_oct;
  int for_oct;
} IPADDR, *IPADDR_PTR;

typedef enum ope_mode {
  MODE_UNKNOWN = 0,
  MODE_MULTICAST,
  MODE_UDP_UNICAST
} UDPSENDER_OPEMODE;

extern BOOL par_ipaddr ( IPADDR_PTR pipaddr, const char *pstr_ipaddr, const char *prefix_errmsg );
extern BOOL par_portnum ( unsigned short *pportnum, const char *pstr_portnum, const char *prefix_errmsg );
