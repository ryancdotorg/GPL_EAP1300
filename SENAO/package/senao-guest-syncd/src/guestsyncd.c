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
#include <eloop.h>
#include <ctrl_iface.h>
#include <guestsyncd.h>
#include <guestsync.h>
#include <guestsynchandle.h>
#include "guestsync_list.h"
LIST_HEAD(maclistHead);

/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/
extern int guestsync_debug_level;

/* Interface */
char* guestsync_interface = NULL;
char guestsync_group[32] = {0};

/* initial callback function */
typedef int (*guestsync_init_callback_f)(struct guestsyncd_data *syscd);
/* de-initial callback function */
typedef void (*guestsync_deinit_callback_f)(struct guestsyncd_data *syscd);

struct guestsyncd_interface_t
{
    guestsync_init_callback_f init;
    guestsync_deinit_callback_f deinit;
};

/* interface list */
struct guestsyncd_interface_t guestsyncd_intf_list[] =
{
    {guestsyncd_cli_init,           guestsyncd_cli_deinit},
    {guestsyncd_ctrl_data_init,     guestsyncd_ctrl_data_deinit},
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
static void handle_dump_state(int sig, void *eloop_ctx, void *signal_ctx)
{
}

/*****************************************************************
* NAME:    guestsyncd_init
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static struct guestsyncd_data * guestsyncd_init(const char *config_file)
{
    struct guestsyncd_data *syscd;

    syscd = malloc(sizeof(*syscd));
    if(syscd == NULL)
    {
        printf("Could not allocate memory for guestsyncd data\n");

        return NULL;
    }
    memset(syscd, 0, sizeof(*syscd));

    return syscd;
}

/*****************************************************************
* NAME:    guestsyncd_cleanup_iface
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static void guestsyncd_cleanup_iface(struct guestsyncd_data *syscd)
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
    printf("usage: guestsyncd [-h]\n"
            "\t-h\t\tthis help\n"
            "\t-d\t\tDebug Level\n"
            "\t-i\t\tInterface\n"
            "\t-g\t\tGroup string, must < 32\n"
            "\n");
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
    guestsyncd_interfaces interfaces;
    int ret = 1, i;
    int c;

    for(;;)
    {
        c = getopt(argc, argv, "di:g:h");
        if(c < 0)
            break;
        switch(c)
        {
            case 'd':
#ifdef CONFIG_NO_STDOUT_DEBUG
                /* no debug log */
#else
                if(guestsync_debug_level > 0)
                    guestsync_debug_level--;
#endif
                break;
            case 'i':
                guestsync_interface = strdup(optarg);
                break;
            case 'g':
                if(strlen(optarg)>32)
                {
                    usage(0);
                    break;
                }
                else
                {
                    memset(guestsync_group, 0 , 32);
                    memcpy(guestsync_group, optarg, strlen(optarg));
                }
                break;
            case 'h':
                usage(0);
                break;
            default:
                break;
        }
    }

    interfaces.count = sizeof(guestsyncd_intf_list)/sizeof(guestsyncd_intf_list[0]);

    interfaces.syscd = malloc(interfaces.count * sizeof(struct guestsyncd_data *));
    if(interfaces.syscd == NULL)
    {
        printf("malloc failed\n");
        exit(1);
    }

    eloop_init(&interfaces);

    eloop_register_signal(SIGHUP, handle_reload, NULL);
    eloop_register_signal(SIGINT, handle_term, NULL);
    eloop_register_signal(SIGTERM, handle_term, NULL);
    eloop_register_signal(SIGUSR1, handle_dump_state, NULL);


    guestsync_printf(GUESTSYNC_DEBUG,"GUESTSYNC_CTRL_INTERFACE:[%s]  GUESTSYNC_CTRL_INTERFACE_IF:[%s]\n",
            GUESTSYNC_CTRL_INTERFACE, GUESTSYNC_CTRL_INTERFACE_IF);

    for(i = 0; i < interfaces.count; i++)
    {
        interfaces.syscd[i] = guestsyncd_init(0);

        if(!interfaces.syscd[i])
            goto out;
        if(guestsyncd_intf_list[i].init(interfaces.syscd[i]) < 0)
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

        guestsyncd_intf_list[i].deinit(interfaces.syscd[i]);
        guestsyncd_cleanup_iface(interfaces.syscd[i]);
    }

    free(interfaces.syscd);
    free(guestsync_interface);

    eloop_destroy();
    guest_list_delete_all(&maclistHead);

    return ret;
}

