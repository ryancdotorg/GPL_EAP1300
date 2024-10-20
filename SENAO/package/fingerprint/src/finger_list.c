#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/sysinfo.h>
/* #include <knlintf.h> */
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "finger_list.h"

extern struct list_head maclistHead;
extern struct list_head iplistHead;

void finger_list_init_node(struct finger_cfg_list *arg)
{
    memset(arg, 0, sizeof(struct finger_cfg_list));
}

struct finger_cfg_list * finger_list_find(unsigned char *mac, struct list_head *head)
{
    struct list_head *iter;
    static struct finger_cfg_list *objPtr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct finger_cfg_list, list_member);
        if(memcmp(objPtr->data.mac, mac, 6)==0) {
            return objPtr;
        }
    }
    return 0;
}

struct finger_cfg_list *finger_list_add_node(struct finger_cfg_list *arg, struct list_head *head)
{
    struct finger_cfg_list *fooPtr = (struct finger_cfg_list *)malloc(sizeof(struct finger_cfg_list));

    assert(fooPtr != NULL);
    if(fooPtr == NULL)
    {
        return NULL;
    }

    memcpy(fooPtr, arg, sizeof(struct finger_cfg_list));
    INIT_LIST_HEAD(&fooPtr->list_member);
    list_add(&fooPtr->list_member, head);
    return fooPtr;
}

int finger_list_delete(unsigned char *mac, struct list_head *head)
{
    struct list_head *iter;
    struct finger_cfg_list *objPtr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct finger_cfg_list, list_member);
        if(memcmp(objPtr->data.mac, mac, 6)==0) {
            list_del(&objPtr->list_member);
            free(objPtr);
            return 1;
        }
    }
    return 0;
}

void finger_list_delete_all(struct list_head *head)
{
    struct list_head *iter;
    struct finger_cfg_list *objPtr;

redo:
    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct finger_cfg_list, list_member);
        list_del(&objPtr->list_member);
        free(objPtr);
        goto redo;
    }
}

void finger_list_display(struct list_head *head)
{
    struct list_head *iter;
    struct finger_cfg_list *objPtr;
    struct in_addr node_ip_addr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct finger_cfg_list, list_member);
        printf("\n=============Node==================\n");
        printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n", \
                objPtr->data.mac[0], objPtr->data.mac[1], objPtr->data.mac[2], \
                objPtr->data.mac[3], objPtr->data.mac[4], objPtr->data.mac[5]);
        printf("update_time:%lld\n", objPtr->data.update_time);
        node_ip_addr.s_addr = objPtr->data.nodeinfo.ipaddr;
        printf("ip:%s ", inet_ntoa(node_ip_addr));
        printf("system:%s ", objPtr->data.nodeinfo.system);
        printf("device:%s ", objPtr->data.nodeinfo.device);
        printf("isOccupy:%d ", objPtr->data.nodeinfo.isOccupy);
        printf("\n=============Node END===============\n");
    }
}

void ip_list_init_node(struct ip_cfg_list *arg)
{
    memset(arg, 0, sizeof(struct ip_cfg_list));
}

struct ip_cfg_list * ip_list_find(unsigned char *mac, struct list_head *head)
{
    struct list_head *iter;
    static struct ip_cfg_list *objPtr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct ip_cfg_list, list_member);
        if(memcmp(objPtr->data.mac, mac, 6)==0) {
            return objPtr;
        }
    }
    return 0;
}

struct ip_cfg_list *ip_list_add_node(struct ip_cfg_list *arg, struct list_head *head)
{
    struct ip_cfg_list *fooPtr = (struct ip_cfg_list *)malloc(sizeof(struct ip_cfg_list));

    assert(fooPtr != NULL);
    if(fooPtr == NULL)
    {
        return NULL;
    }

    memcpy(fooPtr, arg, sizeof(struct ip_cfg_list));
    INIT_LIST_HEAD(&fooPtr->list_member);
    list_add(&fooPtr->list_member, head);
    return fooPtr;
}

int ip_list_delete(unsigned char *mac, struct list_head *head)
{
    struct list_head *iter;
    struct ip_cfg_list *objPtr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct ip_cfg_list, list_member);
        if(memcmp(objPtr->data.mac, mac, 6)==0) {
            list_del(&objPtr->list_member);
            free(objPtr);
            return 1;
        }
    }
    return 0;
}

void ip_list_delete_all(struct list_head *head)
{
    struct list_head *iter;
    struct ip_cfg_list *objPtr;

redo:
    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct ip_cfg_list, list_member);
        list_del(&objPtr->list_member);
        free(objPtr);
        goto redo;
    }
}

void ip_list_display(struct list_head *head)
{
    struct list_head *iter;
    struct ip_cfg_list *objPtr;
    struct in_addr offeraddr, gatewayaddr, maskaddr;

    __list_for_each(iter, head) {
        objPtr = list_entry(iter, struct ip_cfg_list, list_member);
        printf("\n=============Node==================\n");
        offeraddr.s_addr = objPtr->data.ipnodeinfo.offer_ip;
        gatewayaddr.s_addr = objPtr->data.ipnodeinfo.gateway_ip;
        maskaddr.s_addr = objPtr->data.ipnodeinfo.mask_ip;
        printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n", \
                objPtr->data.mac[0], objPtr->data.mac[1], objPtr->data.mac[2], \
                objPtr->data.mac[3], objPtr->data.mac[4], objPtr->data.mac[5]);
        printf("update_time:%lld\n", objPtr->data.update_time);
        printf("offeraddr:%s ", inet_ntoa(offeraddr));
        printf("gatewayaddr:%s ", inet_ntoa(gatewayaddr));
        printf("maskaddr:%s ", inet_ntoa(maskaddr));
        printf("dhcp_mac:%02x:%02x:%02x:%02x:%02x:%02x\n", \
                objPtr->data.ipnodeinfo.dhcp_mac[0], objPtr->data.ipnodeinfo.dhcp_mac[1], objPtr->data.ipnodeinfo.dhcp_mac[2], \
                objPtr->data.ipnodeinfo.dhcp_mac[3], objPtr->data.ipnodeinfo.dhcp_mac[4], objPtr->data.ipnodeinfo.dhcp_mac[5]);
        printf("gateway_mac:%02x:%02x:%02x:%02x:%02x:%02x\n", \
                objPtr->data.ipnodeinfo.gateway_mac[0], objPtr->data.ipnodeinfo.gateway_mac[1], objPtr->data.ipnodeinfo.gateway_mac[2], \
                objPtr->data.ipnodeinfo.gateway_mac[3], objPtr->data.ipnodeinfo.gateway_mac[4], objPtr->data.ipnodeinfo.gateway_mac[5]);
        printf("\n=============Node END===============\n");
    }
}


