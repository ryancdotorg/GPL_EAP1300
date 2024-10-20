#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>

#include "appagentd.h"
#include "stdio.h"
#include "apiport.h"

#if APP_AGENTD_HAS_SSL
#include "http_ssl.h"
#endif

#if HAS_COMMAND_LIST
#include <stdarg.h>
#include <pthread.h>
#include "command_list.h"
LIST_HEAD(authHead);

pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#define HTTPD_PORTS     4

#define ERR_CANNOT_BIND_LOCAL_ADDRESS -2
#define max(x, y) ((x>y)?x:y)

/*---------------------------------------------------------------------*/
/* Global variables                                                    */
/*---------------------------------------------------------------------*/
typedef struct
{
	int s;
	UINT16 port;
}
HTTP_PORT_T;


//*************************
// Variables
//*************************
KNL_MSGQ_ID httpd_MsgQ;
KNL_MSGQ_ID httpd_CmdQ = 0;

static  INT32   (*pHttpdHostCheck)(UINT32 host_ip);

//*************************
// Functions
//*************************
typedef struct
{
	int op;
	UINT16 port;
}
HTTP_CMD_T;

#define HTTP_ADD_PORT       1
#define HTTP_DEL_PORT       2
#define HTTP_STOP           3

#ifdef SUPPORT_DEBUG_TOOL
#define HTTPD_LISTEN_PORT   8080
#else
#define HTTPD_LISTEN_PORT   9090
#endif

#define HTTPD_SERVICE 		"telnet"

static int isHttpdRunning = 1;

static void httpdTerminator(int num)
{
	isHttpdRunning = 0;
}

#if HAS_COMMAND_LIST
void *httpd_Daemon(void *argu)
#else
void httpd_Daemon()
#endif
{
	int childpid;
	int inetdflag;
	int port,client_len;
	int server_sockfd=-1, client_sockfd=-1;
	struct sockaddr_in       tcp_srv_addr;
	struct sockaddr_in       tcp_cli_addr;
	/* select */
	fd_set readfds;
	struct timeval    tv;			  /* Select timeout */
	int on=1;
	int is_error=0;
	int max_sockfd = 0;
#if SUPPORT_IPV6_SETTING
	int server_sockfd6=-1, client_sockfd6=-1, client_len6;
	struct sockaddr_in6      tcp_srv_addr6;
	struct sockaddr_in6      tcp_cli_addr6;
	int v6only=1;
#endif

/*********************************************************/
/* cfho 2006-0921, information get from Internet */
/*
The kernel will send the SIGPIPE signal when the remote end closes or shuts down the socket and you try to send/write. The default signal handler will terminate your program.

One solution is to ignore the signal using 'signal(SIGPIPE, SIG_IGN)' somewhere at the start of your program. You will get an EPIPE error return when send/write fails due to a broken pipe.

Another less portable solution is to pass the MSG_NOSIGNAL flag to send(). This has the same effect.
*/
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, &httpdTerminator);
/*********************************************************/

	//initApShareMem();

	//port = HTTPD_LISTEN_PORT;
	api_get_integer_option("app_agent.agent.port_aes", &port);

#if APP_AGENTD_HAS_SSL
	if (do_ssl) port = https_port;
#endif

	printf("app_agentd starts on IPv4 port %d\n", port);
#if SUPPORT_IPV6_SETTING
	printf("app_agentd starts on IPv6 port %d\n", port);
#endif

	bzero((char *) &tcp_srv_addr, sizeof(tcp_srv_addr));
	tcp_srv_addr.sin_family      = AF_INET;
	tcp_srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	tcp_srv_addr.sin_port = htons(port);

	if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("%s: can't create stream socket\n",__FUNCTION__);
		is_error = 1;
		goto out;
	}

	struct timeval recvTimeout;
	recvTimeout.tv_sec = 1;
	recvTimeout.tv_usec = 0;
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeout,  sizeof(recvTimeout)) < 0 )
	{   
		perror("setsockopt");
		is_error = 1;
		goto out;
	}

	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))<0)
	{
		printf("%s: setsockopt(SOL_SOCKET) error\n",__FUNCTION__);
		is_error = 1;
		goto out;
	}

	if (fcntl(server_sockfd, F_SETFD, FD_CLOEXEC) < 0)
	{
		printf("%s: fcntl(F_SETFD, FD_CLOEXEC) error\n",__FUNCTION__);
		is_error = 1;
		goto out;
	}

	if (bind(server_sockfd, (struct sockaddr *) &tcp_srv_addr, sizeof(tcp_srv_addr)) < 0)
	{
		printf("%s: Cannot bind local address\n", "[app_agentd]");
		is_error = 1;
		goto out;
	}

	if (listen(server_sockfd, 5)==-1)
	{
		printf("%s:listen error  \n",__FUNCTION__);
	}
	//printf("server_sockfd=%d\n",server_sockfd);

#if SUPPORT_IPV6_SETTING
	bzero((char *) &tcp_srv_addr6, sizeof(tcp_srv_addr6));
	tcp_srv_addr6.sin6_family = AF_INET6;
	tcp_srv_addr6.sin6_flowinfo = 0;
	tcp_srv_addr6.sin6_addr = in6addr_any;
	tcp_srv_addr6.sin6_port = htons(port);

	if ((server_sockfd6 = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
	{
		printf("%s: can't create stream socket\n",__FUNCTION__);
		is_error = 1;
		goto out;
	}

	if (setsockopt(server_sockfd6, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))<0)
	{
		printf("%s: setsockopt(SOL_SOCKET) error\n",__FUNCTION__);
		is_error = 1;
		goto out;
	}

	if (setsockopt(server_sockfd6, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only))<0)
	{
		printf("%s: setsockopt(SOL_SOCKET) IPV6_V6ONLY\n",__FUNCTION__);
		exit(1);
	}

	if (fcntl(server_sockfd6, F_SETFD, FD_CLOEXEC) < 0)
	{
		printf("%s: fcntl(F_SETFD, FD_CLOEXEC) error\n",__FUNCTION__);
		is_error = 1;
		goto out;
	}

	if (bind(server_sockfd6, (struct sockaddr *) &tcp_srv_addr6, sizeof(tcp_srv_addr6)) < 0)
	{
		printf("%s: Cannot bind IPv6 local address\n",__FUNCTION__);
		is_error = 1;
		goto out;
	}

	if (listen(server_sockfd6, 5) == -1)
	{
		printf("%s:listen IPv6 error \n",__FUNCTION__);
	}
	//printf("server_sockfd6=%d\n", server_sockfd6);
#endif

#if APP_AGENTD_HAS_SSL
	if (do_ssl && http_ssl_init() < 0)
	{
		printf("%s: Cannot initialize SSL\n",__FUNCTION__);
		is_error = 1;
		goto out;
	}
#endif

	client_len  = sizeof(tcp_cli_addr);
#if SUPPORT_IPV6_SETTING
	client_len6 = sizeof(tcp_cli_addr6);
#endif

	FD_ZERO(&readfds);
	FD_SET(server_sockfd, &readfds);
#if SUPPORT_IPV6_SETTING
	FD_SET(server_sockfd6, &readfds);
#endif

	max_sockfd = server_sockfd;
#if SUPPORT_IPV6_SETTING
	max_sockfd = max(max_sockfd, server_sockfd6);
#endif

	while(isHttpdRunning)
	{
		fd_set rfds = readfds;

		/* Init timeout value -> 500ms*/
		tv.tv_sec = 0;
		tv.tv_usec = 500000;

		if(select(max_sockfd+1, &rfds, NULL, NULL, &tv))
		{
			if(FD_ISSET(server_sockfd, &rfds))
			{
				client_sockfd  = accept(server_sockfd, (struct sockaddr *) &tcp_cli_addr, &client_len);
				if(client_sockfd>=0)
				{
#if APP_AGENTD_HAS_SSL
					if (do_ssl && http_ssl_accept(client_sockfd) < 0)
						goto cli_out;
#endif
					https(client_sockfd);

cli_out:
					shutdown(client_sockfd, 2);
					close(client_sockfd);
#if APP_AGENTD_HAS_SSL
					http_ssl_free();
#endif
				}
			}
#if SUPPORT_IPV6_SETTING
			if(FD_ISSET(server_sockfd6, &rfds))
			{
				client_sockfd6 = accept(server_sockfd6, (struct sockaddr *) &tcp_cli_addr6, &client_len6);
				if (client_sockfd6 >= 0)
				{
#if APP_AGENTD_HAS_SSL
					if (do_ssl && http_ssl_accept(client_sockfd6) < 0)
						goto cli6_out;
#endif
					https(client_sockfd6);
					
					cli6_out:
					shutdown(client_sockfd6, 2);
					close(client_sockfd6);
#if APP_AGENTD_HAS_SSL
					http_ssl_free();
#endif
				}
			}
#endif
		}
	}

out:
	close(client_sockfd);
	close(server_sockfd);
#if SUPPORT_IPV6_SETTING
	close(client_sockfd6);
	close(server_sockfd6);
#endif

#if APP_AGENTD_HAS_SSL
	http_ssl_freectx();
#endif

	if(1 == is_error)
	{
		exit(1);
	}
}

/*--------------------------------------------------------------
* ROUTINE NAME - Httpd_Start
*---------------------------------------------------------------
* FUNCTION: This routine creates the http deamon
*
* INPUT:
* RETURN:
*       0   -- no error
*       others -- cannot start
---------------------------------------------------------------*/
INT32   httpd_Start()
{
	if(httpd_CmdQ != 0)
		return OK;

	return OK;
}
#if HAS_COMMAND_LIST
/*--------------------------------------------------------------
* ROUTINE NAME - command_queue
*---------------------------------------------------------------
* FUNCTION:
* INPUT:
* RETURN:
---------------------------------------------------------------*/
void *command_queue(char *command, ...) {
	static char buf_for_COMMAND[512];
	struct command_list_st command_node;
	va_list ap;

	va_start(ap, command);
	vsnprintf(buf_for_COMMAND, sizeof(buf_for_COMMAND), command, ap);
	va_end(ap);

	command_list_init_node(&command_node, buf_for_COMMAND);

	pthread_mutex_lock(&thread_mutex);
	command_list_add_node(&command_node, &authHead);
	pthread_mutex_unlock(&thread_mutex);
}
/*--------------------------------------------------------------
* ROUTINE NAME - do_command
*---------------------------------------------------------------
* FUNCTION:
* INPUT:
* RETURN:
---------------------------------------------------------------*/
void *do_command(void *argu) {
	int rc = 0;
	struct command_list_st *command_node;

	while (1) {
		usleep(500000); //0.5s to avoid high cpu loading
		command_node = command_list_get_head(&authHead);
		if(!command_node)
		{
			continue;
		}
		printf("do command[%s]\n", command_node->command);
		system(command_node->command);
		pthread_mutex_lock(&thread_mutex);
		rc = command_list_delete(command_node->command, &authHead);
		pthread_mutex_unlock(&thread_mutex);
		command_list_display(&authHead);
	}

	command_list_delete_all(&authHead);
	return NULL;
}
#endif
/*--------------------------------------------------------------
* ROUTINE NAME - httpd_AddListenPort
*---------------------------------------------------------------

* FUNCTION: This routine adds a port to the httpd listen port list
*
* INPUT:    port    -- newly added HTTP listen port number
* RETURN:
*       0   -- no error
*       others -- cannot be added
---------------------------------------------------------------*/
INT32   httpd_AddListenPort(UINT16 port)
{
	HTTP_CMD_T cmd;
	cmd.op = HTTP_ADD_PORT;
	cmd.port = port;
#if HAS_COMMAND_LIST
	pthread_t thread1, thread2;

	pthread_create(&thread1, NULL, &httpd_Daemon, NULL);
	pthread_create(&thread2, NULL, &do_command, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread1, NULL);
#else
	httpd_Daemon(cmd);
#endif
}

