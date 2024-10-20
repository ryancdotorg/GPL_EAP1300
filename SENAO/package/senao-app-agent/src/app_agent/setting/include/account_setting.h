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
;    Creator : Robert Hong
;    File    : account_setting.h
;    Abstract: HNAP methods for device management
;
;       Modification History:
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	-------------------------------------
;		Robert			2014/05/14			First commit
;****************************************************************************/
#ifndef _ACCOUNT_SETTING_H_
#define _ACCOUNT_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "../../appagents.h"

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
int get_user_list_cb(HTTPS_CB *pkt);
int update_user_cb(HTTPS_CB *pkt);
int remove_user_cb(HTTPS_CB *pkt);
int get_account_password_cb(HTTPS_CB *pkt);
int set_account_password_cb(HTTPS_CB *pkt);
#endif
