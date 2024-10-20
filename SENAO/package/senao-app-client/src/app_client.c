/****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;----------------------------------------------------------------------------
;
;    Project : app_client
;    Creator :
;    File    : app_client.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;
;****************************************************************************/

#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <resolv.h>
#include <getopt.h>
#include <aes.h>
#include <base64.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>

#define APP_CLIENT_DEBUG                       0

#define OK                                     1
#define ERROR                                  0

#define DEFAULT_PORT                           9090
#define MAX_LEN_8                              8
#define MAX_LEN_32                             32
#define MAX_LEN_64                             64
#define MAX_LEN_128                            128
#define MAX_BUF_1024                           1024
#define MAX_RECV_BUF                           1024*32
#define MAX_SOCKET_NUM                         32

#define HTTP_REQUEST_PACKET_FORMAT             "%s /json/%s HTTP/1.1\r\n" \
                                               "Host: %s:%d\r\n" \
                                               "User-Agent: SenaoAppClient/1.0\r\n" \
                                               "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" \
                                               "Accept-Encoding: gzip,deflate\r\n" \
                                               "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" \
                                               "Keep-Alive: 115\r\n" \
                                               "Connection: keep-alive\r\n" \
                                               "Content-Length: %d\r\n" \
                                               "Content-Type: text/plain; charset=UTF-8\r\n" \
                                               "Pragma: no-cache\r\n" \
                                               "Cache-Control: no-cache\r\n\r\n" \
                                               "%s"

#define HTTP_CONTENT_TYPE_RESPONSE_FORMAT      "Content-Type: text/html; charset=utf-8\r\n\r\n"

#if APP_CLIENT_DEBUG
#define DEBUG(msg...)                          printf(msg)
#else
#define DEBUG(...)
#endif

char *http_methods[] =
{
    "GET",
    "POST",
    NULL
};

void usage()
{
    printf("usage: app_client [-h?][OPTION]\n");
    printf("\t-m : HTTP method [GET/POST]\n");
    printf("\t-i : IP address of server\n");
    printf("\t-M : IP address of servers\n");
    printf("\t-P : Port of server\n");
    printf("\t-a : API name\n");
    printf("\t-p : Payload of this API. Note:No newline in the payload. Maximum is 1024 bytes.\n");
    printf("\t-e : Payload encryption [0:Disable/1:Enable]\n");
    printf("\t-d : Debug mode. Show packet content. [0:Disable/1:Enable]\n");

    return;
}

int encrypt_payload(char *payload, int *payload_len)
{
    size_t cipher_len, base64_len;
    char *cipher_text = NULL, *base64_text = NULL;

    cipher_len = 0, base64_len = 0;

    cipher_text = encrypt_data((const char *)payload, *payload_len, (int *)&cipher_len);

    if((NULL != cipher_text) && (cipher_len > 0))
    {
        base64_text = base64_encode((const unsigned char *)cipher_text, cipher_len, &base64_len);
    }

    if((NULL != base64_text) && (base64_len > 0))
    {
        if(base64_len >= MAX_BUF_1024)
        {
            printf("[app_client] : Base64 text overflow (%d)!\n", base64_len);
            return ERROR;
        }
        else
        {
            memcpy(payload, base64_text, base64_len);
            payload[base64_len] = '\0';
            *payload_len = base64_len;
        }
    }

    if(NULL != base64_text)
    {
        free(base64_text);
    }

    if(NULL != cipher_text)
    {
        free(cipher_text);
    }

    return OK;
}

int decrypt_payload(char *payload, int payload_len)
{
    char *plain_text = NULL, *cipher_text = NULL;
    char *body_begin = NULL, *body_end = NULL;
    size_t plain_len, cipher_len;

    if(NULL == (cipher_text = (char *)base64_decode((const char *)payload, (size_t)payload_len, &cipher_len)))
    {
        printf("[app_client] : de-base64 data failed!\n");
        return ERROR;
    }

    if(NULL == (plain_text = decrypt_data((const char *)cipher_text, (int)cipher_len, (int *)&plain_len)))
    {
        printf("[app_client] : decrypt data failed!\n");

        if(NULL != cipher_text)
        {
            free(cipher_text);
        }

        return ERROR;
    }
    else
    {
        if(plain_len > 0)
        {
            body_begin = strchr(plain_text, '{');
            body_end = strchr(plain_text, '}');
        }

        if(plain_len > 0 && (body_begin == NULL || body_end == NULL || body_begin > body_end))
        {
            printf("[app_client] : Illegal JSON format, clean up now...\n");
            sprintf(plain_text,"{}");
            plain_text[2] = '\0';
            plain_len = 2;
        }

        if((plain_len+1) > MAX_RECV_BUF)
        {
            printf("[app_client] : ERROR ! plain_len reach MAX_RECV_BUF [%d]...\n", MAX_RECV_BUF);
        }

        snprintf(payload, (plain_len+1) < MAX_RECV_BUF ? (plain_len+1) : MAX_RECV_BUF, "%s", plain_text);
        payload[plain_len] = '\0';
    }

    if(cipher_text != NULL)
    {
        free(cipher_text);
    }

    return OK;
}

int main(int argc, char *argv[])
{
    struct in6_addr serveraddr;
    struct addrinfo hints, *res = NULL;
    struct timeval timeout, select_timeout;
    int  rc, port, len=0;
    int  i=0, pos=0, debug=0, bytes=0, user_def_port=0;
    int  total_bytes=0, payload_len=0, encryption=0;
    int  result=0, send_count=0, receive_count=0, count=0;
    int  max_fd=0;
    int  sfd[MAX_SOCKET_NUM] = { -1 };
    int  ret = 1; //error, for shell use
    int  multiple_address = 0;
    char *ptr;
    char *api_ptr;
    char http_method[MAX_LEN_8];
    char api[MAX_LEN_64];
    char payload[MAX_BUF_1024];
    char buf[MAX_RECV_BUF];
    char recv_buf[MAX_RECV_BUF];
    char ip_addr[MAX_LEN_128];
    char port_str[MAX_LEN_8]={0};
    char scope_if[MAX_LEN_8]={0};
    char get_addr[256]={0};

    char ip[MAX_SOCKET_NUM][MAX_LEN_128];
    fd_set fds, fds_select;

    memset(ip_addr, 0x00, sizeof(ip_addr));
    memset(http_method, 0x00, sizeof(http_method));
    memset(api, 0x00, sizeof(api));
    memset(payload, 0x00, sizeof(payload));
    memset(buf, 0x00, sizeof(buf));
    memset(ip, 0x00, sizeof(ip));
    memset(&timeout, 0x00, sizeof(timeout));
    memset(&select_timeout, 0x00, sizeof(select_timeout));
    FD_ZERO(&fds);
    FD_ZERO(&fds_select);
    count = 0;
    api_ptr = NULL;

    port = DEFAULT_PORT;

    while(EOF != (pos = getopt(argc, argv, "a:d:e:i:I:m:M:p:P:h?")))
    {
        switch(pos)
        {
            case 'm':
                i = 0;
                while(NULL != http_methods[i])
                {
                    if(0 == strcmp(http_methods[i], optarg))
                    {
                        sprintf(http_method, optarg);
                        break;
                    }

                    i++;
                }

                if(NULL == http_methods[i])
                {
                    printf("[app_client] : Only support GET/POST HTTP Methods... [%s]\n", optarg);
                    usage();
                    goto out_main;
                }
                break;
            case 'M':
                multiple_address = 1;
            case 'i':
                sprintf(buf, "%s", optarg);
                i = 0;

                ptr = strtok(buf, ",");

                while (NULL != ptr)
                {
                    sprintf(ip[i], "%s", ptr);
                    i++;

                    ptr = strtok(NULL, ",");
                }

                break;
            case 'I':
                sprintf(scope_if, "%s", optarg);
                break;
            case 'P':
                port = atoi(optarg);
                user_def_port = 1;
                break;
            case 'a':
                sprintf(api, "%s", optarg);

                api_ptr = strchr(api, '/');
                if (NULL != api_ptr)
                {
                    /* The real API name starts after the character '/' */
                    api_ptr += 1;
                }
                else
                {
                    api_ptr = api;
                }
                break;
            case 'p':
                sprintf(payload, "%s", optarg);

                if(strchr(payload, '\n'))
                {
                    printf("[app_client] : There should not be any newlines in payload... [%s]\n", optarg);
                    goto out_main;
                }

                payload_len = strlen(payload);
                break;
            case 'e':
                encryption = atoi(optarg);

                if(!((0 == encryption) || (1 == encryption)))
                {
                    printf("[app_client] : Payload encryption should be 0(Disable) or 1(Enable)...\n");
                    goto out_main;
                }
                break;
            case 'd':
                debug = atoi(optarg);
                break;
            case 'h':
            case '?':
            default:
                usage();
                goto out_main;
                break;
        }
    }

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    if(multiple_address)
    {
        printf("[");
    }

    for (i = 0; (i < MAX_SOCKET_NUM) && (0 != strlen(ip[i])); i++)
    {
        memset(&hints, 0x00, sizeof(hints));
        hints.ai_flags      = AI_NUMERICSERV;
        hints.ai_family     = AF_UNSPEC;
        hints.ai_socktype   = SOCK_STREAM;
        hints.ai_protocol   = 0;
        hints.ai_canonname  = NULL;
        hints.ai_addr       = NULL;
        hints.ai_next       = NULL;

        rc = inet_pton(AF_INET, ip[i], &serveraddr);
        if (rc == 1)
        {
            /* IPv4 */
            //printf("[app_client] : IPv4 address\n");
            hints.ai_family = AF_INET;
            hints.ai_flags |= AI_NUMERICHOST;
        }
        else
        {
            rc = inet_pton(AF_INET6, ip[i], &serveraddr);
            if (rc == 1)
            {
                /* IPv6 */
                //printf("[app_client] : IPv6 address\n");
                hints.ai_family = AF_INET6;
                hints.ai_flags |= AI_NUMERICHOST;
            }
            else
            {
                if (debug) printf("[app_client] : Wrong address\n");
                continue;
            }
        }
        if (!user_def_port) port = DEFAULT_PORT;
        sprintf(port_str, "%d", port);

        /*
           If address is IPv6 link-local, fill
           the sin6_scope_id field after address,
           the format should be 'fe80::1%ne0'
        */
        sprintf(get_addr, "%s", ip[i]);
        if (AF_INET6 == hints.ai_family)
        {
            if (strlen(scope_if))
            {
                strcat(get_addr, "%");
                strcat(get_addr, scope_if);
            }
        }

        rc = getaddrinfo(get_addr, port_str, &hints, &res);
        if (rc != 0)
        {
            if (debug) printf("%s[%d] Host not found (rc): [%s]\n", __func__, __LINE__, gai_strerror(rc));
            continue;
        }

        sfd[i] = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if (sfd[i] < 0)
        {
            if (debug) printf("[app_client] : Failed to create IPv6 socket\n");
            continue;
        }

        FD_SET(sfd[i], &fds);

        if (setsockopt(sfd[i], SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
        {
            close(sfd[i]);
            sfd[i] = -1;
            if (debug) printf("[app_client] : Failed to setsockopt\n");
            continue;
        }

        rc = connect(sfd[i], res->ai_addr, res->ai_addrlen);
        if (rc < 0)
        {
            close(sfd[i]);
            sfd[i] = -1;

            if(multiple_address && count)
            {
                printf(", ");
            }

            printf("{\"%sResult\":\"ERROR_HOST_UNREACHABLE\",\"TargetIP\":\"%s\"}", api_ptr, get_addr);
            count++;
        }

        if (sfd[i] > max_fd)
        {
            max_fd = sfd[i];
        }
    }

    if (encryption)
    {
        if(ERROR == encrypt_payload(payload, &payload_len))
        {
            goto out_socket;
        }
    }

    for(i = 0; (i < MAX_SOCKET_NUM); i++)
    {
        if(sfd[i] > 0)
        {
            memset(buf, 0x00, sizeof(buf));

            len = sprintf(buf, HTTP_REQUEST_PACKET_FORMAT, http_method, api, ip[i], port, payload_len, payload);
            if(debug)
            {
                printf("\n============================== sent data [%d] =============================\n",len);
                printf("%s", buf);
                printf("\n===========================================================================\n");
            }

            send(sfd[i], buf, len, 0);
            send_count++;
        }
    }

    /* End process if there is no connection. */
    if(0 == send_count)
    {
        goto out_socket;
    }

    select_timeout.tv_sec = 15;
    select_timeout.tv_usec = 0;

    while(1)
    {

        fds_select = fds;
        result = select(max_fd+1, &fds_select, NULL, NULL, &select_timeout);
        if (result < 0)
        {
            if (debug) printf("[app_client] : Failed to select. result [%d]\n", result);
            printf("%s{\"%sResult\":\"ERROR_SOCKET_SELECT\"}", (multiple_address && count)?", ":"", api_ptr);
            goto out_socket;
        }
        if (0 == result)
        {
            printf("%s{\"%sResult\":\"ERROR_SOCKET_SELECT_TIMEOUT\"}", (multiple_address && count)?", ":"", api_ptr);
            break;
        }

        for (i = 0; (i < MAX_SOCKET_NUM); i++)
        {
            if ((sfd[i] > 0) && FD_ISSET(sfd[i], &fds_select))
            {
                total_bytes = 0;
                memset(recv_buf, 0x00, sizeof(recv_buf));

                do
                {
                    bzero(buf, sizeof(buf));
                    bytes = recv(sfd[i], buf, sizeof(buf) - 1, 0);
                    if(bytes > 0)
                    {
                        buf[bytes] = '\0';

#if APP_CLIENT_DEBUG
                        if(debug)
                        {
                            DEBUG("\n========================= received encrypted data =========================\n");
                            DEBUG("%s", buf);
                            DEBUG("\n===========================================================================\n");
                        }
#endif

                        if(NULL != (ptr = strstr(buf, HTTP_CONTENT_TYPE_RESPONSE_FORMAT)))
                        {
                            ptr += strlen(HTTP_CONTENT_TYPE_RESPONSE_FORMAT);

                            total_bytes += sprintf(recv_buf + total_bytes, "%s", ptr);
                        }
                        else
                        {
                            total_bytes += sprintf(recv_buf + total_bytes, "%s", buf);
                        }
                    }
                }
                while ( bytes > 0 );

                if(encryption && total_bytes > 0)
                {
                    if(ERROR == decrypt_payload(recv_buf, total_bytes))
                    {
                        continue;
                    }
                }

                if(multiple_address && count)
                {
                    printf(", ");
                }

                if(debug) printf("\n============================== received data ==============================\n");
                printf("%s", recv_buf);
                if(debug) printf("\n===========================================================================\n");

                ret = 0; //ok, for shell use

                FD_CLR(sfd[i], &fds);
                close(sfd[i]);
                sfd[i] = -1;
                receive_count++;
                count++;
                break;
            }
        }

        if(send_count == receive_count)
        {
            // Get all the responses, end the while loop.
            break;
        }

        usleep(100);
    }

out_socket:
    if(multiple_address)
    {
        printf("]");
    }


    for(i = 0; (i < MAX_SOCKET_NUM); i++)
    {
        if(sfd[i] > 0)
        {
            close(sfd[i]);
            sfd[i] = -1;
        }
    }
    if (res != NULL)
        freeaddrinfo(res);

out_main:
    return ret;
}
