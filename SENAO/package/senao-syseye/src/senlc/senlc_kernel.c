#include <linux/module.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include "../nlc.h"

#define SENLC_REGISTER 17
#define SENLC_SELFTEST 20
#define NETLINK_SYSEYE NETLINK_USERSOCK
//#define NETLINK_SYSEYE 30
#define FRAME_SIZE 1024

int _senlc_action(const char *func, int line, char *name, int delay, char *fmt, ...);


typedef enum {
    TYPE_VALUE_PATH=0,
    TYPE_VALUE_STR,
    TYPE_VALUE_INT,
    TYPE_VALUE_DOUBLE,
} type_value_t;

struct sock *nl_sk = NULL;
static int user_pid = -1;
static int senlc_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
    void *data;
    switch (nlh->nlmsg_type) {
        case SENLC_REGISTER:
            user_pid = nlh->nlmsg_pid;
            printk(KERN_INFO "[senlc_rcv_msg] connect to process %d\n", user_pid);
	    return 0;
        case SENLC_SELFTEST:
            if (user_pid > 0){
                data = NLMSG_DATA(nlh);
                //printk(KERN_INFO "[senlc_rcv_msg] get selftest request: [%s]\n", (char *)data);
                _senlc_action(__func__,__LINE__, "act_runcmd", 0, data);
	    return 0;
            }
        default:
            printk(KERN_INFO "[senlc_msg] nothing hannened\n");
            return -EINVAL;
    }
}

static DEFINE_MUTEX(senlc_rcv_mutex);
static void senlc_rcv(struct sk_buff *skb)
{
    int ret = 0;
    mutex_lock(&senlc_rcv_mutex);
    ret = netlink_rcv_skb(skb, &senlc_rcv_msg);
    mutex_unlock(&senlc_rcv_mutex);
}

void senlc_send(char *msg, int msg_size)
{
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int sent = 0;
    int len;
    int i = 0;
    int res;
    while (msg_size > 0){
        len = (msg_size>FRAME_SIZE)?FRAME_SIZE:msg_size;
        skb_out = nlmsg_new(len, 0);
        if (!skb_out) {
            printk(KERN_ERR "Failed to allocate new skb\n");
            return;
        }
        if (msg_size > FRAME_SIZE) // multipart
            nlh = nlmsg_put(skb_out, 0, i, NLM_F_MULTI, len, 0);
	else
            nlh = nlmsg_put(skb_out, 0, i, NLMSG_DONE, len, 0);
        NETLINK_CB(skb_out).dst_group = 0; // not in mcast group 
        memcpy(NLMSG_DATA(nlh), msg + sent, len);
        res = nlmsg_unicast(nl_sk, skb_out, user_pid);
        sent += len;
        msg_size-=len;
        i++;
    }
}

int _senlc_set_value(const char *func, int line, storage_t storage, char *path, void *value ,type_value_t t)
{
    nl_cmd_t nl_cmd;
    nl_set_data_t nl_set_data;
    char *buff = NULL;
    int len_value;
    char int_string[12];

    snprintf(nl_cmd.func, sizeof(nl_cmd.func),"%s", func);
    nl_cmd.line = line;
    nl_cmd.type = CMD_SET;

    nl_set_data.storage = storage;
    snprintf(nl_set_data.path, sizeof(nl_set_data.path),"%s", path);
    // get value len
    switch (t){
        case TYPE_VALUE_PATH:
           len_value = 0;		
           break;		
        case TYPE_VALUE_STR:
           len_value = strlen((char *)value) + 2; // 2 \"
           break;		
        case TYPE_VALUE_INT:
           len_value = sprintf(int_string, "%d", *((int *)value));
           break;		
        case TYPE_VALUE_DOUBLE:
           break;
	default:
	   break;
    }
    // alloc
    buff = kzalloc(sizeof(nl_cmd_t)+sizeof(nl_set_data)+ len_value + 1, GFP_KERNEL); // 1 for \0''
    if(!buff)
    {
        printk(KERN_ALERT "syseye kzalloc error.\n");
        return 0;
    }

    memcpy(buff, &nl_cmd, sizeof(nl_cmd_t));
    memcpy(buff + sizeof(nl_cmd_t), &nl_set_data, sizeof(nl_set_data));
    // append value
    switch (t){
        case TYPE_VALUE_PATH:
           break;		
        case TYPE_VALUE_STR:
           sprintf(buff+ sizeof(nl_cmd_t) + sizeof(nl_set_data), "\"%s\"", (char *)value);
           break;		
        case TYPE_VALUE_INT:
	   memcpy(buff+ sizeof(nl_cmd_t) + sizeof(nl_set_data), int_string, len_value);
           break;		
        case TYPE_VALUE_DOUBLE:
           break;
	default:
	   break;
    }

    senlc_send(buff, sizeof(nl_cmd_t) + sizeof(nl_set_data) + len_value);
    kfree(buff);
    return 0;
}

int _senlc_set_path(const char *func, int line, storage_t storage, char *path)
{
    _senlc_set_value(func, line, storage, path, NULL, TYPE_VALUE_PATH);
    return 0;
}

int _senlc_set_str(const char *func, int line, storage_t storage, char *path, char *value)
{
    _senlc_set_value(func, line, storage, path, (void *)value, TYPE_VALUE_STR);
    return 0;
}

int _senlc_set_int(const char *func, int line, storage_t storage, char *path, int value)
{
    _senlc_set_value(func, line, storage, path, (void *)&value, TYPE_VALUE_INT);
    return 0;

}

int _senlc_prune(const char *func, int line, storage_t storage, char *path)
{
    nl_cmd_t nl_cmd;
    nl_prune_data_t nl_prune_data;
    char *buff;

    snprintf(nl_cmd.func, sizeof(nl_cmd.func),"%s", func);
    nl_cmd.line = line;
    nl_cmd.type=CMD_PRUNE;

    nl_prune_data.storage = storage;
    snprintf(nl_prune_data.path, sizeof(nl_prune_data.path),"%s", path);
    // alloc
    buff = kzalloc(sizeof(nl_cmd_t)+sizeof(nl_prune_data), GFP_KERNEL);
    if(!buff)
    {
        printk(KERN_ALERT "syseye kzalloc error.\n");
        return 0;
    }
    memcpy(buff, &nl_cmd, sizeof(nl_cmd_t));
    memcpy(buff + sizeof(nl_cmd_t), &nl_prune_data, sizeof(nl_prune_data));

    senlc_send(buff, sizeof(nl_cmd_t) + sizeof(nl_prune_data));
    kfree(buff);
    return 0;
}

int _vsenlc_action(const char *func, int line, char *name, int delay, char *fmt)
{	
    nl_cmd_t nl_cmd;
    nl_act_data_t nl_act_data;
    char *buff;
//    va_list ap;
    int len_cmd = 0;
    char cmd[912];

    snprintf(nl_cmd.func, sizeof(nl_cmd.func),"%s", func);
    nl_cmd.line = line;
    nl_cmd.type = CMD_ACTION;

    snprintf(nl_act_data.name, sizeof(nl_act_data.name),"%s", name);
    nl_act_data.delay = delay;

    // get command len
    memset(cmd, 0, sizeof(cmd));

    len_cmd += snprintf(cmd, sizeof(cmd), "%s", fmt);

    // alloc
    buff = kzalloc(sizeof(nl_cmd_t) + sizeof(nl_act_data) + len_cmd + 1, GFP_KERNEL); // 1 for \0''
    if(!buff)
    {
        printk(KERN_ALERT "syseye kzalloc error.\n");
        return 0;
    }

    memcpy(buff, &nl_cmd, sizeof(nl_cmd_t));
    memcpy(buff + sizeof(nl_cmd_t), &nl_act_data, sizeof(nl_act_data));
    // append cmd
    memcpy(buff+ sizeof(nl_cmd_t) + sizeof(nl_act_data), cmd, len_cmd);
    senlc_send(buff, sizeof(nl_cmd_t) + sizeof(nl_act_data) + len_cmd);
    kfree(buff);
    return 0;
}

int _senlc_action(const char *func, int line, char *name, int delay, char *fmt, ...)
{
    int r;

    r = _vsenlc_action(func, line, name, delay, fmt);

    return r;
}


void nlh_system_cmd(char *msg)
{
    _senlc_action(__func__,__LINE__, "act_runcmd", 0, msg);
}

static int __init senlc_init(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,6,0)
    struct netlink_kernel_cfg cfg = {
        .input = senlc_rcv,
    };
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,14,42)
    nl_sk = netlink_kernel_create(&init_net, NETLINK_SYSEYE, &cfg);
#elif LINUX_VERSION_CODE > KERNEL_VERSION(3,6,0)
    nl_sk = netlink_kernel_create(&init_net, NETLINK_SYSEYE, THIS_MODULE, &cfg);
#endif
#else
    nl_sk = netlink_kernel_create(&init_net, NETLINK_SYSEYE, 0, senlc_rcv, NULL, THIS_MODULE);
#endif
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -1;
    }
    printk(KERN_INFO "senlc initialized: %s\n", __FUNCTION__);
    return 0;
}
static void __exit senlc_exit(void)
{
    printk(KERN_INFO "exiting nlh module\n");
    netlink_kernel_release(nl_sk);
    nl_sk = NULL;
}

module_init(senlc_init);
module_exit(senlc_exit);

EXPORT_SYMBOL(senlc_send);
EXPORT_SYMBOL(_senlc_set_path);
EXPORT_SYMBOL(_senlc_set_str);
EXPORT_SYMBOL(_senlc_set_int);
EXPORT_SYMBOL(_senlc_prune);
EXPORT_SYMBOL(_senlc_action);
EXPORT_SYMBOL(_vsenlc_action);
// old version compatible
EXPORT_SYMBOL(nlh_system_cmd);

MODULE_DESCRIPTION("SenaoNetwork");
MODULE_AUTHOR("SenaoNetwork");
MODULE_LICENSE("Proprietary");

