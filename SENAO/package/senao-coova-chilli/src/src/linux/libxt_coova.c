/* -*- mode: c; c-basic-offset: 8 -*- */
/* Shared library add-on to iptables to add coova support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <linux/netfilter/x_tables.h>
#include <xtables.h>
#include "xt_coova.h"

static const struct option coova_opts[] = {
	{ .name = "name",    .has_arg = 1, .flag = 0, .val = 201 },
	{ .name = "sorc",    .has_arg = 0, .flag = 0, .val = 202 },
	{ .name = "dest",    .has_arg = 0, .flag = 0, .val = 203 },
	{ .name = "mdmac",   .has_arg = 1, .flag = 0, .val = 204 },
	{ .name = 0, 	     .has_arg = 0, .flag = 0, .val = 0  }
};

static void coova_help(void)
{
	printf(
"coova match options:\n"
"    --name name                 Name of the table to be used. 'chilli' used if none given.\n"
"    --sorc                      Indicates the source direction (lookup by source MAC/IP)\n"
"    --dest                      Indicates the reply (lookup by dest address).\n"
"    --mdmac de:ma:ca:dd:re:ss   mangle dest mac address\n"
"xt_coova by: David Bird (Coova Technologies) <support@coova.com>.  http://www.coova.org/CoovaChilli\n");
}

static void coova_init(struct xt_entry_match *match)
{
	uint8_t nmac[XT_COOVA_ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	struct xt_coova_mtinfo *info = (void *)(match)->data;
	strncpy(info->name,"chilli", XT_COOVA_NAME_LEN);
	info->name[XT_COOVA_NAME_LEN-1] = '\0';
	info->side = XT_COOVA_SOURCE;
	memcpy(info->mdmac, nmac, XT_COOVA_ETH_ALEN);
}

static int coova_parse(int c, char **argv, int invert, unsigned int *flags,
                        const void *entry, 
			struct xt_entry_match **match)
{
	struct xt_coova_mtinfo *info = (void *)(*match)->data;
	unsigned int maci[XT_COOVA_ETH_ALEN];
	int i;

	switch (c) {
		case 201:
			strncpy(info->name,optarg, XT_COOVA_NAME_LEN);
			info->name[XT_COOVA_NAME_LEN-1] = '\0';
                        if (invert) info->invert = 1;
			break;

		case 202:
			info->side = XT_COOVA_SOURCE;
			break;

		case 203:
			info->side = XT_COOVA_DEST;
			break;

		case 204:
			if(XT_COOVA_ETH_ALEN == sscanf(optarg, "%2x:%2x:%2x:%2x:%2x:%2x",
						&maci[0], &maci[1], &maci[2],
						&maci[3], &maci[4], &maci[5]))
			{
				for(i=0; i<XT_COOVA_ETH_ALEN; i++)
				{
					info->mdmac[i] = maci[i]&0xFF;
				}
			}
			break;

		default:
			return 0;
	}

	return 1;
}

static void coova_check(unsigned int flags)
{
}

static void coova_print(const void *ip, const struct xt_entry_match *match,
                         int numeric)
{
	const struct xt_coova_mtinfo *info = (const void *)match->data;
	if (info->invert)
		fputc('!', stdout);
	printf("coova: ");
	if(info->name) 
		printf(" name: %s",info->name);
	if (info->side == XT_COOVA_SOURCE)
		printf(" side: source");
	if (info->side == XT_COOVA_DEST)
		printf(" side: dest");
	if(info->mdmac)
	{
		printf(" mdmac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
				info->mdmac[0], info->mdmac[1], info->mdmac[2],
				info->mdmac[3], info->mdmac[4], info->mdmac[5]);
	}
}

static void coova_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_coova_mtinfo *info = (const void *)match->data;
	if (info->invert) 
		printf(" !");
	if(info->name) 
		printf(" --name %s",info->name);
	if (info->side == XT_COOVA_SOURCE)
		printf(" --sorc");
	if (info->side == XT_COOVA_DEST)
		printf(" --dest");
	if(info->mdmac)
	{
		printf(" --mdmac %.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
				info->mdmac[0], info->mdmac[1], info->mdmac[2],
				info->mdmac[3], info->mdmac[4], info->mdmac[5]);
	}
}

static struct xtables_match coova_mt_reg = {
	.name          = "coova",
	.version       = XTABLES_VERSION,
	.family        = NFPROTO_IPV4,
	.size          = XT_ALIGN(sizeof(struct xt_coova_mtinfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_coova_mtinfo)),
	.help          = coova_help,
	.init          = coova_init,
	.parse         = coova_parse,
	.final_check   = coova_check,
	.print         = coova_print,
	.save          = coova_save,
	.extra_opts    = coova_opts,
};

static struct xtables_match coova_mt6_reg = {
	.name          = "coova",
	.version       = XTABLES_VERSION,
	.revision      = 0,
	.family        = NFPROTO_IPV6,
	.size          = XT_ALIGN(sizeof(struct xt_coova_mtinfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_coova_mtinfo)),
	.help          = coova_help,
	.init          = coova_init,
	.parse         = coova_parse,
	.final_check   = coova_check,
	.print         = coova_print,
	.save          = coova_save,
	.extra_opts    = coova_opts,
};

void _init(void)
{
	xtables_register_match(&coova_mt_reg);
	xtables_register_match(&coova_mt6_reg);
}
