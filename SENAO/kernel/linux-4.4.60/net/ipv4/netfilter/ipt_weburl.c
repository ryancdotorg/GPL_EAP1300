/*  weburl --	A netfilter module to match URLs in HTTP requests
 *		This module can match using string match or regular expressions
 *		Originally designed for use with Gargoyle router firmware (gargoyle-router.com)
 *
 *
 *  Copyright © 2008-2010 by Eric Bishop <eric@gargoyle-router.com>
 *
 *  This file is free software: you may copy, redistribute and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This file is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <net/sock.h>
#include <net/ip.h>
#include <net/tcp.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_weburl.h>

#include "weburl_deps/regexp.c"
#include "weburl_deps/tree_map.h"


#include <linux/ip.h>


#include <linux/netfilter/x_tables.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eric Bishop");
MODULE_DESCRIPTION("Match URL in HTTP requests, designed for use with Gargoyle web interface (www.gargoyle-router.com)");

string_map* compiled_map = NULL;

#if 0
int strnicmp(const char * cs,const char * ct,size_t count)
{
	register signed char __res = 0;

	while (count)
	{
		if ((__res = toupper( *cs ) - toupper( *ct++ ) ) != 0 || !*cs++)
		{
			break;
		}
		count--;
	}
	return __res;
}
#endif

char *strnistr(const char *s, const char *find, size_t slen)
{
	char c, sc;
	size_t len;


	if ((c = *find++) != '\0')
	{
		len = strlen(find);
		do
		{
			do
			{
				if (slen < 1 || (sc = *s) == '\0')
				{
					return (NULL);
				}
				--slen;
				++s;
			}
			while ( toupper(sc) != toupper(c));

			if (len > slen)
			{
				return (NULL);
			}
		}
		while (strncasecmp(s, find, len) != 0);

		s--;
	}
	return ((char *)s);
}


int do_match_test(unsigned char match_type,  const char* reference, char* query)
{
	int matches = 0;
	struct regexp* r;
	switch(match_type)
	{
		case WEBURL_CONTAINS_TYPE:
			matches = (strstr(query, reference) != NULL);
			break;
		case WEBURL_REGEX_TYPE:

			if(compiled_map == NULL)
			{
				compiled_map = initialize_map(0);
				if(compiled_map == NULL) /* test for malloc failure */
				{
					return 0;
				}
			}
			r = (struct regexp*)get_map_element(compiled_map, reference);
			if(r == NULL)
			{
				int rlen = strlen(reference);
				r= regcomp((char*)reference, &rlen);
				if(r == NULL) /* test for malloc failure */
				{
					return 0;
				}
				set_map_element(compiled_map, reference, (void*)r);
			}
			matches = regexec(r, query);
			break;
		case WEBURL_EXACT_TYPE:
			matches = (strstr(query, reference) != NULL) && strlen(query) == strlen(reference);
			break;
	}
	return matches;
}

int http_match(const struct ipt_weburl_info* info, const unsigned char* packet_data, int packet_length)
{
	int test = 0;

	/* first test if we're dealing with a web page request */
	if(strncasecmp((char*)packet_data, "GET ", 4) == 0 || strncasecmp(  (char*)packet_data, "POST ", 5) == 0 || strncasecmp((char*)packet_data, "HEAD ", 5) == 0)
	{
		/* printk("found a  web page request\n"); */
		char path[625] = "";
		char host[625] = "";
		int path_start_index;
		int path_end_index;
		int last_header_index;
		char last_two_buf[2];
		int end_found;
		char* host_match;
		char* test_prefixes[6];
		int prefix_index;

		/* get path portion of URL */
		path_start_index = (int)(strstr((char*)packet_data, " ") - (char*)packet_data);
		while( packet_data[path_start_index] == ' ')
		{
			path_start_index++;
		}
		path_end_index= (int)(strstr( (char*)(packet_data+path_start_index), " ") -  (char*)packet_data);
		if(path_end_index > 0)
		{
			int path_length = path_end_index-path_start_index;
			path_length = path_length < 625 ? path_length : 624; /* prevent overflow */
			memcpy(path, packet_data+path_start_index, path_length);
			path[ path_length] = '\0';
		}

		/* get header length */
		last_header_index = 2;
		memcpy(last_two_buf,(char*)packet_data, 2);
		end_found = 0;
		while(end_found == 0 && last_header_index < packet_length)
		{
			char next = (char)packet_data[last_header_index];
			if(next == '\n')
			{
				end_found = last_two_buf[1] == '\n' || (last_two_buf[0] == '\n' && last_two_buf[1] == '\r') ? 1 : 0;
			}
			if(end_found == 0)
			{
				last_two_buf[0] = last_two_buf[1];
				last_two_buf[1] = next;
				last_header_index++;
			}
		}

		/* get host portion of URL */
		host_match = strnistr( (char*)packet_data, "Host:", last_header_index);
		if(host_match != NULL)
		{
			int host_end_index;
			host_match = host_match + 5; /* character after "Host:" */
			while(host_match[0] == ' ')
			{
				host_match = host_match+1;
			}

			host_end_index = 0;
			while(	host_match[host_end_index] != '\n' &&
					host_match[host_end_index] != '\r' &&
					host_match[host_end_index] != ' ' &&
					host_match[host_end_index] != ':' &&
					((char*)host_match - (char*)packet_data)+host_end_index < last_header_index
				 )
			{
				host_end_index++;
			}
			memcpy(host, host_match, host_end_index);
			host_end_index = host_end_index < 625 ? host_end_index : 624; /* prevent overflow */
			host[host_end_index] = '\0';


		}

		/* printk("host = \"%s\", path =\"%s\"\n", host, path); */


		switch(info->match_part)
		{
			case WEBURL_DOMAIN_PART:
				test = do_match_test(info->match_type, info->test_str, host);
				if(!test && strstr(host, "www.") == host)
				{
					test = do_match_test(info->match_type, info->test_str, ((char*)host+4) );
				}
				break;
			case WEBURL_PATH_PART:
				test = do_match_test(info->match_type, info->test_str, path);
				if( !test && path[0] == '/' )
				{
					test = do_match_test(info->match_type, info->test_str, ((char*)path+1) );
				}
				break;
			case WEBURL_ALL_PART:

				test_prefixes[0] = "http://";
				test_prefixes[1] = "";
				test_prefixes[2] = NULL;


				for(prefix_index=0; test_prefixes[prefix_index] != NULL && test == 0; prefix_index++)
				{
					char test_url[1250];
					test_url[0] = '\0';
					strcat(test_url, test_prefixes[prefix_index]);
					strcat(test_url, host);
					if(strcmp(path, "/") != 0)
					{
						strcat(test_url, path);
					}
					test = do_match_test(info->match_type, info->test_str, test_url);
					if(!test && strcmp(path, "/") == 0)
					{
						strcat(test_url, path);
						test = do_match_test(info->match_type, info->test_str, test_url);
					}

					/* printk("test_url = \"%s\", test=%d\n", test_url, test); */
				}
				if(!test && strstr(host, "www.") == host)
				{
					char* www_host = ((char*)host+4);
					for(prefix_index=0; test_prefixes[prefix_index] != NULL && test == 0; prefix_index++)
					{
						char test_url[1250];
						test_url[0] = '\0';
						strcat(test_url, test_prefixes[prefix_index]);
						strcat(test_url, www_host);
						if(strcmp(path, "/") != 0)
						{
							strcat(test_url, path);
						}
						test = do_match_test(info->match_type, info->test_str, test_url);
						if(!test && strcmp(path, "/") == 0)
						{
							strcat(test_url, path);
							test = do_match_test(info->match_type, info->test_str, test_url);
						}

						/* printk("test_url = \"%s\", test=%d\n", test_url, test); */
					}
				}
				break;

		}


		/*
		 * If invert flag is set, return true if this IS a web request, but it didn't match
		 * Always return false for non-web requests
		 */
		test = info->invert ? !test : test;
	}

	return test;
}


static bool match(const struct sk_buff *skb, struct xt_action_param *par)
{

	const struct ipt_weburl_info *info = (const struct ipt_weburl_info*)(par->matchinfo);


	int test = 0;
	struct iphdr* iph;

	/* linearize skb if necessary */
	struct sk_buff *linear_skb;
	int skb_copied;
	if(skb_is_nonlinear(skb))
	{
		linear_skb = skb_copy(skb, GFP_ATOMIC);
		skb_copied = 1;
	}
	else
	{
		linear_skb = (struct sk_buff*)skb;
		skb_copied = 0;
	}



	/* ignore packets that are not TCP */
	iph = (struct iphdr*)(skb_network_header(skb));
	if(iph->protocol == IPPROTO_TCP)
	{
		/* get payload */
		struct tcphdr* tcp_hdr		= (struct tcphdr*)( ((unsigned char*)iph) + (iph->ihl*4) );
		unsigned short payload_offset	= (tcp_hdr->doff*4) + (iph->ihl*4);
		unsigned char* payload		= ((unsigned char*)iph) + payload_offset;
		unsigned short payload_length	= ntohs(iph->tot_len) - payload_offset;



		/* if payload length <= 10 bytes don't bother doing a check, otherwise check for match */
		if(payload_length > 10)
		{
			test = http_match(info, payload, payload_length);
		}
	}

	/* free skb if we made a copy to linearize it */
	if(skb_copied == 1)
	{
		kfree_skb(linear_skb);
	}


	/* printk("returning %d from weburl\n\n\n", test); */
	return test;
}


static int checkentry(const struct xt_mtchk_param *par)
{
	return 0;
}


static struct xt_match weburl_match  __read_mostly  =
{
	.name		= "weburl",
	.match		= &match,
	.family		= AF_INET,
	.matchsize	= sizeof(struct ipt_weburl_info),
	.checkentry	= &checkentry,
	.me		= THIS_MODULE,
};

static int __init init(void)
{
	compiled_map = NULL;
	return xt_register_match(&weburl_match);

}

static void __exit fini(void)
{
	xt_unregister_match(&weburl_match);

	if(compiled_map != NULL)
	{
		unsigned long num_destroyed;
		destroy_map(compiled_map, DESTROY_MODE_FREE_VALUES, &num_destroyed);
	}
}

module_init(init);
module_exit(fini);

