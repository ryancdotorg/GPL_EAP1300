#include <wamp.h>

topic_t topic_table[] =
{
	{"getCpuUsage", TOPIC_CPU_USAGE, get_cpu_usage},
	{"getMemoryUsage", TOPIC_MEM_USAGE, get_mem_usage},
	{"getPingResult", TOPIC_PING, get_ping_result},
	{"getStationList", TOPIC_STA_LIST, get_station_list},
	{"getAntennaResult", TOPIC_ANTENNA, get_antenna_result},
	{NULL, 0, NULL}
};

WAMP_Node_t *swd_wamp_new(void)
{
    debug_print("[SWD][DEBUG]\n");
	WAMP_Node_t *tmpNode;
	tmpNode = (WAMP_Node_t *) malloc(sizeof(WAMP_Node_t));
	if(tmpNode == NULL){
		debug_print("out of memory\n");
		exit(1);
	}
	tmpNode->request_id = 0;
	tmpNode->subscription = 0;
	tmpNode->publish_id = 0;
	tmpNode->subscribe_id = 0;
	tmpNode->interval = 1;
    tmpNode->para = NULL;
	tmpNode->topic = TOPIC_NONE;
	INIT_LIST_HEAD(&tmpNode->list);
	return (tmpNode);
}

CLIENT_Node_t *swd_wamp_add(CLIENT_Node_t *cNode, WAMP_Node_t *wNode)
{
	list_add(&wNode->list, &cNode->Wlist);
	return cNode;
}

WAMP_Node_t *swd_wamp_lookup(CLIENT_Node_t *cNode, int request_id, int topic) {
	WAMP_Node_t *node;
	list_for_each_entry(node, &cNode->Wlist, list){
		if( node->request_id == request_id || node->topic == topic)
			return node;
	}
	return NULL;
}

int swd_wamp_del(CLIENT_Node_t *cNode, int subscription){
	WAMP_Node_t *node;
	list_for_each_entry(node, &cNode->Wlist, list){
		if(node->subscription == subscription) {
            free(node->para);
			list_del(&node->list);
			free(node);
			return SUCCESS;
		}
	}
	return FAIL;
}

int swd_wamp_handle(struct lws *wsi, char *input, CLIENT_Node_t *cNode) {
	struct json_object *jobj, *jobj_roles, *jobj_tmp, *jobj_new, *jobj_para;
	int type = 0, role = 0, len, rv, topic_id, have_subscribed = 1, i = 0;
	int exists, session, request_id, subscription, interval;
	char data[65536]={0}, buf[65536]={0}, topic[16] = {0}, *para; /* FIXME : can't not support client number over 500 */
	int topicTableLen = (sizeof(topic_table)/sizeof(topic_table[0]));
	jobj = json_tokener_parse(input);
	if (!jobj)
		return FAIL; /* wrong json format */
	/* len = json_object_array_length(jobj); */
	type = json_object_get_int(json_object_array_get_idx(jobj, 0));
	debug_print("[SWD][DEBUG] type:[%d]\n", type);
	srand( (unsigned)time(NULL) );
	WAMP_Node_t *wNode = NULL, *wamp_head = NULL;

	switch(type)
	{
		case HELLO: /* [HELLO, Realm|uri, Details|dict], ex:[1, "senao.com", { "roles": { "subscriber": {} } }] */
			debug_print("[SWD][DEBUG] HELLO\n");
			session = rand();
			jobj_new = json_object_array_get_idx(jobj, 2);
			jobj_roles = json_object_object_get(jobj_new, "roles");
			/* roles */
			exists=json_object_object_get_ex(jobj_roles,"subscriber", &jobj_tmp);
			if(exists) 
                role |= 1 << ROLE_SUBSCRIBER;
			exists=json_object_object_get_ex(jobj_roles,"publisher", &jobj_tmp);
			if(exists)
				role |= 1 << ROLE_PUBLISHER;

			debug_print("[SWD][DEBUG] role:[%d] \n", role);
			cNode->role = role;

			/******************************************************************************************/
			/* [WELCOME, Session|id, Details|dict], ex:[2, 9129137332, {\"roles\": {\"broker\": {}}}] */
			cNode->session = session;
			memset(data, 0, sizeof(data));
			sprintf(data, "[%d, %d, {\"roles\": {\"broker\": {}}}]", WELCOME, session);
			websocket_write_back(wsi, data, sizeof(data)); 
			json_object_put(jobj_roles);
			json_object_put(jobj_new);
			json_object_put(jobj_tmp);
			break;
		case SUBSCRIBE:  /* [SUBSCRIBE, Request|id, Options|dict, Topic|uri], ex:[32, 22222222, {"interval" : 1}, "throughput"] */
			/* debug_print("[SWD][DEBUG] SUBSCRIBE cNode->wamp_node:[%p] \n", cNode->wamp_node); */
			if (cNode->session == 0)
				return FAIL; /* not hello yet */
			subscription = rand();
			/* request ID */
			request_id = json_object_get_int(json_object_array_get_idx(jobj, 1));
			/* parameter */
			jobj_para = json_object_array_get_idx(jobj, 2);
			para = json_object_to_json_string(jobj_para);
			len = strlen(para);
			interval = json_object_get_int(json_object_object_get(jobj_para, "interval"));
			/* subscribe topic */
			sprintf(topic, "%s", json_object_get_string(json_object_array_get_idx(jobj, 3)));
			debug_print("[SWD][DEBUG] interval:[%d] request id:[%d] subscription:[%d] topic:[%s] \n", interval, request_id, subscription, topic);
			debug_print("[SWD][DEBUG] topicTableLen:[%d]\n", topicTableLen);
			for (i=0; i<topicTableLen-1; i++) {
				if(strcmp(topic, topic_table[i].str)==0) {
					topic_id = topic_table[i].id;
					break;
				}
			}
			if((wNode = swd_wamp_lookup(cNode, request_id, topic_id)) == NULL) {
				debug_print("[SWD][DEBUG] Not subscribed \n");
				wNode = swd_wamp_new();
				debug_print("[SWD][DEBUG] len:[%d] \n", len);
				wNode->para = (char *) malloc(sizeof(char) * len + 1);
				have_subscribed = 0;
			}
			else
				wNode->para = realloc(wNode->para, sizeof(char) * len + 1);

			wNode->request_id = request_id;
			wNode->subscription = subscription;
			wNode->interval = interval;
			wNode->topic = topic_id;
			memcpy(wNode->para, json_object_to_json_string(jobj_para), len + 1);

			if(!have_subscribed)
				cNode = swd_wamp_add(cNode, wNode); 

			/* todo */
			/*************************************************************************************/
			/* [ERROR, SUBSCRIBE, SUBSCRIBE.Request|id, Details|dict, Error|uri] */
			/* [8, 32, 713845233, {}, "wamp.error.not_authorized"] */

			/*************************************************************************************/
			/* [SUBSCRIBED, SUBSCRIBE.Request|id, Subscription|id] ex:[33, 713845233, 5512315355] */
			memset(data, 0, sizeof(data));
			sprintf(data, "[%d, %d, %d]", SUBSCRIBED, request_id, subscription);
			websocket_write_back(wsi, data, strlen(data));
			json_object_put(jobj_para);
			break;
		case UNSUBSCRIBE: /* [UNSUBSCRIBE, Request|id, SUBSCRIBED.Subscription|id], ex:[34, 85346237, 5512315355]*/
			debug_print("[SWD][DEBUG] UNSUBSCRIBE \n");
			request_id = json_object_get_int(json_object_array_get_idx(jobj, 1));
			subscription = json_object_get_int(json_object_array_get_idx(jobj, 2));
			rv = swd_wamp_del(cNode, subscription);
			debug_print("[SWD][DEBUG] rv:[%d]\n", rv);
			if(rv) {
				/*************************************************************************************/
				/* [ERROR, UNSUBSCRIBE, UNSUBSCRIBE.Request|id, Details|dict, Error|uri] */
				/* [8, 34, 85346237, {}, "wamp.error.no_such_subscription"] */
				memset(data, 0, sizeof(data));
				sprintf(data, "[%d, %d, %d, \"wamp.error.no_such_subscription\"]", ERROR, UNSUBSCRIBE, request_id);
				websocket_write_back(wsi, data, strlen(data));
			}
			else {
				/*************************************************************************************/
				/* [UNSUBSCRIBED, UNSUBSCRIBE.Request|id] ex:[35, 85346237] */
				memset(data, 0, sizeof(data));
				sprintf(data, "[%d, %d]", UNSUBSCRIBED, request_id);
				websocket_write_back(wsi, data, strlen(data));
			}
			break;
		case EVENT:  /* [EVENT, SUBSCRIBED.Subscription|id, PUBLISHED.Publication|id, Details|dict, PUBLISH.Arguments|list, PUBLISH.ArgumentKw|dict],
                    ex: [36,    5512315355                , 4429313566              , {}          , []                    , {"color": "orange", "sizes": [23, 42, 7]}] */
			debug_print("[SWD][DEBUG] EVENT \n");
			list_for_each_entry(wNode, &cNode->Wlist, list) {
				debug_print("[SWD][DEBUG] wNode:[%p] topic:[%d] count:[%d] interval:[%d] \n", wNode, wNode->topic, count, wNode->interval);
				WAMP_Node_t *wNode_tmp = list_entry(wNode->list.next, typeof(*wNode), list);
				if (CHECK_EVENT_HANDLED(wNode->interval) == 0) {
					if(count % SHOW_PURE_INTERVAL(wNode->interval) == 0) {
						debug_print("[SWD][DEBUG] wNode:[%p] sent event topic:[%d] \n", wNode, wNode->topic);
						memset(data, 0, sizeof(data));
						for (i=0; i<topicTableLen-1; i++) {
							if(wNode->topic == topic_table[i].id) {
								topic_table[i].cb_function(wNode, buf);
								break;
							}
						}
						debug_print("[SWD][DEBUG] data:[%s]\n", buf);
						sprintf(data, "[%d, %d, %d, {}, [], %s]", EVENT, wNode->subscription, session, buf); /*Todo Publication*/
						websocket_write_back(wsi, data, strlen(data));

						SET_EVENT_HANDLED(wNode->interval);
						if (&wNode_tmp->list == &cNode->Wlist) {
							debug_print("[SWD][DEBUG] last wNode:[%p] \n", wNode);
							list_for_each_entry(wNode_tmp, &cNode->Wlist, list) {
								SET_EVENT_UNHANDLED(wNode_tmp->interval);
							}
							break;
						}
						else{
							debug_print("[SWD][DEBUG] wNode:[%p] \n", wNode);
							lws_callback_on_writable(wsi); /* lws strictly enforces only one lws_write() per WRITEABLE callback. */
							break;
						}
					}
				}
			}
			break;
	}
	if (jobj)
		json_object_put(jobj);
	return 0;
}
