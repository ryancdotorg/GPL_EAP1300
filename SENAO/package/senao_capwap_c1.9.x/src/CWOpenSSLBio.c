/*******************************************************************************************
 * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	       *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.                *
 *                                                                                         *
 * You should have received a copy of the GNU General Public License along with this       *
 * program; if not, write to the:                                                          *
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,                    *
 * MA  02111-1307, USA.                                                                    *
 *                                                                                         *
 * --------------------------------------------------------------------------------------- *
 * Project:  Capwap                                                                        *
 *                                                                                         *
 * Author :  Ludovico Rossi (ludo@bluepixysw.com)                                          *
 *           Del Moro Andrea (andrea_delmoro@libero.it)                                    *
 *           Giovannini Federica (giovannini.federica@gmail.com)                           *
 *           Massimo Vellucci (m.vellucci@unicampus.it)                                    *
 *           Mauro Bisson (mauro.bis@gmail.com)                                            *
 *******************************************************************************************/


#include "CWCommon.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

static int memory_write(BIO *h, const char *buf, int num);
static int memory_read(BIO *h, char *buf, int size);
static int memory_puts(BIO *h, const char *str);
static long memory_ctrl(BIO *h, int cmd, long arg1, void *arg2);
static int memory_new(BIO *h);
static int memory_free(BIO *data);

#ifndef IP_MTU
#define IP_MTU      14
#endif

typedef struct
{
    CWSocket sock;
    CWNetworkLev4Address sendAddress;
    unsigned int nMtu;
    CWSecurityRxCB cbRx;
    void* cbRxArg;
    int controllerId;
} BIO_memory_data;

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
inline BIO_METHOD* CreateMemoryMethods()
{
  BIO_METHOD* methods_memory = BIO_meth_new (BIO_TYPE_DGRAM, "memory packet");
  if (methods_memory) {
    BIO_meth_set_write (methods_memory, memory_write);
    BIO_meth_set_read (methods_memory, memory_read);
    BIO_meth_set_puts (methods_memory, memory_puts);
    BIO_meth_set_ctrl (methods_memory, memory_ctrl);
    BIO_meth_set_create (methods_memory, memory_new);
    BIO_meth_set_destroy (methods_memory, memory_free);
  }
  return methods_memory;
}


static pthread_mutex_t g_methods_memory_mutex = PTHREAD_MUTEX_INITIALIZER;
static BIO_METHOD *g_methods_memory = NULL;

void memoryFreeMethods()
{
  BIO_meth_free (g_methods_memory);
  g_methods_memory = NULL;
}

inline BIO_METHOD* BIO_s_memory()
{
  pthread_mutex_lock(&g_methods_memory_mutex);

  BIO_METHOD* methods = g_methods_memory;
  if (!methods) {
    methods = CreateMemoryMethods();
    if (methods) {
      g_methods_memory = methods;
      atexit(&memoryFreeMethods);
    }
  }

  pthread_mutex_unlock(&g_methods_memory_mutex);

  return methods;
}
#else
static BIO_METHOD methods_memory =
{
    BIO_TYPE_DGRAM,
    "memory packet",
    memory_write,
    memory_read,
    memory_puts,
    NULL, /* dgram_gets, */
    memory_ctrl,
    memory_new,
    memory_free,
    NULL,
};

BIO_METHOD *BIO_s_memory(void)
{
    return(&methods_memory);
}
#define BIO_set_init(x, v)   (x->init=v)
#define BIO_set_data(x, v)   (x->ptr=v)
#define BIO_get_data(x)      (x->ptr)
#endif

BIO *BIO_new_memory(CWSocket sock, CWNetworkLev4Address *pSendAddress, CWSecurityRxCB cbRx, void *cbRxArg, int controllerId)
{
    BIO_memory_data *pData;

    BIO *bio = BIO_new(BIO_s_memory());

    if(bio == NULL)
    {
        return NULL;
    }

    CW_CREATE_OBJECT_SIZE_ERR(pData, sizeof(BIO_memory_data), return 0;);

    pData->sock = sock;
    CW_COPY_MEMORY(&pData->sendAddress, pSendAddress, sizeof(CWNetworkLev4Address));
    pData->cbRx = cbRx;
    pData->cbRxArg = cbRxArg;
    pData->controllerId = controllerId;
    pData->nMtu = 0;

    BIO_set_data (bio, pData);

    return bio;
}

static int memory_new(BIO *b)
{
    BIO_memory_data *pData = (BIO_memory_data *) BIO_get_data(b);
    BIO_set_init (b, 1);
    BIO_set_data (b, NULL);
    BIO_set_flags (b, ~0);

    return 1;
}

static int memory_free(BIO *b)
{
    if(!b)
    {
        return 0;
    }

    BIO_memory_data *pData = (BIO_memory_data *) BIO_get_data(b);

    if(!pData)
    {
        // return 0;
    }
    else
    {
        CW_FREE_OBJECT(pData);
    }

    BIO_set_data(b, NULL);
    BIO_set_init(b, 0);
    BIO_clear_flags(b, ~0);

    return 1;
}

static int memory_read(BIO *b, char *out, int outl)
{
    int ret = -1;
    char *buf;
    int size;
    BIO_memory_data *pData = (BIO_memory_data *) BIO_get_data(b);

    //
    //BIO_clear_retry_flags(b);
    //
    size = pData->cbRx(pData->cbRxArg, &buf);
    if(size <= 0)
    {
        return -1;
    }

    /* Check payload is encrypted  */
    if(buf[0] & CW_PACKET_CRYPT)
    {
        /* ignore first 4 bytes which are not encrypted */
        size -= CW_DTLS_HEADER_SIZE;
        if(size <= 0)
        {
            CWLog("Received Payload size < 4 bytes");
        }
        else
        {
            ret = size < outl ? size : outl;
            CW_COPY_MEMORY(out, &buf[CW_DTLS_HEADER_SIZE], ret);
        }
    }
    else
    {
        CWLog("Not an ecrypted Payload");
    }
    CW_FREE_OBJECT(buf);

    return ret;
}

static int memory_write(BIO *b, const char *in, int inl)
{
    int ret = -1, hdrLen;
    BIO_memory_data *pData = (BIO_memory_data *) BIO_get_data(b);
    char buf[CW_PACKET_BUFFER_SIZE];
    short *cid;

    if(pData->controllerId != 0)
    {
        hdrLen = CW_CID_HEADER_SIZE + CW_DTLS_HEADER_SIZE;

        /* add tag header */
        buf[0] = CW_PACKET_CID_TAG; /* CID type */
        cid = (short *) &buf[1];
        *cid = htons(pData->controllerId); /* controller id */
        buf[3] = 0;
        /* add dtls header */
        buf[4] = (char)(CW_PROTOCOL_VERSION << 4) | (char)(CW_PACKET_CRYPT);
        buf[5] = buf[6] = buf[7] = 0;
    }
    else
    {
        hdrLen = CW_DTLS_HEADER_SIZE;

        /* add dtls header */
        buf[0] = (char)(CW_PROTOCOL_VERSION << 4) | (char)(CW_PACKET_CRYPT);
        buf[1] = buf[2] = buf[3] = 0;
    }

    CW_ASSERT(inl <= sizeof(buf) - hdrLen);

    /* copy the data will be encrypted */
    CW_COPY_MEMORY(&buf[hdrLen], in, inl);

    errno = 0;
    ret = sendto(pData->sock, buf, inl + hdrLen, 0, (struct sockaddr *)&pData->sendAddress, sizeof(struct sockaddr_storage));

    //BIO_clear_retry_flags(b);
    if(ret <= 0)
    {
        if(errno == EINTR)
        {
            BIO_set_retry_write(b);
        }
    }
    else if(ret > hdrLen)
    {
        ret -= hdrLen;
    }
    else
    {
        CWLog("No packet Send");
        ret = 0;
    }

    return ret;
}

static long memory_ctrl(BIO *b, int cmd, long num, void *ptr)
{
    long ret = 1;
    long sockopt_val = 0;
    unsigned int sockopt_len = 0;
    BIO_memory_data *pData = (BIO_memory_data *) BIO_get_data(b);

    switch(cmd)
    {
        case BIO_CTRL_RESET:
            ret = 0;
            break;
        case BIO_CTRL_EOF:
            ret = 0;
            break;
        case BIO_CTRL_INFO:
            ret = 0;
            break;
        case BIO_CTRL_GET_CLOSE:
            ret = 0;
            break;
        case BIO_CTRL_SET_CLOSE:
            ret = 1;
            break;
        case BIO_CTRL_WPENDING:
            ret = 0;
            break;
        case BIO_CTRL_PENDING:
            ret = 0;
            break;
        case BIO_CTRL_DUP:
            ret = 1;
            break;
        case BIO_CTRL_FLUSH:
            ret = 1;
            break;
        case BIO_CTRL_PUSH:
            ret = 0;
            break;
        case BIO_CTRL_POP:
            ret = 0;
            break;
        case BIO_CTRL_DGRAM_QUERY_MTU:
        {
            if(ptr == NULL && pData->nMtu)
            {
                ret = pData->nMtu;
            }
            else
            {
                if(ptr == NULL || *((unsigned int *) ptr) == 0)
                {
                    sockopt_len = sizeof(sockopt_val);
                    if((ret = getsockopt(pData->sock, IPPROTO_IP, IP_MTU, (void *)&sockopt_val, &sockopt_len)) < 0 || sockopt_val < 0)
                    {
                        /* default to 1500 when the query fail */
                        sockopt_val = 1500;
                    }
                }
                else
                {
                    sockopt_val = *((unsigned int *) ptr);
                }

                /* minus ether header + ipv4 header(no options) + udp header + dtls header + (buffer for ipv4 options + dtls others) */
                pData->nMtu = sockopt_val - 18 - 20 - 8 - CW_DTLS_HEADER_SIZE - 20;
                if(pData->controllerId)
                {
                    pData->nMtu -= CW_CID_HEADER_SIZE;
                }
                ret = pData->nMtu;
            }
            break;
        }
        case BIO_CTRL_DGRAM_GET_MTU:
            ret = pData->nMtu;
            break;
        case BIO_CTRL_DGRAM_SET_MTU:
            pData->nMtu = num;
            ret = num;
            break;
        case BIO_CTRL_DGRAM_GET_PEER:
            CW_COPY_MEMORY(ptr, &pData->sendAddress, sizeof(CWNetworkLev4Address));
            ret = 0;
            break;
        case BIO_CTRL_DGRAM_SET_NEXT_TIMEOUT:
            ret = 0;
            break;
        default:
            ret = 0;
            break;
    }

    return ret;
}

static int memory_puts(BIO *bp, const char *str)
{
    return memory_write(bp, str, strlen(str));
}

