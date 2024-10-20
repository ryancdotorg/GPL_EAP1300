// Define some constants.
#define ETH_HDRLEN 14  // Ethernet header length
#define IP4_HDRLEN 20  // IPv4 header length
#define UDP_HDRLEN 8   // ICMP header length for echo request, excludes data
#define MAX_FINGERPRINT_RING_SIZE  512  //for int index, we use 0~511 to recoed, and reserve 512 for another using
#define FINGERPRINT_FILE_PATH "/tmp/fingerprint.txt"
#define FINGERPRINT_DB_FILE_PATH "/tmp/fingerprint_db.txt"
#define FINGERPRINT_IP_FILE_PATH "/tmp/fingerprint_ip.txt"
#define FINGERPRINT_GW_FILE_PATH "/tmp/fingerprint_gw.txt"
#define FINGERPRINT_GW_TMP_FILE_PATH "/tmp/fingerprint_gw_tmp.txt"
#define FINGERPRINT_DEBUG_FILE_PATH "/tmp/fingerprint_debug.txt"
#define MAC_STR_FORMAT  "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"
#define USE_TCPDUMP_PACKET_FILTER   1
#define DHCP_VCI_BUFFER_SIZE                256
#define DHCP_PARAM_REQ_LIST_BUFFER_SIZE     256
#define DHCP_HOSTNAME_BUFFER_SIZE           64
#define OS_NAME_BUFFER_SIZE                 32
#define fp_debug(x, ...) do { \
    if(DebugEnable) \
    { \
        FILE *fp = fopen("/dev/console", "a"); \
        if (fp) { \
            fprintf(fp, "[fingerprint] %s:%d "x"\n", __func__, __LINE__, ##__VA_ARGS__ ); \
            fclose(fp); \
        } \
    } \
} while(0)

#if USE_TCPDUMP_PACKET_FILTER
#include <linux/filter.h>
#endif

typedef struct __finger{
    char os[32];
    char fingerprint[64];
}finger;

typedef struct __vcidb_t{
    char os[32];
    char vci[64];
}vcidb_t;

typedef struct __oui_info_t
{
    char vendor[32];
    char os[32];
}oui_info_t;

typedef struct __client_oui_t
{
    int found;
    oui_info_t info;
}client_oui_t;

typedef struct __ouidb_t{
    oui_info_t oui_info;
    unsigned char oui[3];
}ouidb_t;

void dumpIpDataToFile(int gateway_done);
void dumpFingerDataToFile(char *filename, int ip_done);
void dumpFingerData();
int searchMacInFingerData(char *mac);
void resetFingerData(void);
void resetFingerIpData(void);
int lookup_DHCP_DB(char *os_name, int len, char *param_request_list, client_oui_t *client_oui);
int lookup_VCI_DB(char *os_name, int len, char *vendor_class_ident);
int lookup_OUI_DB(client_oui_t *client_oui, unsigned char *mac);
void printFingerDataToFile(char *filename, struct finger_cfg_list *pfinger_list);
char * allocate_strmem (int len);
uint8_t * allocate_ustrmem (int len);
int * allocate_intmem (int len);
int get_filesize(const char *filename);
void fingerprint_data_receive(int sock, void *eloop_ctx, void *sock_ctx);