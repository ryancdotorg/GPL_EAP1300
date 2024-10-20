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
#include <stdio.h>
#include <dirent.h> 
#include <sys/stat.h>
//#include <string.h>
//#include <stdlib.h>
#include <unistd.h>
#include "mio.h"
#include "phylink.h"
#include "util.h"

int phylink_init(void *mdata, void *idata);
void phylink_close(void *data);
void phylink_send(void *data);
void phylink_recv(void *data);
int phylink_status(void *data);
char *phylink_getmsg(void *data);

static monitor_phylink_t monitor_phylink =
{
	.name = "phylink",
	.sec = 5,
	.usec = 0,
	.lm_initfunc = phylink_init,
	.lm_sendfunc = phylink_send,
	.lm_recvfunc = phylink_recv,
	.lm_statusfunc = phylink_status,
	.lm_msgfunc = phylink_getmsg,
	.lm_uninitfunc = phylink_close,
	.destIF = "br-lan",
};

static int is_folder_exist(char *path)
{
	struct stat sb;
	if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
		return 1;
	return 0;
}

static int read_link_name(char *path, char *name, int len)
{
	if (readlink(path, name, len) < 0){
		return -1;
	}
	else {
		return 0;
	}

}

static int is_if_physical(char *path)
{
	char name[64];
	if (read_link_name(path, name, sizeof(name)) != 0)
		return -1;
	if (strstr(name, "virtual")==NULL)
		return 1;
	else 
		return 0;
}

static int is_if_wifi(char *ifname)
{
	char path[48];
	// /sys/class/net/ath0/wireless/
	sprintf(path, "/sys/class/net/%s/wireless", ifname);
	if (!is_folder_exist(path)) 
		return 0;
	else 	
		return 1;
}

static int get_file_flag(char *path, char *result)
{
	FILE *fd;
	int flag;
	if ((fd = fopen(path, "r"))==NULL)
		return -1;
	if (fscanf(fd, "%s", result) <= 0){
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return 0;
}
static int is_if_up(char *ifname)
{
	char result[8];
	char path[48];
	sprintf(path, "/sys/class/net/%s/operstate", ifname);
	if ( get_file_flag(path, result) != 0)
		return -1;

	if (!strcmp(result, "up"))
		return 1;
	else if (!strcmp(result, "down"))
		return 0;
	else{
		sprintf(path, "/sys/class/net/%s/carrier", ifname);
		if ( get_file_flag(path, result) != 0)
			return -1;

		if (!strcmp(result, "1"))
			return 1;
		else if (!strcmp(result, "0"))
			return 0;
	}

	return -1;
}

struct if_pool_t {
	struct list_head list;
	char ifname[16];
};

static int add_if_pool(struct if_pool_t *if_pool, char *ifname)
{
	struct if_pool_t *tmp;
	tmp = (struct if_pool_t *)calloc(1, sizeof(struct if_pool_t));
	if(!tmp) {
		perror("calloc");
		return -1;
	}
	strcpy(tmp->ifname, ifname);
	list_add_tail( &(tmp->list), &((*if_pool).list));
}
static void remove_if_pool(struct if_pool_t *ifp)
{
        list_del(&(ifp->list));
}
static void traverse_if_pool(struct if_pool_t *ifp)
{
	struct if_pool_t *tmp;
	lmdbg("name list:\n");
	list_for_each_entry(tmp,&((*ifp).list),list) {
		lmdbg("%s ", tmp->ifname);
	}
	lmdbg("\n");
}

int phylink_init(void *mdata, void *data)
{
	if (mdata == NULL || data == NULL){
		lmdbg("init error\n");
		return -1;
	}
	mio_data_t *mio_data;
	phylink_data_t *phylinkdata;
	mio_data = (mio_data_t *) mdata;
	phylinkdata = (phylink_data_t *)data;
	phylinkdata->mio_data = (void *) mio_data;

	if (monitor_phylink.lm_sendfunc){
		mio_timer_t *mt;
		mt = add_timer(&(mio_data->timer_pool), monitor_phylink.sec, monitor_phylink.usec, (timer_func)monitor_phylink.lm_sendfunc, (void *)phylinkdata, 1);
		phylinkdata->mio_timer_snd = (void *) mt;
		return 0;
	}
	else {
		phylinkdata->status = PHYLINK_CREATE_ERR;
		return -1;
	}
}

void phylink_close(void *data)
{
	if (data == NULL){
		lmdbg("phylink_close, data not initialized\n");
		return;
	}
	phylink_data_t *phylinkdata;
	mio_event_t *mio_event;
	mio_timer_t *mio_timer;

	phylinkdata = (phylink_data_t *) data;
	mio_timer = (mio_timer_t *)phylinkdata->mio_timer_snd;
	if (mio_timer != NULL){
		remove_timer(mio_timer);;
		phylinkdata->mio_timer_snd = NULL;
	}

	if (phylinkdata->fd > 0)
	{
		close(phylinkdata->fd);
		phylinkdata->fd = 0;
	}
}

#if defined(TARGET_BOARD_ipq40xx)
static int check_phy_status(char *ifname)
{
	FILE *fp;
	int linkstatus = 0;
	int ret;
    if ((fp = fopen("/proc/LAN_STATE", "r")) == NULL)
		return -1;
	fscanf(fp, "%d", &linkstatus);
	if (linkstatus > 0)
		ret = 1;
	else 
		ret = 0;
//	lmdbg("linkstatus: [%d]\n", linkstatus);
	fclose(fp);
	return ret;
}
#else
int check_phy_status(char *ifname)
{
	char path[64];
	struct if_pool_t if_pool;
	INIT_LIST_HEAD(&if_pool.list);

	// 1. check if this if is bridge
	sprintf(path, "/sys/class/net/%s", ifname);
	if (!is_folder_exist(path))
		return -1;
	sprintf(path, "/sys/class/net/%s/brif", ifname);
	
	if (is_folder_exist(path)){ // 2.1 bridge, get interface to check
		DIR *d;
		struct dirent *dir;
		d = opendir(path);
		if (d) {
			while ((dir = readdir(d)) != NULL) {
				if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
					continue;
				if (is_if_wifi(dir->d_name)) // filter wireless if
					continue;
				if (!is_if_physical(dir->d_name)) // filter virtual if
					continue;
				add_if_pool(&if_pool, dir->d_name);
				//lmdbg("%s added\n", dir->d_name);
			}
			closedir(d);
		}
	}
	else {// 2.2 not bridge
		add_if_pool(&if_pool, ifname);
		//lmdbg("%s added\n", ifname);		
	}
	// 3. check status
	struct if_pool_t *tmp;
	list_for_each_entry(tmp,&(if_pool.list),list) {
		if (is_if_up(tmp->ifname) == 1){
			return 1;
		}
	}
	return 0;
}
#endif

void phylink_send(void *data)
{
	if (data == NULL){
		lmdbg("send data broken error\n");
		return;
	}
	int fd;
	int status;
	phylink_data_t *phylinkdata;
	mio_data_t *mio_data;
	phylinkdata = (phylink_data_t *)data;
	mio_data = (mio_data_t *)phylinkdata->mio_data;
	status = check_phy_status(phylinkdata->if_name);
	lmdbg("status:%d\n", status);
	if (status == 1)
		phylinkdata->status = STATUS_OK;
	else if (status == 0)
		phylinkdata->status = PHYLINK_DOWN;
	else
		phylinkdata->status = PHYLINK_UNKNOWN;
	return;
}
void phylink_recv(void *data)
{
}
int phylink_status(void *data)
{
	phylink_data_t *phylinkdata;
	if (data == NULL){
		return PHYLINK_DATA_ERR;
	}
	phylinkdata = (phylink_data_t *)data;
	return phylinkdata->status;
}

char *phylink_getmsg(void *data)
{
	return "";
}

void init_phylink(linkmon_t *linkmon_pool, char *wan_ifname)
{
	phylink_data_t *phylinkdata;
	phylinkdata = (phylink_data_t *)calloc(1,sizeof(phylink_data_t));
	if(!phylinkdata) {
		perror("calloc");
		return;
	}

	if (*wan_ifname != '\0')
		strcpy(phylinkdata->if_name, wan_ifname);
	else
		strcpy(phylinkdata->if_name, monitor_phylink.destIF);
	phylinkdata->status = LINK_INIT;

	monitor_phylink.linkmon = add_linkmon(linkmon_pool, monitor_phylink.name, monitor_phylink.sec, monitor_phylink.usec, monitor_phylink.lm_initfunc, monitor_phylink.lm_sendfunc, monitor_phylink.lm_recvfunc, monitor_phylink.lm_statusfunc, monitor_phylink.lm_msgfunc, monitor_phylink.lm_uninitfunc, (void *)phylinkdata);
}

void uninit_phylink(linkmon_t *linkmon_pool)
{
	linkmon_t *linkmon;
	if (!linkmon_pool) 
		return;

//	linkmon = find_linkmon_by_name(linkmon_pool, monitor_phylink.name); 
	linkmon = monitor_phylink.linkmon;
	if (linkmon == NULL)
		return;

	if (linkmon->data != NULL){ // phylink_data_t
		free(linkmon->data);
		linkmon->data = NULL;
	}
	remove_linkmon(linkmon_pool, linkmon);
	monitor_phylink.linkmon = NULL;
}
