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
;    Project : app_agent
;    Creator : ynyang
;    File    : sitecom_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	-------------------------------------
;		ynyang			2013/07/08			First commit
;****************************************************************************/

#ifndef _SITECOM_SETTING_H_
#define _SITECOM_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "deviceinfo.h"
#include "gconfig.h"
#include "../../appagents.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/
#define STREAM_BOOST_MAX_UPLOAD_LIMIT    1000 //Mbps
#define STREAM_BOOST_MAX_DOWNLOAD_LIMIT  1000 //Mbps
/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/
typedef struct _STREAM_BOOST_DATA_ {
    bool enabled;
    int upLimit;
    int downLimit;
} STREAM_BOOST_DATA;
/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
#if 1//HAS_SC_AUTO_FW_CHECK
int get_auto_fw_upgrade_status_cb(HTTPS_CB *pkt);
#if 0
int set_auto_fw_upgrade_status_cb(HTTPS_CB *pkt);
#endif
#endif
#if HAS_SC_UTMPROXY_FUNCTION
int get_scs_status_cb(HTTPS_CB *pkt);
int set_scs_status_cb(HTTPS_CB *pkt);
#endif
#if HAS_STREAM_BOOST_SETTING
int get_stream_boost_settings_cb(HTTPS_CB *pkt);
int set_stream_boost_settings_cb(HTTPS_CB *pkt);
#endif
#endif
