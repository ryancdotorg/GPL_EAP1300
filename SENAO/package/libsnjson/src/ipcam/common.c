#include "common.h"

int sys_interact_long(char *output, int outputlen, char *fmt, ...)
{
	char command[MAX_JSON_REPLY_LEN];
	int i, c;
	FILE *pipe;
	va_list ap;

	memset(command, 0, sizeof(command));

	va_start(ap, fmt);
	vsnprintf(command, sizeof(command), fmt, ap);
	va_end(ap);

	memset(output, 0, outputlen);

	if((pipe = popen(command, "r")) == NULL)
	{
		goto err;
	}

	for(i = 0; ((c = fgetc(pipe)) != EOF) && (i < outputlen - 1); i++)
	{
		output[i] = (char) c;
	}
	output[i] = '\0';

	pclose(pipe);

	if(strlen(output) == 0)
	{
		goto err;
	}

	return 0;

err:
	strcpy(output, "---");
	return -1;
}

void addBackslashAndNewline(char *src, char *dst)
{
	while(*src != '\0')
	{
		if(*src == '\"')
		{
			*dst = '\\';
			dst++;
		}

		*dst = *src;
		src++;
		dst++;
	}

	*dst = '\n';
	dst++;
	*dst = '\0';
}

// copied from json_wireless.c
int json_set_login(ResponseEntry *rep, struct json_object *jobj, char *token)
{
	ResponseStatus *res = rep->res;
	json_object_object_add(jobj, "token", json_object_new_string(token));
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
