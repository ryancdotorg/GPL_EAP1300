#include <libwebsockets.h>
#include "lws_config.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <swd.h>
#include <wamp.h>
#define LOCAL_RESOURCE_PATH INSTALL_DATADIR"/swd"
#define PID_FILE "/var/run/swd.pid"
char *resource_path = LOCAL_RESOURCE_PATH;
int debug_level = 7;
pthread_mutex_t lock_established_conns;
volatile int force_exit = 0;
struct lws_context *context;
unsigned int count = 0;
int DEBUG = 0;
int max_poll_elements;
struct lws_pollfd *pollfds;
int *fd_lookup;
int count_pollfds;
static LIST_HEAD(client_list);

void pidfile_init(const char *pid_file)
{
	if(pid_file){

		if (0 == access(pid_file, 0)) { 
			printf("process is running already, terminate!\n");
			exit(0);
		} 
		FILE *f = fopen(pid_file, "w");
		if (f) {
			debug_print("[SWD][DEBUG] %u\n", getpid());
			fclose(f);
		}
	}
}
void pidfile_deinit(const char *pid_file)
{
	if(pid_file){
		unlink(pid_file);
	}
}

void cleanup()
{
	pidfile_deinit(PID_FILE);
}

CLIENT_Node_t *swd_client_new(int fd)
{
	CLIENT_Node_t *tmpNode;
	tmpNode = (CLIENT_Node_t *) malloc(sizeof(CLIENT_Node_t));
	if(tmpNode == NULL){
		printf("out of memory");
		exit(1);
	}
	INIT_LIST_HEAD(&tmpNode->Clist);
	INIT_LIST_HEAD(&tmpNode->Wlist);

	tmpNode->fd = fd;
	tmpNode->session = 0;
	tmpNode->role = 0;
	list_add_tail(&tmpNode->Clist, &client_list);

	debug_print("[SWD][DEBUG] New Node:[%p] \n", tmpNode);

	return (tmpNode);
}

CLIENT_Node_t *swd_client_lookup(int fd) {
	CLIENT_Node_t *node;
	list_for_each_entry(node, &client_list, Clist){
		debug_print("[SWD][DEBUG] node:[%p] node->fd:[%d] \n", node, node->fd);
		if (node->fd == fd) {
			debug_print("[SWD][DEBUG] %p\n", node);
			return node;
		}
	}
	return NULL;
}

void swd_client_remove(int fd){
	CLIENT_Node_t *ptr, *next;
	list_for_each_entry_safe(ptr, next, &client_list, Clist) {
		if(ptr->fd == fd) {
			debug_print("[SWD][DEBUG] free client [%p] \n", ptr);
			WAMP_Node_t *wnext, *wptr;
			list_for_each_entry_safe(wptr, wnext, &ptr->Wlist, list) {
				debug_print("[SWD][DEBUG] free wamp [%p] \n", wptr);
				list_del(&wptr->list);
				free(wptr);
			}
			list_del(&ptr->Clist);
			free(ptr);
		}
	}
}

void swd_client_flush(void)
{
	CLIENT_Node_t *ptr, *next;
	list_for_each_entry_safe(ptr, next, &client_list, Clist) {
		debug_print("[SWD][DEBUG] free client [%p] \n", ptr);
		WAMP_Node_t *wnext, *wptr;
		list_for_each_entry_safe(wptr, wnext, &ptr->Wlist, list) {
			debug_print("[SWD][DEBUG] free wamp [%p] \n", wptr);
			list_del(&wptr->list);
			free(wptr);
		}
		list_del(&ptr->Clist);
		free(ptr);
	}
}

int websocket_write_back(struct libwebsocket *wsi_in, char *str, int str_size_in) 
{
	if (str == NULL || wsi_in == NULL)
		return -1;

	int n;
	int len;
	char *out = NULL;

	if (str_size_in < 1) 
		len = strlen(str);
	else
		len = str_size_in;

	out = (char *)malloc(sizeof(char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));
	//* setup the buffer*/
	memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );
	//* write out*/
	debug_print("[SWD][DEBUG] out:[%s]\n", str);
	n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);

	//* free the buffer*/
	free(out);

	return n;
}

static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	int m;
	struct lws_pollargs *pa = (struct lws_pollargs *)in;

	switch( reason )
	{
		case LWS_CALLBACK_HTTP:
			lws_serve_http_file( wsi, "example.html", "text/html", NULL, 0 );
			break;
		case LWS_CALLBACK_ADD_POLL_FD:
			if (count_pollfds >= max_poll_elements) {
				lwsl_err("LWS_CALLBACK_ADD_POLL_FD: too many sockets to track\n");
				return 1;
			}
			fd_lookup[pa->fd] = count_pollfds;
			pollfds[count_pollfds].fd = pa->fd;
			pollfds[count_pollfds].events = pa->events;
			pollfds[count_pollfds++].revents = 0;
			break;

		case LWS_CALLBACK_LOCK_POLL:
			/*
			 * lock mutex to protect pollfd state
			 * called before any other POLL related callback
			 * if protecting wsi lifecycle change, len == 1
			 */
			if(len)
				pthread_mutex_lock(&lock_established_conns);
			break;

		case LWS_CALLBACK_UNLOCK_POLL:
			/*
			 * unlock mutex to protect pollfd state when
			 * called after any other POLL related callback
			 * if protecting wsi lifecycle change, len == 1
			 */
			if(len)
				pthread_mutex_unlock(&lock_established_conns);

			break;

		case LWS_CALLBACK_DEL_POLL_FD:
			if (!--count_pollfds)
				break;
			m = fd_lookup[pa->fd];
			/* have the last guy take up the vacant slot */
			pollfds[m] = pollfds[count_pollfds];
			fd_lookup[pollfds[count_pollfds].fd] = m;
			break;

		case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
			pollfds[fd_lookup[pa->fd]].events = pa->events;
			break;

		default:
			break;
	}

	return 0;
}


static int callback_wamp( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	char input[256] = {0}, tmp[16] = {0};
	int socketfd = 0;
	CLIENT_Node_t *node, *node_fd;
	socketfd = lws_get_socket_fd(wsi);
	debug_print("[SWD][DEBUG] socketfd:[%d]\n", socketfd);

	switch( reason )
	{
		case LWS_CALLBACK_ESTABLISHED:
			debug_print("[SWD][DEBUG] LWS_CALLBACK_ESTABLISHED \n");
			node = swd_client_new(socketfd);
			break;
		case LWS_CALLBACK_RECEIVE:
			debug_print("[SWD][DEBUG] LWS_CALLBACK_RECEIVE in:[%s] len:[%d]\n", in, len);
			memset(input, 0, sizeof(input));
			memcpy( input, in, len );
			if(strcmp(input, "") != 0) {
				debug_print("[SWD][DEBUG]\n");
				node_fd = swd_client_lookup(socketfd);
				debug_print("[SWD][DEBUG] node:[%p] \n", node_fd);
				swd_wamp_handle(wsi, input, node_fd);
			}
			break;
		case LWS_CALLBACK_SERVER_WRITEABLE:
			debug_print("[SWD][DEBUG] LWS_CALLBACK_SERVER_WRITEABLE \n");
			node_fd = swd_client_lookup(socketfd);
			sprintf(tmp, "[%d]", EVENT);
			debug_print("[SWD][DEBUG] LWS_CALLBACK_SERVER_WRITEABLE node:[%p] fd:[%d] session:[%d] \n",node_fd, node_fd->fd, node_fd->session);
			swd_wamp_handle(wsi, tmp, node_fd);
			break;
		case LWS_CALLBACK_CLOSED:
			debug_print("[SWD][DEBUG] LWS_CALLBACK_CLOSED \n");
			swd_client_remove(socketfd);
			break;
		default:
			break;
	}

	return 0;
}

enum protocols
{
	PROTOCOL_HTTP = 0,
	PROTOCOL_WAMP
};

static struct lws_protocols protocols[] =
{
	/* The first protocol must always be the HTTP handler */
	{
		"http-only",   /* name */
		callback_http, /* callback */
		0,             /* No per session data. */
		0,             /* max frame size / rx buffer */
	},
	{
		"wamp-protocol",
		callback_wamp,
		0,
		MAX_FRAME_SIZE, /* rx buf size must be >= permessage-deflate rx size */ \
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

void *thread_dumb_increment(void *wsi)
{
	struct timeval tv;
	while (!force_exit) {
		gettimeofday(&tv, NULL);
		/*
		 * this lock means wsi in the active list cannot
		 * disappear underneath us, because the code to add and remove
		 * them is protected by the same lock
		 */
		pthread_mutex_lock(&lock_established_conns);
		count = tv.tv_sec;
		lws_callback_on_writable_all_protocol(context, &protocols[PROTOCOL_WAMP]);
		pthread_mutex_unlock(&lock_established_conns);
		usleep(1000000);
	}
	pthread_exit(NULL);
}

void sighandler(int sig)
{
	force_exit = 1;
	lws_cancel_service(context);
}

int main( int argc, char *argv[] )
{
	char *p=NULL;
	int n = 0, use_ssl = 0, daemonize = 0;
	char cert_path[1024] = "", key_path[1024] = "", ca_path[1024] = "";
	struct lws_context_creation_info info;
	void *retval;
	int threads = 1;
	pthread_t pthread_dumb, pthread_service[32];

	pthread_mutex_init(&lock_established_conns, NULL);

	memset( &info, 0, sizeof(info) );
	info.port = 8002;

	while (n >= 0)
	{
		n = getopt(argc, argv, "hsp:dDC:K:A:");
		if (n < 0)
			continue;
		switch(n)
		{
			case 'h':
				fprintf(stderr, "Usage: swd "
						"[--port=<p>] [--ssl] [-D daemonize]"
						"[-d debug]\n");
				return 0;
			case 's':
				use_ssl = 1;
				break;
			case 'p':
				info.port = atoi(optarg);
				break;
			case 'd':
				/* debug_level = atoi(optarg); */
				DEBUG = 1;
				break;
			case 'D':
				daemonize = 1;
				break;
			case 'C':
				strncpy(cert_path, optarg, sizeof cert_path);
				break;
			case 'K':
				strncpy(key_path, optarg, sizeof key_path);
				break;
			case 'A':
				strncpy(ca_path, optarg, sizeof ca_path);
				break;
		}
	}

	if (daemonize == 1){
		daemon(0, 1);
	}
	pidfile_init(PID_FILE);
	atexit(cleanup);

	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.count_threads = threads;
	max_poll_elements = getdtablesize();
	pollfds = malloc(max_poll_elements * sizeof (struct lws_pollfd));
	fd_lookup = malloc(max_poll_elements * sizeof (int));
	if (pollfds == NULL || fd_lookup == NULL) {
		lwsl_err("Out of memory pollfds=%d\n", max_poll_elements);
		return -1;
	}

	signal(SIGINT, sighandler);
	signal(SIGHUP, sighandler);
	signal(SIGTERM, sighandler);

	lws_set_log_level(debug_level, NULL);

	if (use_ssl) {
		info.port = 4432;
		if (strlen(resource_path) > sizeof(cert_path) - 32) {
			lwsl_err("resource path too long\n");
			return -1;
		}
		if (!cert_path[0])
			sprintf(cert_path, "%s/sn-websockets-server.pem",
								resource_path);
		if (strlen(resource_path) > sizeof(key_path) - 32) {
			lwsl_err("resource path too long\n");
			return -1;
		}
		if (!key_path[0])
			sprintf(key_path, "%s/sn-websockets-server.key.pem",
								resource_path);

		info.ssl_cert_filepath = cert_path;
		info.ssl_private_key_filepath = key_path;
		if (ca_path[0])
			info.ssl_ca_filepath = ca_path;
	}

	//info.options &= ~(LWS_SERVER_OPTION_DISABLE_IPV6);

	context = lws_create_context( &info );

	/* start the dumb increment thread */
	n = pthread_create(&pthread_dumb, NULL, thread_dumb_increment, 0);
	if (n) {
		lwsl_err("Unable to create dumb thread\n");
		goto done;
	}

	/*
	 * this represents an existing server's single poll action
	 * which also includes libwebsocket sockets
	 */

	n = 0;
	while (n >= 0 && !force_exit) {

		n = poll(pollfds, count_pollfds, 50);
		if (n < 0)
			continue;

		if (n)
			for (n = 0; n < count_pollfds; n++)
				if (pollfds[n].revents)
					/*
					 * returns immediately if the fd does not
					 * match anything under libwebsockets
					 * control
					 */
					if (lws_service_fd(context,
								&pollfds[n]) < 0)
						goto done;
	}

	/* wait for pthread_dumb to exit */
	pthread_join(pthread_dumb, &retval);

done:
	swd_client_flush();
	lws_context_destroy(context);
	pthread_mutex_destroy(&lock_established_conns);
	free(pollfds);
	free(fd_lookup);
	if (daemonize == 1)
		pidfile_deinit(PID_FILE);
	lwsl_notice("sn-websockets-server exited cleanly\n");
	return 0;
}
