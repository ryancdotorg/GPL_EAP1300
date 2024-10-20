/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                     *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  linkmon                                                                       *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include "liblmipc.h"

typedef enum CMD_TYPE {
	GET = 0,
	SET
}CMD_TYPE;

void show_usage()
{
	printf("lmipc usage:\n" \
" -g <objpath>  show values of object, format: a/b[1]/c/d, use [] to get array value\n" \
" -h         show this help\n" \
"Example: lmipc -g Response[0]/Data/DeviceStatus/Connectivity[0]/status\n");
}

int main(int argc, char *argv[])
{
	int c;
	CMD_TYPE cmd;
	char objpath[256];
	while ((c = getopt(argc, argv,"g:h")) != -1){
		if (c<0){
			printf("input");
			break;
		}
		switch(c) {
			case 'h':
				show_usage();
				break;
			case 'g':
				strcpy(objpath, optarg);
				break;
			default:
				exit(EXIT_FAILURE);
		}		
	}
	if (optind ==1 && argc ==1){
		show_usage();
		return 0;
	}

	lm_handle_t *lh = lm_open();
	lm_handle_t *ptr;
	
	lm_show_obj(lh, objpath);

//	json_object *jo = lm_getjobj(lh, objpath);
//	int a = lm_getint(lh, "Response[0]/Data/DeviceStatus/Connectivity[0]/status");
//	printf("3 get - - <%s>\n", json_object_to_json_string(lh->subobj));
//	printf("3 get - - <%s>\n", json_object_to_json_string(jo));
//	printf("status: %d\n", a);

	lm_close(lh);
}
