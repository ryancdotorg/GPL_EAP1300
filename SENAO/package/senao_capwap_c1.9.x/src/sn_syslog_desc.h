#ifndef __SN_COMMON_SYS_LOG_DESC_H__
#define __SN_COMMON_SYS_LOG_DESC_H__

/******LOG Severity*******************************/
typedef enum sn_log_severity_e
{
    SN_LOG_SEV_EMERG = 0,  /* system is unusable */
    SN_LOG_SEV_ALERT,      /* action must be taken immediately */
    SN_LOG_SEV_CRIT,       /* critical conditions */
    SN_LOG_SEV_ERR,        /* error conditions */
    SN_LOG_SEV_WARNING,    /* warning conditions */
    SN_LOG_SEV_NOTICE,     /* normal but significant condition */
    SN_LOG_SEV_INFO,       /* informational */
    SN_LOG_SEV_DEBUG,      /* debug-level messages */
    SN_LOG_SEV_ALL,        /* all level */
} sn_log_severity_t;

/******LOG Category******************************/
#define SN_LOG_CAT_AP           "AP"
#define SN_LOG_CAT_AC           "AC"
#define SN_LOG_CAT_SWITCH       "Switch"
#define SN_LOG_CAT_WIFI_CLIENT  "Client"

/******Event Group List***************************/
typedef enum sn_event_group_e
{
    SN_EG_SYS_STATE = 1,
    SN_EG_CERT,
    SN_EG_DEV_MNG,
    SN_EG_DEV_STATE,
    SN_EG_DEV_CFG,
    SN_EG_DEV_FW,
    SN_EG_AP_CLIENT
} sn_event_group_t;

typedef struct log_text_severity_s
{
    char text[64];
} log_text_severity_t;

/*Action list*/
enum event_action
{
    MAIL_ACTION = 0,
    SMS_ACTION,
    LINE_ACTION,
    SKYPE_ACTION,
    LOG_ACTION
};

/******SHORTER MARCO******************************/
#define SN_MAC_PRINT                        "(%02X:%02X:%02X:%02X:%02X:%02X)"
#define SN_NAME_MAC_PRINT                   "%s(%02X:%02X:%02X:%02X:%02X:%02X)"
#define SN_NAME_MAC_IP_PRINT                "%s(%02X:%02X:%02X:%02X:%02X:%02X%s)"

/******LOG Description****************************/
/*group system*/
#define SN_SYS_STATE_AC_ENABLED             "Controller is enabled"
#define SN_SYS_STATE_AC_DISABLED            "Controller is disabled"
#define SN_SYS_STATE_AC_INIT_FAIL           "Controller init failed"
#define SN_SYS_STATE_AC_MEM_USAGE           "System memory usage reaches %u%%"
#define SN_SYS_SCHEDULE_TASK_START          "Scheduled task \"%s\" started"

/*group CERT*/
#define SN_CERT_AC_UPDATE                   "SSL certificate updated"
#define SN_CERT_AC_EXPIRE_XDAY              "SSL certificate will expire in %u days"
#define SN_CERT_AC_EXPIRE                   "SSL certificate has expired"
#define SN_CERT_DEV_UPDATE_MAC              SN_NAME_MAC_PRINT"'s SSL certificate has been updated"

/*DEV_MNG*/
#define SN_DEV_MNG_DEV_ADD_MANA             SN_NAME_MAC_PRINT" added to management list"
#define SN_DEV_MNG_DEV_RM_MANA              SN_NAME_MAC_PRINT" removed from management list"

/*DEV_STATE*/
#define SN_DEV_STATE_DEV_ONLINE             SN_NAME_MAC_PRINT" online"
#define SN_DEV_STATE_DEV_ONLINE_UPTIME      SN_NAME_MAC_PRINT" online with uptime %u seconds"
#define SN_DEV_STATE_DEV_RESET              SN_NAME_MAC_PRINT" reset"
#define SN_DEV_STATE_DEV_RECONNECT          SN_NAME_MAC_PRINT" reconnect"
#define SN_DEV_STATE_DEV_OFFLINE            SN_NAME_MAC_PRINT" offline"
#define SN_DEV_STATE_DEV_TRANS_OFFLINE      SN_NAME_MAC_PRINT" offline (continuous retransmission timeout)"
#define SN_DEV_STATE_DEV_REBOOT             SN_NAME_MAC_PRINT" reboot"
#define SN_DEV_STATE_DEV_INVALID_IP         SN_NAME_MAC_PRINT" has invalid IP(%s)"
#define SN_DEV_STATE_DEV_MAX_CLIENT         SN_NAME_MAC_PRINT"'s active client number reaches client limits %d of %sHz"
#define SN_DEV_STATE_DEV_CONNECT_AC         SN_NAME_MAC_PRINT" connected to controller %u.%u.%u.%u"
#define SN_DEV_LOG_MSG                      SN_NAME_MAC_PRINT": %s"

/*CFG*/
#define SN_DEV_CFG_CHG_CONF                 SN_NAME_MAC_PRINT" configuration updated"
#define SN_DEV_CFG_CHG_CONF_FAIL            SN_NAME_MAC_PRINT" configuration upadte failed"
#define SN_AP_HEALING_CHANGE_TXPWOER        SN_NAME_MAC_PRINT" %s %s transmit power to %d%%"
#define SN_AP_AUTOCHANNEL_CHANNELCHANGE     SN_NAME_MAC_PRINT" %sG channel changed from %d to %d"

/*DEV_FW*/
#define SN_DEV_FW_DEV_FW_INCOMP             SN_NAME_MAC_PRINT" firmware version is incompatible"
#define SN_DEV_FW_DEV_FW_UPGRADE            SN_NAME_MAC_PRINT" started to upgrade firmware from %s to %s"
#define SN_DEV_FW_DEV_FW_UPGRADE_FAIL       SN_NAME_MAC_PRINT" firmware upgrade failed"
#define SN_DEV_FW_DEV_FW_TRANS_FAIL         SN_NAME_MAC_PRINT" firmware upgrade failed (continuous retransmission error)"

/*AP_CLIENT*/
#define SN_AP_CLIENT_STATE_CHANGE           SN_NAME_MAC_IP_PRINT" %s WLAN(%s) from %s"SN_MAC_PRINT

#endif
