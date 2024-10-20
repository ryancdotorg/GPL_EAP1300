#include <stdio.h>
#include <sys/wait.h>
#include "ostypes.h"
#include "appagentd.h"
#include "appagents.h"

#define APP_AGENT_VER	"v1.0.0 [2013/08/09]"

int do_encrypt_payload = 0;

int main(int argc, char *argv[]) 
{
    int c;

	printf("app_agent - %s\n", APP_AGENT_VER);

    for(;;)
    {
        c = getopt(argc, argv, "eskpd");
        if(c < 0)
            break;
        switch(c)
        {
#if HAS_ENCRYPT_PAYLOAD
            case 'e':
                do_encrypt_payload = 1;
                printf(" - encrypt JSON payload: enable\n");
                break;
#endif
            case 'd':
				do_dbg = 1;
                break;
            default:
                break;
        }
    }

#if APP_AGENTD_HAS_SSL
	http_ssl_params(argc, argv);
#endif

	if (do_dbg)
	{
		printf(" - debug: enable\n");
	}

	AdminCfg_Init(); //httputil_DIR

	AuthCfg_Init();

	HttpCfgData_Init();

	return 1;
}
