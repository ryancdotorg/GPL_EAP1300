/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                    *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  phylink                                                                        *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <linux/module.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/of_device.h>
#include <linux/proc_fs.h>
#include <linux/phy.h>
#include <linux/mdio.h>
#include <linux/of_mdio.h>

#include "phylink.h"

struct ipq40xx_mdio_data {
        struct mii_bus          *mii_bus;
        void __iomem            *membase;
        int phy_irq[PHY_MAX_ADDR];
};

/*
 * Porting Guide: compatible string
 *
 * */
typedef enum drvname {
    COMPAT_IPQ40XX,
    COMPAT_AG71XX,
} drvname;

typedef struct compatible_strings_t {
    drvname name;
    char* string;
} compatible_strings_t;

static compatible_strings_t compatible_strings[] =
{
    { COMPAT_IPQ40XX, "qcom,ipq40xx-mdio"},
    { COMPAT_AG71XX, "qca,ag71xx-mdio"},
};


/*
 * Porting Guide: mdio read link status register address
 *
 * */
#define MDIO_LINK_READ_REG MII_BMSR //0x1
#define MDIO_LINK_READ_REG_C45 MII_ADDR_C45 | (MDIO_MMD_AN << 16) | (MDIO_STAT1 & 0xffff)

int get_lan_state(unsigned int *link_state)
{
    struct device_node *mdio_node = NULL;
    struct platform_device *mdio_plat = NULL;

    struct ipq40xx_mdio_data *mdio_data = NULL;
    struct phy_device *phydev;
    int reg;
    struct device_node *np, *child;
    u32 read_mask = 0;
    u32 c45_mask = 0;
    int addr;
    u32 mdio_link_read_reg;

    struct mii_bus *miibus = NULL;
    unsigned int lan_state=0;

    int i;
    for (i=0; i < ARRAY_SIZE(compatible_strings); i++){
        mdio_node = of_find_compatible_node(NULL, NULL, compatible_strings[i].string);
        if (mdio_node) break;
    }

    if (mdio_node == NULL){
        return -1;
    }
    mdio_plat = of_find_device_by_node(mdio_node);
    if (mdio_plat == NULL){
	of_node_put(mdio_node);
	*link_state=0;
        return -1;
    }
    mdio_data = dev_get_drvdata(&mdio_plat->dev);
    if (mdio_data == NULL){
	of_node_put(mdio_node);
	*link_state=0;
        return -1;
    }
    miibus = mdio_data->mii_bus;
    if (miibus == NULL){
	of_node_put(mdio_node);
	*link_state=0;
        return -1;
    }
    //printk("miibus name: %s id: %s maxport:%d\n", miibus->name, miibus->id, PHY_MAX_ADDR);
    np = miibus->dev.of_node;
    if (!np) {
	read_mask = ~miibus->phy_mask;
    } else {
        for_each_available_child_of_node(np, child){
            addr = of_mdio_parse_addr(&miibus->dev, child);
	    if (addr < 0)
               continue;
            read_mask |= 1 << addr;
	    if (of_device_is_compatible(child, "ethernet-phy-ieee802.3-c45") != 0){
                c45_mask |= 1 << addr;
	    }
	}
    }

    for (addr = 0; addr < PHY_MAX_ADDR; addr++){
        if (read_mask & 1<< addr){
            phydev = get_phy_device(miibus, addr, c45_mask & 1<< addr);
            if (IS_ERR(phydev)){
                if (phydev != NULL)
                    phy_device_free(phydev);
                continue;
	    }
            if (phydev == NULL)
               continue;

            mdio_link_read_reg = (phydev->is_c45) ? MDIO_LINK_READ_REG_C45 : MDIO_LINK_READ_REG;
            reg = mdiobus_read(miibus, addr, mdio_link_read_reg);
            if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS)){
                lan_state &= ~(1 << (addr));
                 //printk("%d link down\n", addr);
            }
            else {
                lan_state |= 1 << (addr);
                //printk("%d link up\n", addr);
            }
            phy_device_free(phydev);
        }
    }
    of_node_put(mdio_node);
    *link_state = lan_state;
    return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
int seq_read_proc(char *buf,char **start,off_t offset,int count,int *eof,void *data)
{
    int len=0;
    unsigned int lan_state;
    if (!get_lan_state(&lan_state))
        len = sprintf(buf,"%x\n", lan_state);
    else
        len = sprintf(buf,"\n");
    return len;
}
#else
int athprocinfo_show_ul(struct seq_file *m, void *v)
{
    unsigned int lan_state;

    if (!get_lan_state(&lan_state)){
        seq_printf(m, "%x\n", lan_state);
    } else {
        seq_printf(m, "\n");
    }
    return 0;
}

static int lan_state_open(struct inode *inode, struct file *file)
{
    return single_open(file, athprocinfo_show_ul, NULL);
}

static struct file_operations lan_state_fops = {
    .owner = THIS_MODULE,
    .open = lan_state_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};
#endif // LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)

static int __init phylink_init(void)
{
    struct proc_dir_entry *file;
    dbg_printk(KERN_INFO "init phylink module\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
    file = create_proc_entry("LAN_STATE",0644, NULL);
    if (!file){
        return -ENXIO;
    file->read_proc = seq_read_proc;
#else
    file = proc_create("LAN_STATE", 0644, NULL, &lan_state_fops);
    if (!file)
        return -ENXIO;
#endif
    return 0;
}

static void __exit phylink_exit(void)
{
    dbg_printk(KERN_INFO "exiting phylink module\n");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
    remove_proc_entry("LAN_STATE", NULL);
#else
    remove_proc_entry("LAN_STATE", &proc_root);
#endif

}


module_init(phylink_init);
module_exit(phylink_exit);

MODULE_DESCRIPTION("SenaoNetwork");
MODULE_AUTHOR("SenaoNetwork");
MODULE_LICENSE("Proprietary");

