#include "WTPBoardApiMesh.h"
#include "WTPBoardApiCommon.h"
#include <ctype.h>
#include <variable/api_wireless.h>
#include <variable/api_mesh.h>
CWBool CWWTPBoardSetRadioMeshEnableCfg(int radioIdx,  int enable)
{
    int value = 0;
    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), enable);
    value = !enable;
#if 0 // SUPPORT_WLAN_5G_2_SETTING
    if(radioIdx == 1)
    {
        return CW_TRUE;
    }
#endif   
    if(uci_set_wifix_mesh_disabled_option(CWConvertRadioIdx(radioIdx), value) != API_RC_SUCCESS)
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioMeshEnableCfg(int radioIdx, int *enable)
{
    int value = 0;
    *enable = CW_FALSE;

#if SUPPORT_MESH_SETTING
    if(uci_get_wifix_mesh_disabled_option(CWConvertRadioIdx(radioIdx), &value) != API_RC_SUCCESS)
    {
        // return CW_FALSE;
        *enable = CW_FALSE;
    }
    else
    {
        *enable = !value;
    }
#else
    *enable = CW_FALSE;
#endif

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}


CWBool CWWTPBoardGetRadioMeshIDCfg(int radioIdx,  char **pstr)
{
    char *val = NULL;
#if SUPPORT_MESH_SETTING
    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_MESHID_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(uci_get_wifix_mesh_id_option(CWConvertRadioIdx(radioIdx) , val, MAX_MESHID_SIZE+1) != API_RC_SUCCESS)
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        CW_FREE_OBJECT(val);
    }
    else
    {
        *pstr = val;
    }
#else
    CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "12345678", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
#endif
    CWDebugLog("%s %d radio:[%d]%s", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx),  *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioMeshIDCfg(int radioIdx,  char *pstr)
{
    char *pUciStr = NULL;

    CWDebugLog("%s %d  %s", __FUNCTION__, CWConvertRadioIdx(radioIdx), pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(uci_set_wifix_mesh_id_option(CWConvertRadioIdx(radioIdx), pUciStr, strlen(pUciStr)) != API_RC_SUCCESS)
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(pUciStr);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioMeshWPAKeyCfg(int radioIdx,  char **pstr)
{
    char *val = NULL;
#if SUPPORT_MESH_SETTING
    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_MASH_WPAKEY_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(uci_get_wifix_mesh_wpa_key_option(CWConvertRadioIdx(radioIdx) , val, MAX_MASH_WPAKEY_SIZE+1) != API_RC_SUCCESS)
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        CW_FREE_OBJECT(val);
    }
    else
    {
        *pstr = val;
    }
#else
    CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "1234567890", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
#endif
    CWDebugLog("%s %d radio:[%d]%s", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx),  *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioMeshWPAKeyCfg(int radioIdx,  char *pstr)
{
    char *pUciStr = NULL;

    CWDebugLog("%s %d  %s", __FUNCTION__, CWConvertRadioIdx(radioIdx), pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(uci_set_wifix_mesh_wpa_key_option(CWConvertRadioIdx(radioIdx), pUciStr, strlen(pUciStr)) != API_RC_SUCCESS)
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(pUciStr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioMeshLinkRobustThresholdCfg(int radioIdx,  short link_robust_threshold)
{
    CWDebugLog("%s %d %d ", __FUNCTION__, CWConvertRadioIdx(radioIdx), link_robust_threshold);

    if(uci_set_wifix_mesh_link_robust_threshold_option(CWConvertRadioIdx(radioIdx), (int)link_robust_threshold) != API_RC_SUCCESS)
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioMeshLinkRobustThresholdCfg(int radioIdx, short *link_robust_threshold)
{
    int value = 0;
#if SUPPORT_MESH_SETTING
    if(uci_get_wifix_mesh_link_robust_threshold_option(CWConvertRadioIdx(radioIdx), &value) != API_RC_SUCCESS)
    {
        *link_robust_threshold = 0;
    }
    else
    {
        *link_robust_threshold = (short)value;
    }
#else
    *link_robust_threshold = 0;
#endif

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *link_robust_threshold);

    return CW_TRUE;
}

/////////////////////////////////////////////////////////////////////////

CWBool CWWTPBoardSetMeshEnableTotalCfg(int enable)
{
    int value = 0;
    CWDebugLog("%s %d", __FUNCTION__, enable);

    value = !enable;
    if(uci_set_mesh_disabled_option(value) != API_RC_SUCCESS)
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetMeshEnableTotalCfg(int *enable)
{
    int value = 0;
    *enable = CW_FALSE;

#if SUPPORT_MESH_SETTING
    if(uci_get_mesh_disabled_option(&value) != API_RC_SUCCESS)
    {
        *enable = CW_FALSE;
        //return CW_FALSE;
    }
    else
    {
        *enable = !value;
    }
#else
    *enable = CW_FALSE;
#endif

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioMeshInfo(int radioIdx, CWRadioMeshInfo *meshInfo)
{
    int value = 0;
    char macStr[32];
    int mesh_disabled = CW_TRUE, wifi_disabled = CW_TRUE;

#if SUPPORT_MESH_SETTING
    uci_get_mesh_disabled_option(&mesh_disabled);
    uci_get_wifix_mesh_disabled_option(CWConvertRadioIdx(radioIdx), &wifi_disabled);

    if(wifi_disabled == CW_FALSE && mesh_disabled == CW_FALSE)
    {
        if(uci_get_wifix_mesh_current_role_option(CWConvertRadioIdx(radioIdx), &value) != API_RC_SUCCESS)
        {
            CWDebugLog("Get configuration fail");

        }
        meshInfo->role = value;
        if(uci_get_wifix_mesh_mac_option(CWConvertRadioIdx(radioIdx), macStr, 32) != API_RC_SUCCESS)
        {
            CWDebugLog("Get configuration fail");
        }
        sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &(meshInfo->bssid[0]), &(meshInfo->bssid[1]), &(meshInfo->bssid[2]), &(meshInfo->bssid[3]), &(meshInfo->bssid[4]), &(meshInfo->bssid[5]));
    }
    else //mesh disable
    {
        meshInfo->role = MESH_UNKNOWN;
        sscanf("00:00:00:00:00:00", "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &(meshInfo->bssid[0]), &(meshInfo->bssid[1]), &(meshInfo->bssid[2]), &(meshInfo->bssid[3]), &(meshInfo->bssid[4]), &(meshInfo->bssid[5]));
    }
#else
    meshInfo->role = value;
    sscanf("00:00:00:00:00:00", "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &(meshInfo->bssid[0]), &(meshInfo->bssid[1]), &(meshInfo->bssid[2]), &(meshInfo->bssid[3]), &(meshInfo->bssid[4]), &(meshInfo->bssid[5]));
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardSetMeshModeCfg(int mode)
{
    CWDebugLog("%s %d", __FUNCTION__, mode);

    if(uci_set_mesh_mode_option(mode) != API_RC_SUCCESS)
    {
        CWDebugLog("Set configuration fail");
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetMeshModeCfg(int *mode)
{
    *mode = 0;


    if(uci_get_mesh_mode_option(mode) != API_RC_SUCCESS)
    {
        CWDebugLog("Get configuration fail");
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *mode);

    return CW_TRUE;
}

