/*****************************************************************************
;
;   (C) Unpublished Work of SENAO Networks Incorporated.  All Rights Reserved.
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
;------------------------------------------------------------------------------
;
;    Project : 
;    Creator : David Chang
;    File    : httpd.h
;    Abstract: Simple httpd daemon
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       David           20101014        Newly Create
;*****************************************************************************/
#ifndef _HTTPD_H
#define _HTTPD_H

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/*This port configuration can be customerization*/
#define HTTP_PORT   80

/*HTTPD state*/
enum 
{
    HTTP_GET = 0,
    HTTP_POST_START,
    HTTP_POST_COND,
    HTTP_MAX_STATES /* Leave at the end! */
};

/*-----------------------------------------------------------------------------
                    data declarations, extern, static, const
 ----------------------------------------------------------------------------*/
extern int g_httpd_started;

extern unsigned char readyToUpgrade;
extern unsigned long g_httpd_upload_mem;

#if defined(Realtek_platform)
extern unsigned long load_addr; //senao
#endif  /* defined(CONFIG_AM33XX) */

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
extern int http_send_main_page(void);
extern int http_upload_file(unsigned char *payload , int length, unsigned char http_state);

#endif /* #ifndef _HTTPD_H */





