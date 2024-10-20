/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                     *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  linkmon                                                                       *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include "mio.h"
#include "util.h"
#include "linkmon.h"
#include "phylink.h"
#include "arpdup.h"
#include "icmp.h"
#include "dns.h"
#include "http.h"
#include "https.h"
#include "cmdipc.h"
#include "ipc_udp.h"
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define PID_FILE "/var/run/linkmon.pid"

void show_usage()
{
	printf("%s usage:\n" \
" -d               daemonlize this process\n" \
" -l <seconds>     run the process as recharge mode, the process will terminate\n" \
"                  after <seconds> without the trigger of external signal(10)\n" \
" -i <ifname>      select interface for ip duplicate detection\n" \
" -a <ipaddr>      select ip address for icmp detection\n" \
" -n <name_server> select ip address of dns server for dns check\n" \
" -q <host>        select target host for dns check\n" \
" -h               show this help\n", __FILE__);
}


void pidfile_init(const char *pid_file)
{
	if(pid_file){

		if (0 == access(pid_file, 0)) { 
			lmdbg("process is running already, terminate!\n");
			exit(0);
		} 
		FILE *f = fopen(pid_file, "w");
		if (f) {
			fprintf(f, "%u\n", getpid());	
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

void sig_exit_func(int sig, void *data)
{
	int *terminate = (int *)data;
	lmdbg("got signal:[%d]\n", sig);
	*terminate = 1;
}

struct recharge_data_t {
	int *terminate;
	int terminate_ignore;
}recharge_data_t;

void set_terminate(void *data)
{
	struct recharge_data_t *rd = (struct recharge_data_t *)data;
	if (rd->terminate_ignore == 1){
		rd->terminate_ignore = 0;
		lmdbg("ignore terminate\n");
		return;
	}
	*(rd->terminate) = 1;
}

void print_monitor_result(void *data)
{
	linkmon_t *linkmon = (linkmon_t *)data;
	if (linkmon)
		traverse_linkmon(linkmon);
}

void register_monitor(linkmon_t *linkmon_pool, char *wan_ifname, char *ipaddr, char *dns_server, char*query_host )
{
	init_phylink(linkmon_pool, wan_ifname);
	init_arpdup(linkmon_pool, wan_ifname);	
	init_icmp(linkmon_pool, ipaddr);	
	init_dns(linkmon_pool, dns_server, query_host);
	init_http(linkmon_pool, query_host);
	init_https(linkmon_pool, query_host);
}

void unregister_monitor(linkmon_t *linkmon_pool)
{
	uninit_phylink(linkmon_pool);
	uninit_arpdup(linkmon_pool);
	uninit_icmp(linkmon_pool);
	uninit_dns(linkmon_pool);
	uninit_http(linkmon_pool);
	uninit_https(linkmon_pool);
}

void init_monitor(mio_data_t *mio_data, linkmon_t *linkmon_pool)
{
	linkmon_t *ptr;
	list_for_each_entry(ptr, &(linkmon_pool->list),list) {
		if (ptr->lm_initfunc)
			ptr->lm_initfunc(mio_data, ptr->data);
	}
}
void uninit_monitor(linkmon_t *linkmon_pool)
{
	linkmon_t *ptr;
	list_for_each_entry(ptr, &(linkmon_pool->list),list) {
		if (ptr->lm_uninitfunc){
			ptr->lm_uninitfunc(ptr->data);
		}
	}
}

void register_ipc(mio_data_t *mio_data, cmdipc_t *cmdipc)
{
	cmdipc_t *ptr;
	list_for_each_entry(ptr, &(cmdipc->list),list) {
		if (ptr->ipc_sockfunc){
			ptr->fd = ptr->ipc_sockfunc(ptr->data);
		}
		if (ptr->ipc_recvfunc){
			if (ptr->fd > 0)
				add_event(&(mio_data->read_pool), ptr->fd, (event_func)ptr->ipc_recvfunc, ptr->data);
		}
/*		if (ptr->lm_sendfunc){
			add_timer(&(mio_data->timer_pool), ptr->sec, ptr->usec, (timer_func)ptr->lm_sendfunc, ptr->data, 1);
		}
*/	}
}
void unregister_ipc(mio_data_t *mio_data, cmdipc_t *cmdipc)
{
	cmdipc_t *ptr;
	list_for_each_entry(ptr, &(cmdipc->list),list) {
		if (ptr->ipc_closesockfunc && ptr->data){
			ptr->ipc_closesockfunc(ptr->data);
		}
	}
}

void init_ipc(cmdipc_t *cmdipc_pool, linkmon_t *linkmon, int *terminate_ignore)
{
	init_ipc_udp(cmdipc_pool, linkmon, terminate_ignore);
}
void uninit_ipc(cmdipc_t *cmdipc_pool)
{
	uninit_ipc_udp(cmdipc_pool);	
}

typedef struct sig_data_t {
	struct mio_timer_t *timer_pool;
	struct mio_timer_t *mio_timer;
	int timesec;
}sig_data_t;
void sig_modtimer(int sig,void *data)
{
	sig_data_t *sigdata = (sig_data_t *)data;
	lmdbg("sig:%d, data-> timesec:[%d]\n", sig, sigdata->timesec);
	modify_timer(sigdata->timer_pool, sigdata->mio_timer, sigdata->timesec, 0, 1);
}

int main(int argc, char *argv[])
{
	int daemonize = 0;
	int recharge_time=0;
	int c;
	char wan_ifname[16] = "";
	char ipaddr[16] = "";
	char dns_server[16] = "";
	char query_host[32] = "";
	while ((c = getopt(argc, argv,"dl:i:a:n:q:h")) != -1){
		if (c<0){
			printf("input");
			break;
		}
		switch(c) {
			case 'h':
				show_usage();
				return 0;
			case 'd':
				daemonize = 1;
				break;
			case 'l':
				sscanf(optarg,"%d", &recharge_time);
				break;
			case 'i':
				sscanf(optarg,"%s", wan_ifname);
				break;
			case 'a':
				sscanf(optarg,"%s", ipaddr);
				break;
			case 'n': // dns server
				sscanf(optarg,"%s", dns_server);
				break;
			case 'q': // query host
				sscanf(optarg,"%s", query_host);
				break;
			default:
				exit(EXIT_FAILURE);
		}		
	}
	if (optind ==1 && argc ==1){
		show_usage();
		return 0;
	}

	if (daemonize == 1){
		daemon(0, 0);
	}
	pidfile_init(PID_FILE);
	atexit(cleanup);
	mio_data_t *mio_data = mio_init();
	add_signal(&(mio_data->signal_pool), SIGINT, sig_exit_func, &(mio_data->terminate));
	add_signal(&(mio_data->signal_pool), SIGHUP, sig_exit_func, &(mio_data->terminate));
	add_signal(&(mio_data->signal_pool), SIGTERM, sig_exit_func, &(mio_data->terminate));
	linkmon_t *linkmon = linkmon_init();
	cmdipc_t *cmdipc = cmdipc_init();
	mio_timer_t *mt;
	struct recharge_data_t recharge_data;
	recharge_data.terminate = &(mio_data->terminate);
	recharge_data.terminate_ignore = 0;
	if (recharge_time > 0)
		mt = add_timer(&(mio_data->timer_pool), recharge_time, 0, set_terminate, (void *)&recharge_data, 1);
	
	register_monitor(linkmon, wan_ifname, ipaddr, dns_server, query_host);
	init_ipc(cmdipc, linkmon, &(recharge_data.terminate_ignore));
	init_monitor(mio_data, linkmon);
	register_ipc(mio_data, cmdipc);

	sig_data_t sigdata;
	sigdata.mio_timer = mt;
	sigdata.timer_pool = &(mio_data->timer_pool);
	sigdata.timesec = recharge_time;
	add_signal(&(mio_data->signal_pool), SIGUSR1, sig_modtimer, &sigdata);

	if (recharge_time > 0)
		lmdbg("=== %s started at pid:[%d], recharge signal: [%d]===\n", __FILE__, getpid(), SIGUSR1);
	lmdbg("iface:[%s] icmp ipaddr [%s] recharge time:[%d] dns_server:[%s] query_host:[%s]\n", wan_ifname, ipaddr, recharge_time, dns_server, query_host);
	//add_timer(&(mio_data->timer_pool), 3, 0, print_monitor_result, (void *)linkmon, 1);
	//mio_data->terminate = 1;
	mio_loop(mio_data);

	unregister_ipc(mio_data, cmdipc);
	uninit_monitor(linkmon);
	uninit_ipc(cmdipc);
	unregister_monitor(linkmon);
	cmdipc_uninit(cmdipc);
	// remove recharge time timer
	if (recharge_time > 0)
		remove_timer(mt);
	linkmon_uninit(linkmon);
	mio_uninit(mio_data);

	lmdbg("gracefully end\n");
	if (daemonize == 1)
		pidfile_deinit(PID_FILE);
}
