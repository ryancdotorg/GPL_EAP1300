#ifndef _OPMODES_H
#define _OPMODES_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************/
/*****       Define 0-7 for eight kind of Operation Mode      *****/
/******************************************************************/

typedef enum {
SYS_OPM_AP=0,
SYS_OPM_CB,
SYS_OPM_WDSAP,
SYS_OPM_WDSB,
SYS_OPM_WDSSTA,
SYS_OPM_AR,
SYS_OPM_CBRT,
SYS_OPM_RP,
//Add two new operation modes on 2013-12-18 which is asked by John for all models supporting Mesh
SYS_OPM_MESH_AP,
SYS_OPM_MESH_ONLY,
/* Count the mode */
SYS_OPM_ARRP,
/*DUAL AP+APRP*/
SYS_OPM_AP_APRP,
SYS_OPM_COUNT,
} SYSTEM_OP_MODE;

typedef enum {
RADIO_OPM_AP=0,
RADIO_OPM_CB,
RADIO_OPM_CR,
RADIO_OPM_WDSB,
RADIO_OPM_WDSR,
RADIO_OPM_REPEATER_AP,
RADIO_OPM_REPEATER_STA,
RADIO_OPM_U_REPEATER,/*universal repeater*/
RADIO_OPM_DISABLE
}RADIO_OP_MODE;

#define HAS_VENDOR_DEFINE_DEFAULT_IP
#define DEFAULT_MULTI_OP_AP_CLASS_IP	"192.168.1.1"
#define DEFAULT_MULTI_OP_CB_CLASS_IP	"192.168.1.2"
#define DEFAULT_MULTI_OP_APRP_CLASS_IP	"192.168.1.2"
#define DEFAULT_MULTI_OP_AR_CLASS_IP	DEFAULT_MULTI_OP_AP_CLASS_IP

#define MODEL_TEST		0

/******************************************************************/
/*****       Define eight kind of Operation Modes             *****/
/******************************************************************/

#define OPM_AP              (1<<SYS_OPM_AP)   /* AP mode */

//Add two new operation modes on 2013-12-18 which is asked by John for all models supporting Mesh
#define OPM_MESH_AP         (1<<SYS_OPM_MESH_AP)   /* Mesh AP mode */
#define OPM_MESH_ONLY       (1<<SYS_OPM_MESH_ONLY)   /* Mesh Only mode */

#define OPM_CB              (1<<SYS_OPM_CB)   /* Client Bridge mode */
#define OPM_WDSAP           (1<<SYS_OPM_WDSAP) /*WDS AP Mode*/
#define OPM_WDSB            (1<<SYS_OPM_WDSB) /* WDS Bridge mode NO ROUTER FUNCTION*/
#define OPM_WDSSTA          (1<<SYS_OPM_WDSSTA)/*WDS STA mode */
#define OPM_AR              (1<<SYS_OPM_AR)   /* AP Router mode */
#define OPM_CBRT            (1<<SYS_OPM_CBRT) /* Client Router mode */
#define OPM_RP              (1<<SYS_OPM_RP) /* Repeater mode */
#define OPM_ARRP            (1<<SYS_OPM_ARRP) /* AP Router Repeater mode */
#define OPM_AP_APRP         (1<<SYS_OPM_AP_APRP) /*AP + Repeater(AP+CB)*/
#define OPM_ALL             (OPM_WDSAP|OPM_WDSSTA|OPM_RP|OPM_AP|OPM_AR|OPM_CB|OPM_CBRT|OPM_WDSB)

/*********Service Operation Mode*******/
#define WAN_OPM         (OPM_AR|OPM_CBRT)
#define DHCPD_OPM       (OPM_AR|OPM_CBRT|OPM_AP)
#define DNSPROXY_OPM    (OPM_AR|OPM_CBRT)
#define NTPD_OPM        (OPM_ALL) /* (OPM_AR|OPM_CBRT|OPM_RP) */
#define DDNS_OPM        (OPM_AR|OPM_CBRT)
#define HTTPPTOXY_OPM   (OPM_AR|OPM_CBRT)
#define UPNPD_OPM       (OPM_AP|OPM_AR|OPM_CBRT)
#define ALG_OPM         (OPM_AR|OPM_CBRT)
#define IGMP_OPM        (OPM_AR|OPM_CBRT)

/******************************************************************/
/*****             Define Model's Operation Mode              *****/
/******************************************************************/
//models operation mode
#define MODEL_OPM	(OPM_AP|OPM_WDSB|OPM_WDSAP|OPM_RP)
/******************************************************************/
/*****            Define the group of Model's ID              *****/
/******************************************************************/

//define MODEL_SINGLE for all single-radio models
#define MODEL_SINGLE			0
//define MODEL_DUAL for all dual-radio models
#define MODEL_DUAL			1
//define MODEL_ALL for all models
#define MODEL_ALL			1
//define MODEL_MGMT models
#define MODEL_MGMT			1
//Modify on 2013-10-18 to define old WEB for the following models
#define MODEL_OLD_WEB			0
//Modify on 2013-08-06 to define new WEB for the following models
#define MODEL_NEW_WEB			1
//define MODEL_2G for all models with 2.4G radio
#define MODEL_2G			1
//define MODEL_5G for all models with 5G radio
#define MODEL_5G			1
//define MODEL_EAP_ECB for all EAP and ECB models
#define MODEL_EAP_ECB			1
//define MODEL_ENH for all ENH models
#define MODEL_ENH			0
//define MODEL_TRUE for TRUE WIFI
#define MODEL_TRUE			0

/******************************************************************/
/*****      Define Model's Group for Function Alignment       *****/
/******************************************************************/

//Add on 2012-12-12 by Philips to define MODEL_IPV6 for these models support IPV6
#define MODEL_IPV6			1
//Add on 2012-12-12 by Philips to define MODEL_RATIO2 for all 2.4G models which support load balance
#define MODEL_RATIO2			0
//Add on 2012-12-12 by Philips to define MODEL_RATIO5 for all 5G models which support load balance
#define MODEL_RATIO5			0
//Add on 2012-12-12 by Philips to define MODEL_CLIMIT for all models except EAP600TRUE and ECB350TRUE which support client limit
#define MODEL_CLIMIT			1
//Add on 2013-01-09 by Philips to define MODEL_WIFISCH for old models except EAP350 and ECB350 which support wifi schedule
#define MODEL_WIFISCH			0
//Add on 2013-03-04 by Philips to define MODEL_WIFISCH_CRONDJOB for new models which support wifi schedule
#define MODEL_WIFISCH_CRONDJOB		0
//Modify for C600 on 2013-05-02 to disable traffic shaping
#define MODEL_DUAL_TRAFFIC		0
//Modify on 2013-06-26 to enable Traffic control per SSID function only for the following models
#define MODEL_NEWTC			1
//Modify on 2013-07-02 to enable band steer function only for outdoor models
#define MODEL_DUAL_BANDSTEER		1
//Modify on 2013-06-26 to enable distance function only for outdoor models
//define MODEL_OUTDOOR for outdoor models
#define MODEL_OUTDOOR			0
//Add on 2013-02-23 by Philips to define MODEL_INDOOR which will skip Compliance Band(21) and Compliance Band 2(22)
#define MODEL_INDOOR	                1
//Modify on 2013-06-26 to enable HTTPS function only for the following models
#define MODEL_HTTPS			1
//Modify on 2013-06-26 to enable SSH function only for the following models
#define MODEL_SSH			1
//Modify on 2013-06-26 to enable Email Alert function only for the following models
#define MODEL_EMAIL			1
//Modify on 2013-12-11 to Add into the model list of Guest Network function
#define MODEL_GUEST			1
//Modify on 2013-12-11 to enable Fast Handover function only for the following models which is requested by Shpin
#define MODEL_FAST_HANDOVER		1
//Modify for C600 on 2013-05-02 to disable MAC filter
#define MODEL_ALL_MACFILTER		0
//Add NEW_WEB model on 2013-12-26 which is asked by Jackey to support NEW MAC filter for ENH900EXT
#define MODEL_NEW_MACFILTER		1
//Add on 2013-10-18 by Philips to define MODEL_GREEN for models to support SKU control
#define MODEL_GREEN			1
//Add on 2013-11-20 by Philips to define MODEL_OLD_WEB_GREEN for EnGenius Old models to support SKU control
#define MODEL_OLD_WEB_GREEN		0
//Add on 2013-11-20 by Philips to define MODEL_OLD_WEB_GREEN for models to support SKU control
#define MODEL_OBEY_REG_POWER		0
//Add on 2013-11-01 by Po-Yao todefine MODEL_AUTOREBOOT_SINGLE for all single-radio models to execute autoreboot command
#define MODEL_AUTOREBOOT_SINGLE		0
//Add two new model on 2013-11-04 which is asked by Po-Yao to support LED
#define MODEL_2LED_5LED			1
//Add on 2013-12-04 which by Philips to define MODEL_WPSLED for models to support WPS LED
#define MODEL_WPSLED			0
//Add on 2013-07-24 by Philips to define MODEL_AC for models to support 802.11 AC
#define MODEL_AC			1
//Add on 2013-12-18 by Philips to define MODEL_MESH for models to support MESH function
#define MODEL_MESH		        0
//Add on 2013-12-19 by Philips define MODEL_WPS for all models supporting WPS
#define MODEL_WPS		        1
//Add on 2013-12-23 by Philips to define MODEL_10.1.478 for models to support atheros driver 10.1.478
#define MODEL_10_1_478			1
//Add on 2014-01-08 by Philips to define MODEL_FAST_ROAMING for models to support FAST ROAMING function
#define MODEL_FAST_ROAMING		1
//Add on 2016-05-03 by Caridee to define MODEL_FAST_ROAMING_SUPPORT_ADVSCH for models to support FAST ROAMING advsch function
#define MODEL_FAST_ROAMING_SUPPORT_ADVSCH		0
//Add on 2014-01-08 by Philips to define MODEL_SNMP_PORT for models to support SNMP PORT function
#define MODEL_SNMP_PORT			1
//Add on 2015-09-04 by Tim to define MODEL_MULTICAST2UNICAST for models to support multicast to unicast function
#define MODEL_MULTICAST2UNICAST         1
//Add on 2014-12-01 by TIm to define MODEL_EXTCHANNEL for models to support extension channel function
#define MODEL_EXTCHANNEL                1

/**************************************/

#ifdef __cplusplus
}
#endif

#endif /* _REGX_H */
