#ifndef _WAMP_
#define _WAMP_
#include <libwebsockets.h>
#include <string.h>
#include <swd.h>

#define CHECK_EVENT_HANDLED(interval) \
                    ((interval) & 0xF0000000)
#define SET_EVENT_HANDLED(interval) \
                    (interval) = ((interval) | 0xF0000000)
#define SET_EVENT_UNHANDLED(interval) \
                    (interval) = ((interval) & 0x0FFFFFFF)
#define SHOW_PURE_INTERVAL(interval) \
                    ((interval) & 0x0000FFFF)
#define SHOW_PING_COUNT(interval) \
                    ((interval) & 0x0FFF0000)

typedef struct __topic_t
{
    const char *str;
    const int id;
    int (*cb_function)(WAMP_Node_t *wNode, char *info);
} topic_t;

enum topics {
	TOPIC_CPU_USAGE = 0,
	TOPIC_MEM_USAGE,
	TOPIC_PING,
	TOPIC_STA_LIST,
	TOPIC_ANTENNA,
	TOPIC_NONE
};

int get_cpu_usage(WAMP_Node_t *wNode, char *info);
int get_mem_usage(WAMP_Node_t *wNode, char *info);
int get_ping_result(WAMP_Node_t *wNode, char *info);
int get_station_list(WAMP_Node_t *wNode, char *info);
int get_antenna_result(WAMP_Node_t *wNode, char *info);

WAMP_Node_t *swd_wamp_new(void);
CLIENT_Node_t *swd_wamp_add(CLIENT_Node_t *cNode, WAMP_Node_t *wNode);
WAMP_Node_t *swd_wamp_lookup(CLIENT_Node_t *cNode, int request_id, int topic);
int swd_wamp_del(CLIENT_Node_t *cNode, int subscription);
int swd_wamp_handle(struct lws *wsi, char *input, CLIENT_Node_t *cNode);

#endif

