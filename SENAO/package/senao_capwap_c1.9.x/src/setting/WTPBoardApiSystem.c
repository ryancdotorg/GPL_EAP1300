#include "WTPBoardApiSystem.h"
#include "WTPBoardApiCommon.h"
#include "WTPBoardApiNetwork.h"
#if SUPPORT_CONFIG_HAS_SECTIONNAME || SUPPORT_WLAN_5G_2_SETTING
#include "WTPBoardApiWireless_wn.h"
#else
#include "WTPBoardApiWireless.h"
#endif
#include "CWVersion.h"
#include <ctype.h>
#include <sysWlan.h>
#include <api_tokens.h>
#include <variable/variable.h>
#include <gconfig.h>
#include <sysCore.h>
#include <sysFile.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

int CWWTPBoardGetRebootTime(CWBool factory)
{
    /* estimating time */
    return factory ? FACTORY_REBOOT_TIME : REBOOT_TIME;

}

int CWWTPBoardGetImageRebootTime(char *imagePath)
{
    /* estimating time */
    return IMAGE_REBOOT_TIME;
}
//NOT_FINISH
int CWWTPBoardGetImageBurningTime(char *imagePath)
{
    FILE *fp;
    int fileSize;

    fp = fopen(imagePath, "r");
    if(!fp)
    {
        CWLog("Cannot open %s", imagePath);
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    fileSize = ftell(fp);

    fclose(fp);

    /* measure by file size */
    //return ((fileSize * 80) / (6886400)) + 1;
    CWDebugLog("%s %d fileSize:[%d] burnTime:[%d]", __FUNCTION__, __LINE__, fileSize, (fileSize / 45846));
    return (fileSize / 45846);
}

int CWWTPBoardGetSystemUpTime()
{
    char val[63 + 1];
    int uptime = 0;

    memset(val, 0, sizeof(val));

    if(sysutil_interact(val, sizeof(val), "cat /proc/uptime") > 0)
    {
        val[strcspn(val, "\n")] = '\0';
        uptime = atoi(val);
    }
    CWDebugLog("%s %d uptime:[%d]", __FUNCTION__, __LINE__, uptime);
    return uptime;
}

int CWWTPBoardGetOutdoor()
{
    int outdoor = 0;

    if(api_get_integer_option("sysProductInfo.model.outdoor", &outdoor))
    {
        outdoor = 0;
    }
    CWDebugLog("%s %d outdoor:[%d]", __FUNCTION__, __LINE__, outdoor);
    return outdoor;
}

CWBool CWWTPBoardGetHardwareVersion(char **pstr)
{
    CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CWDebugLog("%s %d HardwareVersion:[%s]", __FUNCTION__, __LINE__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardGetSoftwareVersion(char **pstr)
{
    char *val;
    unsigned int verNum[3];

    if(!CWCreateStringByFile("/etc/version", &val))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if(sscanf(val, "%u.%u.%u", &verNum[0], &verNum[1], &verNum[2]) != 3)
    {
        CW_FREE_OBJECT(val);
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    CW_FREE_OBJECT(val);

    CW_CREATE_STRING_ERR(*pstr, MAX_STRING_LENGTH_32, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    sprintf(*pstr, "v%u.%u.%u_c%u.%u.%u", verNum[0], verNum[1], verNum[2], CW_VERSION_MAJOR, CW_VERSION_MINOR, CW_VERSION_TARGET);

    CWDebugLog("%s %d sw version:[%s]", __FUNCTION__, __LINE__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardGetBootVersion(char **pstr)
{
    CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CWDebugLog("%s %d BootVersion:[%s]", __FUNCTION__, __LINE__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardGetNameCfg(char **pstr)
{
    char *val;

    CWDebugLog("%s %d ", __FUNCTION__, __LINE__);

    CW_CREATE_STRING_ERR(val, MAX_STRING_LENGTH_32, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(api_get_string_option(SYSTEM_SYSTEM_SYSTEMNAME_OPTION, val, MAX_STRING_LENGTH_32+1))
    {
        CW_FREE_OBJECT(val);
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    *pstr = val;

    CWDebugLog("%s %s %s", __FUNCTION__, *pstr, val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetNameCfg(char *pstr)
{
    CWDebugLog("%s %s", __FUNCTION__, pstr);

    api_set_string_option(SYSTEM_SYSTEM_SYSTEMNAME_OPTION, pstr, strlen(pstr));

    return CW_TRUE;
}

CWBool CWWTPBoardGetLocationCfg(char **pstr)
{
    char *val;

    CW_CREATE_STRING_ERR(val, MAX_STRING_LENGTH_32, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(api_get_string_option(SYSTEM_SYSTEM_LOCATION_OPTION, val, MAX_STRING_LENGTH_32+1))
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        CW_FREE_OBJECT(val);
    }
    else
    {
        *pstr = val;
    }

    CWDebugLog("%s %d Location:[%s]", __FUNCTION__, __LINE__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLocationCfg(char *pstr)
{
    CWDebugLog("%s %s", __FUNCTION__, pstr);

    api_set_string_option(SYSTEM_SYSTEM_LOCATION_OPTION, pstr, strlen(pstr));

    return CW_TRUE;
}

CWBool CWWTPBoardGetDefaultInterfaceName(char **pstr)
{
    CW_CREATE_STRING_FROM_STRING_ERR(*pstr, LAN_IF_NAME, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CWDebugLog("%s %d DefaultInterface:[%s]", __FUNCTION__, __LINE__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardBurnImage(char *imagePath)
{
    //char buf[31+1];

    //CWDebugLog("%s %d imagePath:[%s]", __FUNCTION__, __LINE__, imagePath);

    //CWSystem("tar -zxvf %s -C /tmp", imagePath);

    //CWWaitSec(1); /* wait 1 */

    //sysutil_interact(buf, sizeof(buf), "ls /tmp/*.bin");

    //CWDebugLog("%s %d buf:[%s]", __FUNCTION__, __LINE__, buf);

    //buf[strcspn(buf,"\n")] = '\0';

    //CWDebugLog("%s %d buf:[%s]", __FUNCTION__, __LINE__, buf);
#if SUPPORT_SENAOWRT_IMAGE
    CWSystem("/sbin/snwrtupgrade %s %s", imagePath, "keep");
#else
    CWSystem("killall uhttpd");
    CWSystem("sleep 1");
    CWSystem("/sbin/sysupgrade %s", imagePath);
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardReboot()
{
#if SUPPORT_FORCE_REBOOT
	CWSystem("/bin/sync; /sbin/reboot -f");
#else
    CWSystem("/bin/sync; /sbin/reboot");
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetAdminCfg(char **pstr)
{
    char *val;

#if SUPPORT_MULTI_ACCOUNT
    if(!(val = CWCreateStringByCmdStdout("/lib/auth.sh get_multi_username 1")))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }
#else
    if(!(val = CWCreateStringByCmdStdout("/lib/auth.sh get_username")))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }
#endif

    val[strcspn(val, "\n")] = '\0';

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, val);

    *pstr = val;

    return CW_TRUE;
}

CWBool CWWTPBoardSetAdminCfg(char *pstr)
{
    char *pUciStr;

    CWDebugLog("%s %s", __FUNCTION__, pstr);

    if(pstr[0] == '\0')
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "empty admin length");
    }

    pUciStr = CWCreateUciString(pstr);

    CWDebugLog("%s %d pUciStr:[%s]", __FUNCTION__, __LINE__, pUciStr);

#if SUPPORT_MULTI_ACCOUNT
    CWSystem("/lib/auth.sh mod_multi_username 1 \"%s\"", pUciStr);
#else
    CWSystem("/lib/auth.sh set_usrname \"%s\"", pUciStr);

#endif
    CW_FREE_OBJECT(pUciStr);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPasswordMD5Cfg(unsigned char **md5)
{
    char *val, *c;
    int i;

#if SUPPORT_MULTI_ACCOUNT
    if(!(val = CWCreateStringByCmdStdout("/lib/auth.sh get_multi_password 1")))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }
#else
    if(!(val = CWCreateStringByCmdStdout("/lib/auth.sh get_password")))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }
#endif

    val[strcspn(val, "\n")] = '\0';

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, val);

    c = val;

    CW_CREATE_OBJECT_SIZE_ERR(*md5, 16,
    {
        CW_FREE_OBJECT(val);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    for(i = 0; i < 16; i++, c += 2)
    {
        (*md5)[i] = *c >= 'a' ? (*c - 'a' + 10) * 16 : (*c - '0') * 16;
        (*md5)[i] += *(c + 1) >= 'a' ? (*(c + 1) - 'a' + 10) : (*(c + 1) - '0');
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPasswordMD5Cfg(unsigned char *md5)
{
    CWDebugLog("%s %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", __FUNCTION__,
               md5[0], md5[1], md5[2], md5[3],
               md5[4], md5[5], md5[6], md5[7],
               md5[8], md5[9], md5[10], md5[11],
               md5[12], md5[13], md5[14], md5[15]);

#if SUPPORT_MULTI_ACCOUNT
    int i;

    for(i = 0; i < 16; i++)
    {
        if(md5[i] != 0)
        {
            break;
        }
    }
    if(i == 16)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "zero password md5");
    }

    CWSystem("/lib/auth.sh mod_multi_password 1 %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
             md5[0], md5[1], md5[2], md5[3],
             md5[4], md5[5], md5[6], md5[7],
             md5[8], md5[9], md5[10], md5[11],
             md5[12], md5[13], md5[14], md5[15]);
#else
    char *val, *c;
    FILE *fp;
    int i;

    CWDebugLog("%s %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", __FUNCTION__,
               md5[0], md5[1], md5[2], md5[3],
               md5[4], md5[5], md5[6], md5[7],
               md5[8], md5[9], md5[10], md5[11],
               md5[12], md5[13], md5[14], md5[15]);

    for(i = 0; i < 16; i++)
    {
        if(md5[i] != 0)
        {
            break;
        }
    }
    if(i == 16)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "zero password md5");
    }


    if(!CWCreateStringByFile("/etc/webpasswd", &val))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }


    c = strstr(val, ":");
    if(!c)
    {
        CW_FREE_OBJECT(val);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    *c = '\0';

    if(!(fp = fopen("/etc/webpasswd", "w")))
    {
        CW_FREE_OBJECT(val);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    fprintf(fp, "%s:%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
            val,
            md5[0], md5[1], md5[2], md5[3],
            md5[4], md5[5], md5[6], md5[7],
            md5[8], md5[9], md5[10], md5[11],
            md5[12], md5[13], md5[14], md5[15]);
    fclose(fp);

    CW_FREE_OBJECT(val);

#endif

    return CW_TRUE;
}

#if SUPPORT_WLAN_5G_2_SETTING
CWBool CWWTPBoardGetApCfgInitMode(CWBool *enable)
{
    int val = 0;

    if(api_get_integer_option("apcontroller.capwap.apCfgInit", &val))
    {
        val = 0;
    }

    *enable = val;
    return CW_TRUE;
}

CWBool CWWTPBoardSetApCfgInitMode(CWBool enable)
{
    if(api_set_integer_option("apcontroller.capwap.apCfgInit", enable))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }
    return CW_TRUE;
}
#endif

CWBool CWWTPBoardSetAcMode(CWBool enable)
{
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, enable);
#if SUPPORT_MESH_SETTING
    int val = 0;

    if(api_get_integer_option("functionlist.functionlist.CONTROLLER_MODE_SUPPORT_MESH", &val))
    {
        val = 0;
    }
#endif

    if(enable)
    {
        CWSystem("uci set apcontroller.capwap.enable=1");
        CWSystem("uci set cfg_cli.@cli[0].cli_enable=0");
        CWSystem("uci set snmpd.@system[0].sysEnable=0");
        CWSystem("uci set snmpd.@snmpv3[0].snmpv3Enable=0");
#if AP_MODE_SUPPORT_MESH_EZSETUP
        CWSystem("uci set network.sys.EnMesh=0");
        if(sysutil_check_file_existed("/sbin/senao_check.sh"))
                CWSystem("sh /sbin/senao_check.sh");
#endif
#if SUPPORT_COMBINED_SSID_SETTING /*only support SmartWrt: bandSteer/portal  per ssid*/
        CWSystem("uci set portal.general.enable=1");
        CWSystem("luci-reload auto");
        CWSystem("/etc/init.d/fingerprint reload");
#else
        /*CWSystem("luci-reload snmpd apcontroller"); uncomment if we don't need telnet backdoor */
        CWSystem("uci commit");
        CWSystem("luci-reload snmpd apcontroller");
#endif
    }
    else
    {
        CWSystem("uci set apcontroller.capwap.enable=0");
        CWSystem("uci set cfg_cli.@cli[0].cli_enable=1");
        CWSystem("uci set snmpd.@system[0].sysEnable=1");
        CWSystem("uci set snmpd.@snmpv3[0].snmpv3Enable=1");

#if SUPPORT_MESH_SETTING
        if (val == 1)
        {
            CWSystem("uci set mesh.wifi.disabled=1");
        }
#endif

#if SUPPORT_COMBINED_SSID_SETTING /*only support SmartWrt: bandSteer/portal  per ssid*/
        CWSystem("uci set portal.general.enable=0");
        CWSystem("sh /usr/sbin/capwap_sn_event.sh standalone");
        CWSystem("luci-reload auto");
        CWSystem("/etc/init.d/fingerprint reload");
#else
        /*CWSystem("luci-reload snmpd apcontroller");  uncomment if we don't need telnet backdoor */
        CWSystem("uci commit");
        CWSystem("luci-reload snmpd apcontroller");
#endif
    }
    return CW_TRUE;
}

CWBool CWWTPBoardFactoryReset()
{
    CWDebugLog("%s", __FUNCTION__);

    CWSystem("killall dropbear uhttpd; sleep 1; /sbin/sysresetdef; reboot -f");

    return CW_TRUE;
}

CWBool CWWTPBoardGetAcAddress(CWAcAddress *acAddr)
{
    char *val;
    CWBool ret = CW_TRUE;

    if(!(val = CWCreateStringByUci("apcontroller.capwap.ac")))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    ret = CWParseAcAddrString(val, acAddr);

    CW_FREE_OBJECT(val);

    return ret;
}

CWBool CWWTPBoardSetAcAddress(CWAcAddress *acAddr)
{
    char *pstr, *val, *pch, *ptr = NULL;
    int rdidx;

    CWDebugLog("%s %s:%u#%d", __FUNCTION__,
               acAddr->hostName, acAddr->port, acAddr->controllerId);

    if(acAddr->hostName[0] == '\0')
    {
        CWSystem("uci set apcontroller.capwap.ac=");
    }
    else
    {
        if(!CWCreateAcAddrString(acAddr, &pstr))
        {
            return CW_TRUE;
        }

        CWSystem("uci set apcontroller.capwap.ac='%s'", pstr);

        //////////////////////////////////
        /*
         *    If ac address has been changed and cative portal is enabled,
         *    the portal_AC_IP  in chilli's config could not be updated.
         *    Hence, we set PortalEnable=0 to trigger reload captive portal.
         */
        if(access("/tmp/etc/chilli/config", R_OK) == 0)
        {
            if(!(val = CWCreateStringByCmdStdout("cat /tmp/etc/chilli/config 2>/dev/null \
                | grep HS_UAMFORMAT | awk '{FS=\"/\"} {print $3}' | awk '{FS=\":\"} {print $1}'")))
            {
                CW_FREE_OBJECT(pstr);
                CWDebugLog("Get configuration fail");
                return CW_TRUE;
            }

            pch = strtok_r(pstr, ":", &ptr);
            if(pch != NULL)
            {
                for(rdidx = 0; rdidx < CWWTPBoardGetMaxRadio(); rdidx++)
                {
                    if(strncmp(pch, val, strlen(pch)))
                    {
                        CWWTPBoardSetPortalEnableCfg(rdidx, CW_FALSE);
                    }
                }
            }

            CW_FREE_OBJECT(val);
        }
        //////////////////////////////////

        CW_FREE_OBJECT(pstr);
    }
#if 0 // revert [coova-chilli] each ssid GuestNetwork Disable/Bridge/NAT mode
    if(access("/tmp/etc/chilli/config", R_OK) == -1)
    {
       CWSystem("luci-reload auto portal &");
    }
#endif
    CWSystem("uci commit");

    return CW_TRUE;
}

CWBool CWWTPBoardGetAcListCfg(int *count, CWHostName **hostName)
{
    char *val;
    CWBool ret;

    if(!(val = CWCreateStringByUci("apcontroller.capwap.ac_list")))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    ret = CWParseAcList(val, hostName, count);

    CW_FREE_OBJECT(val);

    return ret;

}

CWBool CWWTPBoardSetAcListCfg(int count, CWHostName *hostName)
{
    char *val;
    int i, len = 0;

    if(count)
    {
        CW_CREATE_OBJECT_SIZE_ERR(val, count * sizeof(CWHostName),
        {
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });

        for(i = 0; i < count; i++)
        {
            len += sprintf(&val[len], "%s%s", i == 0 ? "" : ",", hostName[i]);
        }

        CWSystem("uci set apcontroller.capwap.ac_list=\"%s\"", val);

        CW_FREE_OBJECT(val);
    }
    else
    {
        CWSystem("uci set apcontroller.capwap.ac_list=\"\"");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetForceAcAddress(CWHostName acAddr)
{
    char *val;

    if(!(val = CWCreateStringByUci("apcontroller.capwap.force_ac")))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    strcpy(acAddr, val);

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetForceAcAddress(const CWHostName acAddr)
{
    if(acAddr)
    {
        CWSystem("uci set apcontroller.capwap.force_ac='%s'", acAddr);
    }
    else
    {
        CWSystem("uci set apcontroller.capwap.force_ac=''");
    }

    CWSystem("uci commit");

    return CW_TRUE;
}

CWBool CWWTPBoardGetLogRemoteEnable( int *Enabled)
{
    int val = CW_FALSE;

    if (api_get_bool_option(SYSTEM_SYSTEM_LOG_REMOTELOG_ENABLE_OPTION, &val)) 
    {
        val = CW_FALSE;
    }

    *Enabled = val;

    CWDebugLog("%s %d Enabled:[%d]", __FUNCTION__, __LINE__, *Enabled);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLogRemoteEnable( int Enabled)
{
    CWDebugLog("%s %d Enabled:[%d]", __FUNCTION__, __LINE__, Enabled);

    if (api_set_bool_option(SYSTEM_SYSTEM_LOG_REMOTELOG_ENABLE_OPTION, Enabled)) 
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetLogRemoteCfg( char **pcfg)
{
    char ipaddr[15+1]={0};
    char *logRemote = NULL;
    int port = 0;

    if (api_get_string_option(SYSTEM_SYSTEM_LOG_IP_OPTION, ipaddr, sizeof(ipaddr))) 
    {
        return CW_TRUE;
    }
    if (api_get_integer_option(SYSTEM_SYSTEM_LOG_PORT_OPTION, &port)) 
    {
        port = 514; //default port
    }

    CW_CREATE_STRING_ERR(logRemote, 22, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    snprintf(logRemote, 22, "%s:%d", ipaddr, port);

    *pcfg = logRemote;

    CWDebugLog("%s %d pcfg:[%s]", __FUNCTION__, __LINE__, *pcfg);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLogRemoteCfg( char *pcfg)
{
    char *pstr = NULL, *pch = NULL, *ptr = NULL;
    int port = 0;

    CWDebugLog("%s %d pcfg:[%s][%d]", __FUNCTION__, __LINE__, pcfg, strlen(pcfg));

    if (pcfg == NULL)
    {
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(pstr, pcfg, strlen(pcfg), return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CWDebugLog("%s %d pstr:[%s]", __FUNCTION__, __LINE__, pstr);

    pch = strtok_r(pstr, ":", &ptr);
    CWDebugLog("%s %d pch:[%s]", __FUNCTION__, __LINE__, pch);
    if (pch == NULL)
    {
        CW_FREE_OBJECT(pstr);
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }
    if (api_set_string_option(SYSTEM_SYSTEM_LOG_IP_OPTION, pch, strlen(pch)))
    {
        CW_FREE_OBJECT(pstr);
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    pch = strtok_r(NULL, ":", &ptr);
    port = (pch == NULL)?514:atoi(pch);
    CWDebugLog("%s %d pch:[%s] port:[%d]", __FUNCTION__, __LINE__, pch, port);
    if (api_set_integer_option(SYSTEM_SYSTEM_LOG_PORT_OPTION, port))
    {
        CW_FREE_OBJECT(pstr);
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    CW_FREE_OBJECT(pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardSetUTCTime(int intime)
{
    return CW_TRUE;
}

CWBool CWWTPBoardGetUTCTime(int *outtime)
{
    return CW_TRUE;
}

CWBool CWWTPBoardGetTimeZone( int *timezone)
{
    return CW_TRUE;
}

CWBool CWWTPBoardSetTimeZone( int timezone)
{
    return CW_TRUE;
}

CWBool CWWTPBoardGetLogTrafficEnable(int *Enabled)
{
    int enable = CW_FALSE;

    if (api_get_integer_option("system.@system[0].trafficlog_enable", &enable))
    {
        enable = CW_FALSE;
    }

    *Enabled = enable;

    CWDebugLog("%s %d Enabled:[%d]", __FUNCTION__, __LINE__, *Enabled);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLogTrafficEnable(int Enabled)
{
    CWDebugLog("%s %d Enabled:[%d]", __FUNCTION__, __LINE__, Enabled);

    if (api_set_integer_option("system.@system[0].trafficlog_enable", Enabled))
    {
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetComEnable(void)
{
    CWDebugLog("EzCom is default enable");
    return CW_TRUE;
}
