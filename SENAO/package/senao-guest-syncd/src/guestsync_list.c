#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/sysinfo.h>
/* #include <knlintf.h> */
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "guestsync_list.h"

extern struct list_head maclistHead;

void guest_list_init_node(struct guest_cfg_list *arg)
{
    memset(arg, 0, sizeof(struct guest_cfg_list));
}

struct guest_cfg_list * guest_list_find(unsigned char *mac, char *dhcpif, unsigned char *random, struct list_head *head)
{
    struct list_head *iter;
    static struct guest_cfg_list *objPtr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct guest_cfg_list, list_member);
        if(memcmp(objPtr->data.mac, mac, 6)==0 && strcmp(objPtr->data.dhcpif, dhcpif)==0) {
            if(random == NULL)
            {
                return objPtr;
            }
            else if (memcmp(objPtr->data.random, random, 16)==0)
            {
                return objPtr;
            }
        }
    }
    return 0;
}

struct guest_cfg_list *guest_list_add_node(struct guest_cfg_list *arg, struct list_head *head)
{
    struct guest_cfg_list *fooPtr = (struct guest_cfg_list *)malloc(sizeof(struct guest_cfg_list));

    assert(fooPtr != NULL);
    if(fooPtr == NULL)
    {
        return NULL;
    }

    memcpy(fooPtr, arg, sizeof(struct guest_cfg_list));
    INIT_LIST_HEAD(&fooPtr->list_member);
    list_add(&fooPtr->list_member, head);
    return fooPtr;
}

int guest_list_delete(unsigned char *mac, char *dhcpif, unsigned char *random, struct list_head *head)
{
    struct list_head *iter;
    struct guest_cfg_list *objPtr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct guest_cfg_list, list_member);
        if(memcmp(objPtr->data.mac, mac, 6)==0 && strcmp(objPtr->data.dhcpif, dhcpif)==0 && memcmp(objPtr->data.random, random, 16)==0) {
            list_del(&objPtr->list_member);
            free(objPtr);
            return 1;
        }
    }

    return 0;
}

void guest_list_delete_all(struct list_head *head)
{
    struct list_head *iter;
    struct guest_cfg_list *objPtr;

redo:
    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct guest_cfg_list, list_member);
        list_del(&objPtr->list_member);
        free(objPtr);
        goto redo;
    }
}

void guest_list_display(struct list_head *head)
{
    struct list_head *iter;
    struct guest_cfg_list *objPtr;
    struct in_addr node_ip_addr;
    int i;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct guest_cfg_list, list_member);
        printf("\ntype:%d\t", objPtr->data.type);

        printf("random:");
        for(i=0; i<16; ++i)
            printf("%c", objPtr->data.random[i]);
        printf(" ");
        printf("dhcpif:%s ", objPtr->data.dhcpif);
        printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n", \
                objPtr->data.mac[0], objPtr->data.mac[1], objPtr->data.mac[2], \
                objPtr->data.mac[3], objPtr->data.mac[4], objPtr->data.mac[5]);
        printf("sessiontime:%u ", ntohl(objPtr->data.nodeinfo.sessiontime));
        printf("sessiontimeout:%u ", ntohl(objPtr->data.nodeinfo.sessiontimeout));
        printf("idletime:%u ", ntohl(objPtr->data.nodeinfo.idletime));
        printf("idletimeout:%u ", ntohl(objPtr->data.nodeinfo.idletimeout));
        printf("gonetime:%u \n", ntohl(objPtr->data.nodeinfo.gonetime));
        printf("input_packets:%u ", ntohl(objPtr->data.nodeinfo.input_packets));
        printf("output_packets:%u ", ntohl(objPtr->data.nodeinfo.output_packets));
        printf("input_octets:%u ", ntohl(objPtr->data.nodeinfo.input_octets));
        printf("output_octets:%u \n", ntohl(objPtr->data.nodeinfo.output_octets));
        printf("acct-interim:%u ", ntohl(objPtr->data.nodeinfo.interiminterval));
        printf("username:%s ", objPtr->data.nodeinfo.username);
        node_ip_addr.s_addr = objPtr->data.nodeinfo.ipaddr;
        printf("ip:%s ", inet_ntoa(node_ip_addr));
    }
}


