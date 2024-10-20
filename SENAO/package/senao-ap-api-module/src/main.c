#include <fcgi_stdio.h>
#include <stdlib.h>
#include <api_tokens.h>
#include <api.h>
#include <sn_api.h>
#include <ext_api.h>
#include <time.h>
#include <jwt_api.h>

// color define
#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"
// color define


#define error_printf(x, ...) do { \
    FILE *fp = fopen("/dev/console", "a"); \
    if (fp) { \
        fprintf(fp, LIGHT_RED"[fcgi][%d] %s:%d "x"\n"NONE, getpid(),__func__, __LINE__, ##__VA_ARGS__ ); \
        fclose(fp); \
    } \
} while(0)


//#include <variable/variable.h>
/* EnGenius Open API URI
==wifi==
POST
/wifi/radio/24g
/wifi/radio/5g
/wifi/radio/triband/5g/{id}
/wifi/mesh
/wifi/ssid/{id}
/wifi/ssid/{id}/security
/wifi/ssid/{id}/captive_portal
/wifi/ssid/{id}/accounting_server
/wifi/ssid/{id}/traffic_shaping
/wifi/ssid/{id}/band_steering
/wifi/ssid/{id}/radius_server
/wifi/ssid/{id}/scheduling
GET
/wifi/radio/24g
/wifi/radio/5g
/wifi/radio/triband/5g/{id}
/wifi/mesh
/wifi/mesh/mesh_info
/wifi/ssid/{id}
/wifi/ssid/{id}/security
/wifi/ssid/{id}/captive_portal
/wifi/ssid/{id}/accounting_server
/wifi/ssid/{id}/traffic_shaping
/wifi/ssid/{id}/band_steering
/wifi/ssid/{id}/radius_server
/wifi/ssid/{id}/scheduling
/wifi/wifi_client_info
==net==
POST
/net/ethernet
GET
/net/ethernet

==sys==
GET
/sys/sys_info
POST
/sys/system_config 
*/ 
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
#define MAX_VARS 33
#define T_NUM_OF_ELEMENTS(x)		(sizeof(x)/sizeof(x[0]))
#define NEXTTABLE     ""
#define safeFree(p) saferFree((void**)&(p));
#define JWT_TOKEN_LENGTH 1024
#define GET_EXPR 0
#define GET_ROLE 1
#define GET_RAND 2
#define ROLE_GUEST 1
#define ROLE_ADMIN 2

extern int basicTableLen;
extern ApiEntry basicTable[]; 
extern int extTableLen;
extern ApiEntry extensionTable[]; 
extern char queryTable[];
int now_role=ROLE_GUEST; //1:GUEST , 2:ADMIN
/*-----------------------------------------------------------------------*/
/*                        GLOBAL VARIABLES                               */
/*-----------------------------------------------------------------------*/
/*
ex:https://192.168.1.1:4430/api/wifi/radio/24g?aaa=1,bbb=2
QUERY_STRING: aaa=1,bbb=2
REQUEST_METHOD: GET
REQUEST_URI: /api/wifi/radio/24g?aaa=1,bbb=2
*/ 

char* vars[MAX_VARS] = {
    "CONTENT_LENGTH",
    "DOCUMENT_ROOT",
    "GATEWAY_INTERFACE",
    "HTTP_ACCEPT",
    "HTTP_ACCEPT_ENCODING",
    "HTTP_ACCEPT_LANGUAGE",
    "HTTP_CACHE_CONTROL",
    "HTTP_CONNECTION",
    "HTTP_HOST",
    "HTTP_PRAGMA",
    "HTTP_RANGE",
    "HTTP_REFERER",
    "HTTP_TE",
    "HTTP_USER_AGENT",
    "HTTP_X_FORWARDED_FOR",
    "PATH",
    "QUERY_STRING",
    "REMOTE_ADDR",
    "REMOTE_HOST",
    "REMOTE_PORT",
    "REQUEST_METHOD",
    "REQUEST_URI",
    "SCRIPT_FILENAME",
    "SCRIPT_NAME",
    "SERVER_ADDR",
    "SERVER_ADMIN",
    "SERVER_NAME",
    "SERVER_PORT",
    "SERVER_PROTOCOL",
    "SERVER_SIGNATURE",
    "SERVER_SOFTWARE",
    "JWT_TOKEN",
    "X_IM_APP"
};

void openapi_response(ResponseEntry *rep, FCGX_Request *request)
{
    char* repResult = NULL;
    FCGX_FPrintF(request->out,"Status: %d \r\n", rep->res->statusCode);
    FCGX_FPrintF(request->out,"Access-Control-Allow-Origin: *\r\n");
    FCGX_FPrintF(request->out,"Access-Control-Allow-Methods: GET, POST, DELETE, PUT\r\n");
    FCGX_FPrintF(request->out,"Access-Control-Allow-Headers: Content-Type, api_key, Authorization\r\n");
    FCGX_FPrintF(request->out,"X-Rate-Limit: 5000\r\n");
    FCGX_FPrintF(request->out,"Content-type: application/json\r\n\r\n");

    repResult = printResponseResult(rep);

    if ( repResult != NULL )
    {
        FCGX_FPrintF(request->out, "%s\n", repResult);
        //free(repResult); // no need to free this, json_put will free this.
    }
    else
    {
        FCGX_FPrintF(request->out, "\n");
    }
}

int point_to_array(char *p, char *query_string, int query_string_limit_size)
{
    if (strlen(p) < query_string_limit_size)
    {
        strcpy(query_string, p);
        return 0;
    }
    else
    {
        debug_print("[Senao API] Error! point string size > array size !\n");
        return -1;  //error handling.. todo..
    }
}

bool check_jwt_rand(char* input)
{
    FILE *fp = NULL;
    char jwt_rand[32] = {0};

    if ( input == NULL )
    {
        return false;
    }

    fp = fopen(SENAO_JWT_RANDOM_TOKEN_FILE, "r");

    if ( fp )
    {
        fread(jwt_rand, sizeof(jwt_rand)-1, 1, fp);
        fclose(fp);

        if ( strcmp(input, jwt_rand) == 0 )
        {
            return true;
        }
    }

    return false;
}

unsigned int getVmSize()
{
    FILE *fp = NULL;
    char line[2048] = {0};
    unsigned int vmsize = 0;

    fp = fopen("/proc/self/status", "r");
    if (fp == NULL)
    {
        return 0;
    }

    while(fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "VmSize:", 7) == 0)
        {
            sscanf(line, "%*s %u", &vmsize);
            break;
        }
    }

    fclose(fp);

    return vmsize;
}

#define LISTENSOCK_FILENO 0
#define LISTENSOCK_FLAGS 0

int main(void)
{
    unsigned int cgi_counter = 0;
    unsigned int memory_usage = 0 ;

    int err = FCGX_Init();
    if (err) { error_printf ("FCGX_Init failed: %d", err); return 1; }

    FCGX_Request request;
    err = FCGX_InitRequest(&request, LISTENSOCK_FILENO, LISTENSOCK_FLAGS);
    if (err) { error_printf ("FCGX_InitRequest failed: %d", err); return 2; }

    /*
    ex:https://192.168.1.1:4430/api/wifi/radio/24g?aaa=1,bbb=2
    QUERY_STRING: aaa=1,bbb=2
    REQUEST_METHOD: GET
    REQUEST_URI: /api/wifi/radio/24g?aaa=1,bbb=2
    */ 

    while(1)
    {
        cgi_counter++;
        err = FCGX_Accept_r(&request);
        if(err)
        {
            error_printf ("FCGX_Accept_r stopped: %d", err);
            break;
        }

        char *pbody = NULL;
        ResponseEntry *rep = Response_create();

        // print all env value
        char **env = request.envp;
        while (*(++env))
        {
            debug_print("request.envp[%s]\n", *env);
        }

        int count = 0;
        int t=0, i=0, j=0, match=0, table_length=0, init_idx = 0;
        int return_val=0;
        int content_length=0;
        char *p=NULL;
        char query_string[256]={0}, request_method[16]={0}, request_uri[512]={0}, request_remoteAddr[64]={0}, request_im_app[16]={0};
        char api_uri[256]={0};
        char api_uri_array[API_PARSING_LEVEL][API_LEVEL_LENGTH] = {{0}};
        ApiEntry *p_basicTable = NULL;
        HTTPEntry packet;
        memset(&packet, 0, sizeof(packet));

        if (FCGX_GetParam("CONTENT_LENGTH", request.envp))
        {  
            content_length = atoi(FCGX_GetParam("CONTENT_LENGTH", request.envp));
        }
        if (content_length>0)
        {
            pbody = (char *) malloc(content_length+1);
            if (pbody == NULL)
            {
                goto end;
            }
            memset(pbody, 0, sizeof(pbody));
            FCGX_GetLine(pbody, content_length+1, request.in);
            debug_print("\ndata[%s]  content_length[%d]\n", pbody, content_length); 
        }

        p = FCGX_GetParam("QUERY_STRING", request.envp);
        if (p)
        {
            return_val = point_to_array(p, query_string, sizeof(query_string));
        }

        p = FCGX_GetParam("REQUEST_METHOD", request.envp);
        if (p)
        {
            return_val = point_to_array(p, request_method, sizeof(request_method));
        }

        p = FCGX_GetParam("REQUEST_URI", request.envp);
        if (p)
        {
            return_val = point_to_array(p, request_uri, sizeof(request_uri));
        }

        p = FCGX_GetParam("REMOTE_ADDR", request.envp);
        if (p)
        {
            return_val = point_to_array(p, request_remoteAddr, sizeof(request_remoteAddr));
        }

        p = FCGX_GetParam("X_IM_APP", request.envp);
        if (p)
        {
            return_val = point_to_array(p, request_im_app, sizeof(request_im_app));
        }

        debug_print("\nJason DEBUG %s[%d], query_string [%s]\n", __FUNCTION__, __LINE__, query_string); 
        debug_print("Jason DEBUG %s[%d], request_method [%s]\n", __FUNCTION__, __LINE__, request_method); 
        debug_print("Jason DEBUG %s[%d], request_uri [%s]\n", __FUNCTION__, __LINE__, request_uri); 
        debug_print("Jason DEBUG %s[%d], request_remoteAddr [%s]\n", __FUNCTION__, __LINE__, request_remoteAddr); 
        debug_print("Jason DEBUG %s[%d], request_im_app [%s]\n", __FUNCTION__, __LINE__, request_im_app);

        //ex: /api/wifi/radio/24g?aaa=1,bbb=2  get /wifi/radio/24g
        sscanf(request_uri, "/api%[^?]", api_uri);
        debug_print("Jason DEBUG %s[%d], api_uri [%s]\n", __FUNCTION__, __LINE__, api_uri);

        memset(api_uri_array, 0, sizeof(api_uri_array)); 
        //EnGenius API only support 7 Level now
        sscanf(api_uri, "/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]", api_uri_array[0], api_uri_array[1], api_uri_array[2], api_uri_array[3], api_uri_array[4], api_uri_array[5], api_uri_array[6]);

/*-----------------------------------------------------------------------*/
/*                        JWT Check                                      */
/*-----------------------------------------------------------------------*/

        char Token_Header[JWT_TOKEN_LENGTH] = {0};
        char *expr = NULL;
        char *jwt_rand = NULL;
        char *role={0};
        time_t now = time(NULL);
        int x = now;
        int Token_time = 0;
        debug_print("%s:%d now[%d] ###\n", __FUNCTION__, __LINE__, now);

        if (strcmp(api_uri_array[1],"login") != 0 && strcmp(api_uri_array[2],"client_info") != 0 && strcmp(api_uri_array[1],"reset_with_key") != 0
#if SUPPORT_LOCAL_ADDR_WITHOUT_CERTIFICATE
                && strcmp(request_remoteAddr,"127.0.0.1") != 0 
#endif
                )
        {
            p = FCGX_GetParam("JWT_TOKEN", request.envp);
            debug_print("%s:%d p[%s] ###\n", __FUNCTION__, __LINE__, p);
            if (p)
            {
                return_val = point_to_array(p, Token_Header, sizeof(Token_Header));
                debug_print("%s:%d Token_Header[%s] ###\n", __FUNCTION__, __LINE__, Token_Header);
            }
            else
            {
                genErrorMessage(rep->res, API_INVALID_TOKEN, "EMPTY JWT TOKEN");
                goto end;
            }

            decode_hs256(Token_Header, GET_RAND, &jwt_rand);
            debug_print("%s:%d jwt_rand[%s] ###\n", __FUNCTION__, __LINE__, jwt_rand);

            decode_hs256(Token_Header, GET_EXPR, &expr);
            debug_print("%s:%d expr[%s] ###\n", __FUNCTION__, __LINE__, expr);

            if ( jwt_rand && check_jwt_rand(jwt_rand) )
            {
                if (expr)
                {
                    Token_time = atoi(expr);

                    debug_print("%s:%d Token_time[%d] ###\n", __FUNCTION__, __LINE__, Token_time);

                    if (Token_time >= now)
                    {
                        //Get role
                        decode_hs256(Token_Header, GET_ROLE, &role);

                        if (strcmp(role, "guest")==OK)
                        {
                            now_role=ROLE_GUEST; //1
                        }
                        else
                        {
                            now_role=ROLE_ADMIN; //2
                        }
                    }
                    else
                    {
                        genErrorMessage(rep->res, API_INVALID_TOKEN, "Expired");
                        goto end;
                    }
                }
                else
                {
                    genErrorMessage(rep->res, API_INVALID_TOKEN, "Error");
                    goto end;
                }
            }
            else
            {
                genErrorMessage(rep->res, API_INVALID_TOKEN, "Error");
                goto end;
            }
        }

# if SUPPORT_LOCAL_ADDR_WITHOUT_CERTIFICATE
        if ( strcmp(request_remoteAddr,"127.0.0.1") == 0 )
            now_role=ROLE_ADMIN; //2
#endif

#if SUPPORT_NETGEAR_FUNCTION
        if (strcmp(request_im_app, "true") == 0 && strcmp(api_uri_array[1], "radio") == 0 && strcmp(api_uri_array[2], "5g") == 0 
            && ( strcmp(request_method, "PATCH") == 0 || strcmp(request_method, "POST") == 0))
        {
            system("rm /tmp/insight_im_app");
            system("touch /tmp/insight_im_app");
        }
#endif

/*-----------------------------------------------------------------------*/
/*                        JWT Check End                                  */
/*-----------------------------------------------------------------------*/

        memset(queryTable, 0, 256);
        strcpy(queryTable, query_string);

        packet.query_string = query_string;
        packet.request_method = request_method;
        packet.api_uri_array = api_uri_array;
        packet.body = pbody;

        for (t=0; t<2; t++)
        {
            p_basicTable = (t==0) ? &(basicTable[0]) : &(extensionTable[0]);
            table_length = (t==0) ? basicTableLen : extTableLen;

            for (i=0; i<table_length; i++)
            {
                for (j=0; j<API_PARSING_LEVEL; j++)
                {
                    if ((strcmp(api_uri_array[j], p_basicTable[i].uri[j])==OK) || (strcmp(VARIABLE, p_basicTable[i].uri[j])==OK) || (strcmp(VARIABLE_OPMODE, p_basicTable[i].uri[j])==0 && strstr(VARIABLE_OPMODE,api_uri_array[j])))
                    {
                        if ((j==(API_PARSING_LEVEL-1)) && (strcmp(request_method, p_basicTable[i].method) == 0))
                        {
                            if ((p_basicTable[i].group & now_role)==now_role)
                            {
                                p_basicTable[i].cb_function(packet, rep);
                                match=1;
                                goto end;
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        if (match==0)
        {
            debug_print("[Senao API] Error! Can't Find match API!\n");
            genErrorMessage(rep->res, API_UNKNOWN_ACTION, "not match");
            goto end;
            //return error packet...todo..
        }

        /*
    //libapi function ex:
    char api_get_buf[64]={0};
    api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION, api_get_buf, sizeof api_get_buf);
    printf("api get lan ip: [%s]\n", api_get_buf);
        */

      
      //User Login:
      //printf("%s","logged in user session:1510798305917");
      
      
      /*
    //Pet PUT:      
    printf("%s\n",  "{\"id\":8977020506658297764,\"category\":{\"id\":0,\"name\":\"string\"}, \
    \"name\":\"doggie\",\"photoUrls\":[\"string\"],\"tags\":[{\"id\":0,\"name\":\"string\"}], \
    \"status\":\"available\"}");      
      */
end:
        debug_print("Jason DEBUG %s[%d], query_string [%s]\n", __FUNCTION__, __LINE__, "end");
        openapi_response(rep, &request);
        Response_destroy(rep);

        free(pbody);
        pbody = NULL;
        free(expr);
        expr = NULL;
        free(role);
        role = NULL;
        free(jwt_rand);
        jwt_rand = NULL;

        FCGX_Finish_r(&request);

        if(cgi_counter > 50)
        {
            memory_usage = getVmSize();
            //check memory > 15mb
            if(memory_usage > 15000)
            {
                error_printf("memory_usage = %u, Self-Healing, fcgi recall.\n", memory_usage);
                break;
            }
            cgi_counter = 0;
        }
    }
    FCGX_ShutdownPending();
    return 0;
}
