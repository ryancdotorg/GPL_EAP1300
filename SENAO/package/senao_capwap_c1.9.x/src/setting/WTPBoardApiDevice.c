#include "WTPBoardApiDevice.h"
#include "WTPBoardApiCommon.h"
#include "WTPBoardApiSystem.h"
#include <ctype.h>
#include <sysWlan.h>
#include <sysCommon.h>
#include <api_tokens.h>
#include <variable/variable.h>
#include <variable/api_wireless.h>
#include <gconfig.h>
#include <autogconfig.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWBool CWWTPBoardGetModelName(char **pstr)
{
    char *modelName = NULL;

    //modelName = CWCreateStringEntry(MAX_MODELNAME_SIZE);
    CW_CREATE_STRING_ERR(modelName, MAX_MODELNAME_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    //memset(modelName, 0, sizeof(char)*MAX_MODELNAME_SIZE);
    if (api_get_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION, modelName, MAX_MODELNAME_SIZE+1))
    {
        return CW_FALSE;
    }

    *pstr = modelName;

    CWDebugLog("%s %d modelName:[%s]", __FUNCTION__, __LINE__, modelName);

    return CW_TRUE;
}

CWBool CWWTPBoardGetSku(char **pstr)
{
    int reg = 0;
    char *sku = NULL;

    //sku = CWCreateStringEntry(MAX_SKU_SIZE);
    CW_CREATE_STRING_ERR(sku, MAX_SKU_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (!sysutil_get_regular_domain_info(&reg))
    {
        return CW_FALSE;
    }

    switch (reg) 
    {
        case REG_FCC:
            sprintf(sku, "FCC");
            break;
        case REG_ETSI:
            sprintf(sku, "ETSI");
            break;
        case REG_INT:
            sprintf(sku, "INT");
            break;
        default:
            sprintf(sku, "INT");
            break;
    }
    *pstr = sku;

    CWDebugLog("%s %d sku:[%s]", __FUNCTION__, __LINE__, sku);

    return CW_TRUE;
}

CWBool CWWTPBoardGetCapCode(unsigned int *code)
{
    int outdoor = 0, radioNum = 0, antennaNum = 0, support11ac = 0, supportBand = 0;

    if (api_get_integer_option("sysProductInfo.model.outdoor", &outdoor)) {
        outdoor = 0;
    }
    antennaNum = ANTENNA_NUM;
    supportBand = RADIO_MODE; //2, 5, 7 for identify model's radio (Single Band:2, 5; Dual Band:7, Tri Band:12)
    radioNum = (supportBand==12)?3:(supportBand==7)?2:1;
#if HWMODE_AC || HWMODE_AX
    support11ac = 1;
#else
    support11ac = 0;
#endif
    if (outdoor == 0) {
        switch (radioNum)
        {
        case 1:
            if (supportBand == 2) {
                *code = (antennaNum == 2)?0x10:(antennaNum == 3)?0x30:(antennaNum == 4)?0x52:0x01;
            }
            else {
                *code = (antennaNum == 2)?0x11:(antennaNum == 3)?0x31:(antennaNum == 4)?0x51:0x02;
            }
            break;
        case 2:
            if (support11ac == 0) {
                *code = (antennaNum == 2)?0x12:(antennaNum == 3)?0x32:(antennaNum == 4)?0x52:0x03;
            }
            else {
                *code = (antennaNum == 2)?0x13:(antennaNum == 3)?0x33:(antennaNum == 4)?0x53:0x04;
            }
            break;
        case 3:
            *code = (antennaNum == 2)?0x16:(antennaNum == 3)?0x36:(antennaNum == 4)?0x56:0x07;
            break;
        default:
            break;
        }
    }
    else {
        switch (radioNum)
        {
        case 1:
            if (supportBand == 2) {
                *code = (antennaNum == 2)?0x20:(antennaNum == 3)?0x40:(antennaNum == 4)?0x60:0x01;
            }
            else {
                *code = (antennaNum == 2)?0x21:(antennaNum == 3)?0x41:(antennaNum == 4)?0x61:0x02;
            }
            break;
        case 2:
            if (support11ac == 0) {
                *code = (antennaNum == 2)?0x22:(antennaNum == 3)?0x42:(antennaNum == 4)?0x62:0x03;
            }
            else {
                *code = (antennaNum == 2)?0x23:(antennaNum == 3)?0x43:(antennaNum == 4)?0x63:0x04;
            }
            break;
        case 3:
            *code = (antennaNum == 2)?0x26:(antennaNum == 3)?0x46:(antennaNum == 4)?0x66:0x07;
            break;
        default:
            break;
        }
    }
    CWDebugLog("%s %d Code:[%d]", __FUNCTION__, __LINE__, *code);
    return CW_TRUE;
}

CWBool CWWTPBoardGetSerialNum(char **pstr)
{
    char *serialNum = NULL;

    //serialNum = CWCreateStringEntry(MAX_SERIALNUMBER_SIZE);
    //memset(serialNum, 0, sizeof(char)*MAX_SERIALNUMBER_SIZE);

    CW_CREATE_STRING_ERR(serialNum, MAX_SERIALNUMBER_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
/*
    if (sysutil_get_serial_number_info(serialNum, MAX_SERIALNUMBER_SIZE) < 0)
    {
        return CW_FALSE;
    }*/
    sprintf(serialNum,"N/A");
    *pstr = serialNum;
    CWDebugLog("%s %d serialNum:[%s]", __FUNCTION__, __LINE__, serialNum);

    return CW_TRUE;

}

CWBool CWWTPBoardGetLedPowerCfg(int *enable)
{
    int val=0;

    *enable = 1;

    if(IS_OUTDOOR_AP())
    {
        return CW_TRUE;
    }
#if SUPPORT_LED_MODULE_NAME
#ifdef LED_POWER
    if (api_get_led_option_by_sectionname(LED_POWER, &val))
#else
    if (api_get_integer_option(SYSTEM_POWER_LED_DEFAULT_OPTION, &val))
#endif
    {
        val = 1;
    }
#else
    if (api_get_integer_option(SYSTEM_LED2_DEFAULT_OPTION, &val))
    {
        val = 1;
    }
#endif
    *enable = !val;

    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLedPowerCfg(int enable)
{
    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, enable);

    if(IS_OUTDOOR_AP())
    {
        if(!enable)
        {
            CWDebugLog("LED control is not supported by outdoor AP");
        }

        return CW_TRUE;
    }
#if SUPPORT_LED_MODULE_NAME
#ifdef LED_POWER
    api_set_led_option_by_sectionname(LED_POWER, !enable);
#else
    api_set_integer_option(SYSTEM_POWER_LED_DEFAULT_OPTION, !enable);
#endif
#else
    api_set_integer_option(SYSTEM_LED2_DEFAULT_OPTION, !enable);
    api_set_string_option(SYSTEM_LED2_SYSFS_OPTION, "power", sizeof("power"));
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetLedLanCfg(int *enable)
{
    int val=0;

    *enable = 1;

    if(IS_OUTDOOR_AP())
    {
        return CW_TRUE;
    }

#if SUPPORT_LED_MODULE_NAME
#ifdef LED_LAN
    if(api_get_led_option_by_sectionname(LED_LAN, &val))
#else
    if(api_get_integer_option(SYSTEM_LAN1_LED_DEFAULT_OPTION, &val))
#endif
    {
        val = 1;
    }
#else
    if(api_get_integer_option(SYSTEM_LED1_DEFAULT_OPTION, &val))
    {
        val = 1;
    }
#endif

    *enable = !val;

    CWDebugLog("%s enable = %d", __FUNCTION__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLedLanCfg(int enable)
{
    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, enable);

    if(IS_OUTDOOR_AP())
    {
        if(!enable)
        {
            CWDebugLog("LED control is not supported by outdoor AP");
        }

        return CW_TRUE;
    }

#if SUPPORT_LED_MODULE_NAME
#ifdef LED_LAN
    api_set_led_option_by_sectionname(LED_LAN, !enable);
#else
    api_set_integer_option(SYSTEM_LAN1_LED_DEFAULT_OPTION, !enable);
    api_set_integer_option(SYSTEM_LAN2_LED_DEFAULT_OPTION, !enable);
#endif
#else
    api_set_integer_option(SYSTEM_LED1_DEFAULT_OPTION, !enable);
    api_set_string_option(SYSTEM_LED1_SYSFS_OPTION, "lan", sizeof("lan"));
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetLedWlan0Cfg(int *enable)
{
    int val=0;

    *enable = 1;

    if(IS_OUTDOOR_AP())
    {
        return CW_TRUE;
    }
#if SUPPORT_LED_MODULE_NAME
#ifdef LED_WIFI0
    if (api_get_led_option_by_sectionname(LED_WIFI0, &val))
#else
    if (api_get_integer_option(SYSTEM_WIFI0_LED_DEFAULT_OPTION, &val))
#endif
    {
        val = 1;
    }
#else
    if (api_get_integer_option(SYSTEM_LED3_DEFAULT_OPTION, &val))
    {
        val = 1;
    }
#endif

    *enable = !val;

    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLedWlan0Cfg(int enable)
{
    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, enable);

    if(IS_OUTDOOR_AP())
    {
        if(!enable)
        {
            CWDebugLog("LED control is not supported by outdoor AP");
        }

        return CW_TRUE;
    }

#if SUPPORT_LED_MODULE_NAME
#ifdef LED_WIFI0
    api_set_led_option_by_sectionname(LED_WIFI0, !enable);
#else
    api_set_integer_option(SYSTEM_WIFI0_LED_DEFAULT_OPTION, !enable);
#endif
#else
    api_set_integer_option(SYSTEM_LED3_DEFAULT_OPTION, !enable);
    api_set_string_option(SYSTEM_LED3_SYSFS_OPTION, "wlan-2g", sizeof("wlan-2g"));
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetLedWlan1Cfg(int *enable)
{
    int val=0;

    *enable = 0;

    if(CWWTPBoardGetMaxRadio() < 2)
    {
        return CW_TRUE;
    }

    if(IS_OUTDOOR_AP())
    {
        *enable = 1;

        return CW_TRUE;
    }

#if SUPPORT_LED_MODULE_NAME
#ifdef LED_WIFI1
    if (api_get_led_option_by_sectionname(LED_WIFI1, &val))
#else
    if (api_get_integer_option(SYSTEM_WIFI1_LED_DEFAULT_OPTION, &val))
#endif
    {
        val = 1;
    }
#else
    if (api_get_integer_option(SYSTEM_LED4_DEFAULT_OPTION, &val))
    {
        val = 1;
    }
#endif

    *enable = !val;

    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLedWlan1Cfg(int enable)
{
    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, enable);

    if(CWWTPBoardGetMaxRadio() < 2)
    {
        if(enable)
        {
            CWDebugLog("wlan1 LED control is not supported by single-band AP");
        }

        return CW_TRUE;
    }

    if(IS_OUTDOOR_AP())
    {
        if(!enable)
        {
            CWDebugLog("LED control is not supported by outdoor AP");
        }

        return CW_TRUE;
    }

#if SUPPORT_LED_MODULE_NAME
#ifdef LED_WIFI1
    api_set_led_option_by_sectionname(LED_WIFI1, !enable);
#else
    api_set_integer_option(SYSTEM_WIFI1_LED_DEFAULT_OPTION, !enable);
#endif
#else
    api_set_integer_option(SYSTEM_LED4_DEFAULT_OPTION, !enable);
    api_set_string_option(SYSTEM_LED4_SYSFS_OPTION, "wlan-5g", sizeof("wlan-5g"));
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetLedWlan2Cfg(int *enable)
{
    int val=0;

    *enable = 0;

    if(CWWTPBoardGetMaxRadio() < 3)
    {
        return CW_TRUE;
    }

    if(IS_OUTDOOR_AP())
    {
        *enable = 1;

        return CW_TRUE;
    }

#if SUPPORT_LED_MODULE_NAME
#ifdef LED_WIFI2
    if (api_get_led_option_by_sectionname(LED_WIFI2, &val))
#else
    if (api_get_integer_option(SYSTEM_WIFI2_LED_DEFAULT_OPTION, &val))
#endif
    {
        val = 1;
    }
#else
    if (api_get_integer_option(SYSTEM_LED4_DEFAULT_OPTION, &val))
    {
        val = 1;
    }
#endif
    *enable = !val;

    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLedWlan2Cfg(int enable)
{
    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, enable);

    if(CWWTPBoardGetMaxRadio() < 3)
    {
        if(enable)
        {
            CWDebugLog("wlan2 LED control is supported by tri-band AP");
        }

        return CW_TRUE;
    }

    if(IS_OUTDOOR_AP())
    {
        if(!enable)
        {
            CWDebugLog("LED control is not supported by outdoor AP");
        }

        return CW_TRUE;
    }

#if SUPPORT_LED_MODULE_NAME
#ifdef LED_WIFI2
    api_set_led_option_by_sectionname(LED_WIFI2, !enable);
#else
    api_set_integer_option(SYSTEM_WIFI2_LED_DEFAULT_OPTION, !enable);
#endif
#else
    api_set_integer_option(SYSTEM_LED4_DEFAULT_OPTION, !enable);
    api_set_string_option(SYSTEM_LED4_SYSFS_OPTION, "wlan-5g", sizeof("wlan-5g"));
#endif

    return CW_TRUE;
}

//NOT_FINISH
CWBool CWWTPBoardGetLedMeshCfg(int *enable)
{
#if 0
    char *val;

    if(!(val = CWCreateStringByUci("led.led.WLAN1LED")))
    {
        return CW_FALSE;
    }

    *enable = atoi(val);
    CWLog("%s %d", __FUNCTION__, *enable);

    CW_FREE_OBJECT(val);
#endif
    *enable = 0;
    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}
//NOT_FINISH
CWBool CWWTPBoardSetLedMeshCfg(int enable)
{
    CWDebugLog("%s %d enable = %d", __FUNCTION__, __LINE__, enable);
#if 0
    CWDebugLog("%s %d", __FUNCTION__, enable);

    CWSystem("uci set led.led.WLAN1LED=%d", enable);
    CWLog("%s %d", __FUNCTION__, enable);
#endif

    return CW_TRUE;
}
