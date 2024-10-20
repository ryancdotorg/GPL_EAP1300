#ifndef __NETNS_VLAN_H__
#define __NETNS_VLAN_H__

struct ctl_table_header;
struct prot_inuse;

struct netns_vlan {
    /* vlan sysctls */
    struct ctl_table_header	*sysctl_hdr;

    struct prot_inuse __percpu *inuse;
};

#endif
