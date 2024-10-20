/* -*- linux-c -*-
 * sysctl_net_vlan.c: sysctl interface to net core subsystem.
 *
 * July 03, 2018.
 * Added /proc/sys/net/8021q directory entry (empty =) ). [MS]
 */

#include <linux/mm.h>
#include <linux/sysctl.h>
#include <linux/module.h>
#include <linux/socket.h>
#include <net/sock.h>
#include <linux/if_vlan.h>

struct ctl_table net_vlan_table[] = {
    {
        .procname	= "vlan_passthrough",
        .data		= &vlan_pass.vlan_passthrough,
        .maxlen		= sizeof(vlan_pass.vlan_passthrough),
        .mode		= 0644,
        .proc_handler	= &proc_dointvec,
    },
    {
        .procname	= "wds_ifname",
        .data		= vlan_pass.wds_ifname,
        .maxlen		= sizeof(vlan_pass.wds_ifname),
        .mode		= 0644,
        .proc_handler	= &proc_dostring,
    },
    { }
};

static __net_init int sysctl_vlan_net_init(struct net *net)
{
	return 0;
}

static __net_exit void sysctl_vlan_net_exit(struct net *net)
{
	struct ctl_table *tbl;

	tbl = net->vlan.sysctl_hdr->ctl_table_arg;
	unregister_net_sysctl_table(net->vlan.sysctl_hdr);
	kfree(tbl);
}

static __net_initdata struct pernet_operations sysctl_vlan_ops = {
	.init = sysctl_vlan_net_init,
	.exit = sysctl_vlan_net_exit,
};

static __init int sysctl_vlan_init(void)
{
	struct ctl_table_header *hdr;

	hdr = register_net_sysctl(&init_net, "net/8021q", net_vlan_table);

	if (!hdr) {
		//printk("%s[%d] hdr is null, return -ENOMEM <===.\n", __FUNCTION__, __LINE__);
		return -ENOMEM;
	}
	return register_pernet_subsys(&sysctl_vlan_ops);
}

fs_initcall(sysctl_vlan_init);

