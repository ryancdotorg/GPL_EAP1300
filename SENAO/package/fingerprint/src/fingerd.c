/*****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project :
;    Creator :
;    File    :
;    Abstract:
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       Yolin          2017-08-01
;*****************************************************************************/

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include "eloop.h"
#include "ctrl_iface.h"
#include "finger.h"
#include "fingerhandle.h"
#include "finger_list.h"
#include "fingerprint.h"
LIST_HEAD(maclistHead);
LIST_HEAD(iplistHead);

/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/
extern int fingersync_debug_level;
extern int DebugEnable;
extern int ifcount;
extern unsigned int mon_ifindex[32];

/* Interface */
char* fingersync_interface = "br-lan";

/* initial callback function */
typedef int (*fingersync_init_callback_f)(struct fingersyncd_data *syscd);
/* de-initial callback function */
typedef void (*fingersync_deinit_callback_f)(struct fingersyncd_data *syscd);

struct fingersyncd_interface_t
{
    fingersync_init_callback_f init;
    fingersync_deinit_callback_f deinit;
};

/* interface list */
struct fingersyncd_interface_t fingersyncd_intf_list[] =
{
    {fingersyncd_cli_init,           fingersyncd_cli_deinit},
    //{fingersyncd_ctrl_data_init,     fingersyncd_ctrl_data_deinit},
    {fingersyncd_fingerprint_init,   fingersyncd_fingerprint_deinit},
    {fingerprint_init,               fingerprint_deinit},
};

/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/

/*****************************************************************
* NAME:    handle_reload
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static void handle_reload(int sig, void *eloop_ctx, void *signal_ctx)
{
    printf("Signal %d received - reloading configuration\n", sig);
}

/*****************************************************************
* NAME:    handle_term
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static void handle_term(int sig, void *eloop_ctx, void *signal_ctx)
{
    printf("Signal %d received - terminating\n", sig);
    eloop_terminate();
}

/*****************************************************************
* NAME:    handle_dump_state
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static void fingerprint_dump_state(int sig, void *eloop_ctx, void *signal_ctx)
{
    dumpFingerData();
    dumpFingerDataToFile(FINGERPRINT_DB_FILE_PATH, 0);
}

static void fingerprint_debug_enable(int sig, void *eloop_ctx, void *signal_ctx)
{
    DebugEnable = !DebugEnable;
    fp_debug("debug: %d", DebugEnable);
}

/*****************************************************************
* NAME:    fingersyncd_init
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static struct fingersyncd_data * fingersyncd_init(const char *config_file)
{
    struct fingersyncd_data *syscd;

    syscd = malloc(sizeof(*syscd));
    if(syscd == NULL)
    {
        printf("Could not allocate memory for fingersyncd data\n");

        return NULL;
    }
    memset(syscd, 0, sizeof(*syscd));

    return syscd;
}

/*****************************************************************
* NAME:    fingersyncd_cleanup_iface
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static void fingersyncd_cleanup_iface(struct fingersyncd_data *syscd)
{
    free(syscd);
}

/*****************************************************************
* NAME:    usage
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void usage(int rc)
{
    printf("usage: fingerprint [-h]\n"
            "\t-h\t\tthis help\n"
            "\t-d\t\tDebug Level\n"
            "\t-i\t\tInterface\n"
            "\n");
    printf("SIGUSR1 to list fingerData and output to fingerprint_db.txt\nSIGUSR2 to enable debug mode\n\n");
    printf("command(use mac address to get ip from other device): \nfinger_syncli send request \"clientmac\" update\n\n");
    printf("command(use mac address to get gateway from other device): \nfinger_syncli send request \"clientmac\" renew\n\n");
    printf("command(add gateway mac address for arp_scan.sh): \nfinger_syncli set data \"clientmac\" \"gatewaymac\" add\n\n");
    printf("command(add vlan info from hostapd): \nfinger_syncli set vlanInfo \"clientmac\" \"vlanid\" \"ifname\" add\n\n");
    printf("Ps. Format of MAC Address: xxxxxxxxxxxx\n");
    exit(rc);
}

/*****************************************************************
* NAME:    main
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2011-10-25
* Modify:
******************************************************************/
int main(int argc, char *argv[])
{
    fingersyncd_interfaces interfaces;
    int ret = 1, i;
    int c;

    for(;;)
    {
        c = getopt(argc, argv, "di:h");
        if(c < 0)
            break;
        switch(c)
        {
            case 'd':
                DebugEnable = 1;
#ifdef CONFIG_NO_STDOUT_DEBUG
                /* no debug log */
#else
                if(fingersync_debug_level > 0)
                    fingersync_debug_level--;
#endif
                break;
            case 'i':
                fingersync_interface = strdup(optarg);
                mon_ifindex[ifcount] = if_nametoindex(optarg);
                ++ifcount;
                break;
            case 'h':
                usage(0);
                break;
            default:
                break;
        }
    }

    interfaces.count = sizeof(fingersyncd_intf_list)/sizeof(fingersyncd_intf_list[0]);

    interfaces.syscd = malloc(interfaces.count * sizeof(struct fingersyncd_data *));
    if(interfaces.syscd == NULL)
    {
        printf("malloc failed\n");
        exit(1);
    }

    eloop_init(&interfaces);

    eloop_register_signal(SIGHUP, handle_reload, NULL);
    eloop_register_signal(SIGINT, handle_term, NULL);
    eloop_register_signal(SIGTERM, handle_term, NULL);
    eloop_register_signal(SIGUSR1, fingerprint_dump_state, NULL);
    eloop_register_signal(SIGUSR2, fingerprint_debug_enable, NULL);

    fingersync_printf(FINGER_DEBUG,"FINGERD_CTRL_INTERFACE:[%s]  FINGERD_CTRL_INTERFACE_IF:[%s]\n",
            FINGERD_CTRL_INTERFACE, FINGERD_CTRL_INTERFACE_IF);

    for(i = 0; i < interfaces.count; i++)
    {
        interfaces.syscd[i] = fingersyncd_init(0);

        if(!interfaces.syscd[i])
            goto out;
        if(fingersyncd_intf_list[i].init(interfaces.syscd[i]) < 0)
            goto out;
    }

    eloop_run();

    ret = 0;

out:
    /* Deinitialize all interfaces */
    for(i = 0; i < interfaces.count; i++)
    {
        if(!interfaces.syscd[i])
            continue;

        fingersyncd_intf_list[i].deinit(interfaces.syscd[i]);
        fingersyncd_cleanup_iface(interfaces.syscd[i]);
    }

    free(interfaces.syscd);
    free(fingersync_interface);

    eloop_destroy();
    finger_list_delete_all(&maclistHead);

    return ret;
}

