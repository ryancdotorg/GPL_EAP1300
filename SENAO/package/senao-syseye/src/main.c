/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                    *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  syseye                                                                        *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <mio.h>
#include <ezjson.h>
#include "util.h"
#include "global.h"
#include "cmdipc.h"
#include "ipc_tcp.h"
#include "ipc_nl.h"
#include "cmdparser.h"
#include "nl_cmdparser.h"
#include "thread.h"
#include "action.h"
#if THREAD_LINKMON
#include <linkmon.h>
#endif
#if ACT_SAMPLE
#include <act_sample.h>
#endif
#if ACT_SELFTEST
#include <act_selftest.h>
#endif
#if ACT_WIFI_CLINFO
#include <act_wifi_clinfo.h>
#endif
#if ACT_WS_WIFI_CLINFO
#include <act_ws_wifi_clinfo.h>
#endif
#if ACT_SITESURVEY
#include <act_sitesurvey.h>
#endif
#if ACT_LINKCHECK
#include <act_linkcheck.h>
#include <act_check_phylink.h>
#include <act_check_meshap.h>
#include <act_check_arpdup.h>
#include <act_check_icmp.h>
#include <act_check_dns.h>
#include <act_check_http.h>
#include <act_check_https.h>
#endif
#if ACT_PSSCAN
#include <act_psscan.h>
#endif
#if ACT_RUNCMD
#include <act_runcmd.h>
#endif
#if ACT_RUNLIB_SNLOG
#include <act_runlib_snlog.h>
#endif

#define PID_FILE "/var/run/syseye.pid"

void pidfile_init(const char *pid_file)
{
    if(!pid_file){
        printf("pid_file is empty!\n");
        exit(1);
    }

    int fd = open(pid_file, O_RDWR | O_CREAT,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd < 0){
        printf("pid_file open failed!\n");
        exit(1);
    }

    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    if (fcntl(fd, F_SETLK, &fl) < 0){
        if (errno == EACCES || errno == EAGAIN){
            close(fd);
            printf("process is running already, terminate!\n");
            exit(EXIT_FAILURE);
        }
        printf("fnctl set file fd fail\n");
        close(fd);
        exit(1);
    }
    ftruncate(fd, 0);
    char pidstr[16];
    sprintf(pidstr, "%ld", (long) getpid());
    write(fd, pidstr, strlen(pidstr));
}
void pidfile_deinit(const char *pid_file)
{
    if(pid_file){
        unlink(pid_file);
    }
}

void cleanup()
{
}


static void sig_exit_func(int sig, void *mio_data, void *data)
{
    mio_data_t *mdata = (mio_data_t *) mio_data;
    sedbg("[main]got signal:[%d]\n", sig);
    mdata->terminate = 1;
}

static void sig_pipe_func(int sig, void *mio_data, void *data)
{
    mio_data_t *mdata = (mio_data_t *) mio_data;
    sedbg("[main]got sigpipe, client leave:[%d]\n", sig);
}

static void sig_chld_func(int sig, void *mio_data, void *data)
{
    int pid, status;
    mio_data_t *mdata = (mio_data_t *) mio_data;

    while (((pid = waitpid(-1, &status, WNOHANG)) != -1) && (pid != 0)) {
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                sedbg("Exit Code: %d\n", WEXITSTATUS(status));
                sig = SIGABRT; // Child process return non-zero status
            }else {
                sedbg("Child exit, exit Code: 0x%.4X pid:%d\n", status, pid);
	    }
        } else if (WIFSIGNALED(status)) {
            sig = WTERMSIG(status);
            sedbg("Child process %d terminated with signal %d", pid, sig);
        }
    }
}



void register_ipc(mio_data_t *mio_data, cmdipc_t *cmdipc)
{
    cmdipc_t *ptr;
    list_for_each_entry(ptr, &(cmdipc->list),list) {
        if (ptr->ipc_sockfunc){
            ptr->fd = ptr->ipc_sockfunc(ptr->data); // tcp_srv_data_t
        }
        if (ptr->ipc_recvfunc){
            if (ptr->fd > 0)
                add_event(&(mio_data->read_pool), ptr->fd, (event_func)ptr->ipc_recvfunc, ptr->data);
        }
    }
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

void init_thread(thread_t *thread_pool, mio_data_t *mdata)
{
#if THREAD_LINKMON
    add_thread(thread_pool, "linkmon", linkmon_start, (void *)mdata);
#endif
}

void uninit_thread(thread_t *thread_pool)
{
#if THREAD_LINKMON
    remove_thread(thread_pool, find_thread_by_name(thread_pool, "linkmon"));
#endif
}

void register_thread(mio_data_t *mio_data, thread_t *thread)
{
    pthread_attr_t attr;
    thread_t *ptr;
    if (pthread_attr_init(&attr)!=0){
        sedbg("create thread attr error\n");
        return;
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)!=0){
        perror("thread set attr detach error\n");
        return;
    }

    list_for_each_entry(ptr, &(thread->list),list) {
        if (!ptr->thread_start_func) 
            continue;
        if (pthread_create(&(ptr->thread_id), &attr, ptr->thread_start_func, ptr->data) != 0){
            sedbg("create thread:[%s] fail\n", ptr->name);
            continue;
        }
    }

    if (pthread_attr_destroy(&attr) != 0){
        sedbg("destroy thread attr error\n");
        return;
    }
}

void unregister_thread(mio_data_t *mio_data, thread_t *thread)
{
    thread_t *ptr;
    void *res;
    int s = 0;
    int pthread_kill_err;
    list_for_each_entry(ptr, &(thread->list),list) {
        pthread_kill_err = pthread_kill(ptr->thread_id, SIGUSR1);
        if (pthread_kill_err == ESRCH)
            sedbg("[%s] thread not exist\n", ptr->name);
        else if (pthread_kill_err == EINVAL)
            sedbg("[%s] thread signal invalid\n", ptr->name);
        else 
            sedbg("[%s] thread terminal signal sent\n", ptr->name);
    }
}

void init_action(action_t *action_pool, gdata_t *gdata)
{
#if ACT_SAMPLE
    add_action(action_pool, "act_sample", act_sample, ACT_QUICK);
#endif
#if ACT_SELFTEST
    add_action(action_pool, "act_selftest", act_selftest, ACT_QUICK);
#endif
#if ACT_WIFI_CLINFO
    add_action(action_pool, "act_wifi_clinfo", act_wifi_clinfo, ACT_QUICK);
#endif
#if ACT_WS_WIFI_CLINFO
    add_action(action_pool, "act_ws_wifi_clinfo", act_ws_wifi_clinfo, ACT_QUICK);
#endif
#if ACT_SITESURVEY
    add_action(action_pool, "act_sitesurvey", act_sitesurvey, ACT_NORMAL);
#endif
#if ACT_LINKCHECK
    add_action(action_pool, "act_linkcheck", act_linkcheck, ACT_NORMAL);
    add_action(action_pool, "act_check_phylink", act_check_phylink, ACT_NORMAL);
    add_action(action_pool, "act_check_meshap", act_check_meshap, ACT_NORMAL);
    add_action(action_pool, "act_check_arpdup", act_check_arpdup, ACT_NORMAL);
    add_action(action_pool, "act_check_icmp", act_check_icmp, ACT_NORMAL);
    add_action(action_pool, "act_check_dns", act_check_dns, ACT_NORMAL);
    add_action(action_pool, "act_check_http", act_check_http, ACT_NORMAL);
    add_action(action_pool, "act_check_https", act_check_https, ACT_NORMAL);
#endif
#if ACT_PSSCAN
    add_action(action_pool, "act_psscan", act_psscan, ACT_QUICK);
#endif
#if ACT_RUNCMD
    add_action(action_pool, "act_runcmd", act_runcmd, ACT_NORET);
#endif
#if ACT_RUNLIB_SNLOG
    add_action(action_pool, "act_runlib_snlog", act_runlib_snlog, ACT_NORET);
#endif
}

void uninit_action(action_t *action_pool)
{
#if ACT_SAMPLE
    remove_action_by_name(action_pool, "act_sample");
#endif
#if ACT_SELFTEST
    remove_action_by_name(action_pool, "act_selftest");
#endif
#if ACT_WIFI_CLINFO
    remove_action_by_name(action_pool, "act_wifi_clinfo");
#endif
#if ACT_WS_WIFI_CLINFO
    remove_action_by_name(action_pool, "act_ws_wifi_clinfo");
#endif
#if ACT_SITESURVEY
    remove_action_by_name(action_pool, "act_sitesurvey");
#endif
#if ACT_LINKCHECK
    remove_action_by_name(action_pool, "act_linkcheck");
    remove_action_by_name(action_pool, "act_check_phylink");
    remove_action_by_name(action_pool, "act_check_meshap");
    remove_action_by_name(action_pool, "act_check_arpdup");
    remove_action_by_name(action_pool, "act_check_icmp");
    remove_action_by_name(action_pool, "act_check_dns");
    remove_action_by_name(action_pool, "act_check_http");
    remove_action_by_name(action_pool, "act_check_https");
#endif
#if ACT_PSSCAN
    remove_action_by_name(action_pool, "act_psscan");
#endif
#if ACT_RUNCMD
    remove_action_by_name(action_pool, "act_runcmd");
#endif
#if ACT_RUNLIB_SNLOG
    remove_action_by_name(action_pool, "act_runlib_snlog");
#endif
}

void show_usage()
{
	printf("%s usage:\n" \
" -s               preload config file path, default /etc/syseye\n" \
" -d               daemonlize this process\n" \
" -h               show this help\n", __FILE__);
}

void init_storage(gdata_t *gdata, char *storage_path)
{
    char cfg_path[128];
    if (gdata == NULL){ 
        sedbg("%s(%d) gdata null\n", __func__, __LINE__);
        return;
    }
    sprintf(cfg_path, "%s/%s", storage_path, "cfg_saved.json");
    gdata->cfg_saved = js_parse_file(cfg_path);
    gdata->cfg_runtime = js_dup(gdata->cfg_saved);
    sedbg("%s(%d) config load: %s\n", __func__, __LINE__, cfg_path);
    sprintf(cfg_path, "%s/%s", storage_path, "cfg_ram.json");
    gdata->cfg_ram = js_parse_file(cfg_path);
    sedbg("%s(%d) config load: %s\n", __func__, __LINE__, cfg_path);
}
void uninit_storage(gdata_t *gdata)
{
    js_free(gdata->cfg_runtime);
    js_free(gdata->cfg_saved);
    js_free(gdata->cfg_ram);
}

int main(int argc, char *argv[])
{
    mio_data_t *mio_data;
    cmdipc_t *cmdipc;
    thread_t *thread;
    action_t *action;
    gdata_t gdata;
    char storage_path[128] = "/etc/syseye";

    int daemonize = 0, c;
    while ((c = getopt(argc, argv,"dhs:")) != -1){
        switch(c) {
            case 'h':
                show_usage();
                return 0;
	    case 'd':
                daemonize = 1;
                break;
            case 's':
                sscanf(optarg,"%s", storage_path);
                break;
            default:
                //exit(EXIT_FAILURE);
                break;
        }
    }

    if (daemonize == 1){
        daemon(0, 0);
    }
    pidfile_init(PID_FILE);
    atexit(cleanup);

    memset(&gdata, 0, sizeof(gdata_t));

    init_storage(&gdata, storage_path);
    strcpy(gdata.storage_path, storage_path);
    mio_data = mio_init();
    gdata.mio_data = mio_data;

    add_signal(mio_data->signal_pool, SIGINT,sig_exit_func, NULL);
    add_signal(mio_data->signal_pool, SIGHUP,sig_exit_func, NULL);
    add_signal(mio_data->signal_pool, SIGTERM,sig_exit_func, NULL);
    add_signal(mio_data->signal_pool, SIGPIPE,sig_pipe_func, NULL);
    add_signal(mio_data->signal_pool, SIGCHLD,sig_chld_func, NULL);

    cmdipc = cmdipc_init();
    // init_ipc
#if IPC_UNIX
    init_ipc_tcp(cmdipc, cmdhandler, (void *) &gdata);
#endif
#if IPC_NL
    init_ipc_nl(cmdipc, nl_cmdhandler, (void *) &gdata);
#endif
    thread = thread_init();
    init_thread(thread, mio_data);
    action = action_init();
    gdata.action = action;
    init_action(action, &gdata);
    register_ipc(mio_data, cmdipc);
    register_thread(mio_data, thread);

    mio_loop(mio_data);

    unregister_ipc(mio_data, cmdipc);
    unregister_thread(mio_data, thread);
    // uninit_ipc
#if IPC_UNIX
    uninit_ipc_tcp(cmdipc);
#endif
#if IPC_NL
    uninit_ipc_nl(cmdipc);
#endif
    uninit_thread(thread);
    thread_uninit(thread);
    uninit_action(action);
    action_uninit(action);
    cmdipc_uninit(cmdipc);
    sleep(2); // wait child threads terminate
    mio_uninit(mio_data);
    uninit_storage(&gdata);
    sedbg("gracefully end\n");

    pidfile_deinit(PID_FILE);
    return 0;
}
