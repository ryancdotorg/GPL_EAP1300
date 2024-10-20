#ifndef _SWD_SERVER_
#define _SWD_SERVER_

#include <list.h>

extern int DEBUG;

#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, ##__VA_ARGS__); } while (0)
#define MAX_FRAME_SIZE 2048 

extern unsigned int count;

enum {
	SUCCESS = 0, 
	FAIL 
};

typedef struct WAMP_node_s {
	int request_id;
	int subscription;
	int publish_id;
	int subscribe_id;
	int interval;
	char *para;
	int topic;
	struct list_head list;
} __attribute__ ((packed)) WAMP_Node_t;

typedef struct CLIENT_node_s {
	int fd;
	int session;
	int role;
	struct list_head Wlist;
	struct list_head Clist;
} __attribute__ ((packed)) CLIENT_Node_t;

enum WAMP_MSG_SPEC {
    HELLO = 1,
    WELCOME = 2,
    ABORT = 3,
    CHALLENGE = 4,
    AUTHENTICATE = 5,
    GOODBYE = 6,
    ERROR = 8,
    PUBLISH = 16,
    PUBLISHED = 17,
    SUBSCRIBE = 32,
    SUBSCRIBED = 33,
    UNSUBSCRIBE = 34,
    UNSUBSCRIBED = 35,
    EVENT = 36,
    CALL = 48,
    CANCEL = 49,
    RESULT = 50,
    REGISTER = 64,
    REGISTERED = 65,
    UNREGISTER = 66,
    UNREGISTERED = 67,
    INVOCATION = 68,
    INTERRUPT = 69,
    YIELD = 70
};

enum WAMP_ROLES {
	ROLE_ROUTER = 1,
	ROLE_CALLEE,
	ROLE_CALLER,
	ROLE_PUBLISHER,
	ROLE_SUBSCRIBER
};

#endif
