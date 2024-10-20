/************************************************************************************************
 * Copyright (c) 2006-2009 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica	*
 *                          Universita' Campus BioMedico - Italy								*
 *																								*
 * This program is free software; you can redistribute it and/or modify it under the terms		*
 * of the GNU General Public License as published by the Free Software Foundation; either		*
 * version 2 of the License, or (at your option) any later version.								*
 *																								*
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY				*
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A				*
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.						*
 *																								*
 * You should have received a copy of the GNU General Public License along with this			*
 * program; if not, write to the:																*
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,							*
 * MA  02111-1307, USA.																			*
 *																								*
 * -------------------------------------------------------------------------------------------- *
 * Project:  Capwap																				*
 *																								*
 * Authors : Ludovico Rossi (ludo@bluepixysw.com)												*
 *           Del Moro Andrea (andrea_delmoro@libero.it)											*
 *           Giovannini Federica (giovannini.federica@gmail.com)								*
 *           Massimo Vellucci (m.vellucci@unicampus.it)											*
 *           Mauro Bisson (mauro.bis@gmail.com)													*
 *	         Antonio Davoli (antonio.davoli@gmail.com)											*
 ************************************************************************************************/


#ifndef __CAPWAP_CWProtocol_HEADER__
#define __CAPWAP_CWProtocol_HEADER__

//#define CWSetField32(obj, start, val)	((obj)[start/64]) |= ((val) << (start%64))
//#define CWGetField32(obj, start, len)	(((obj)[start/64]) & ((0xFFFFFFFFFFFFFFFF >> (64-(len))) << (start%64)) ) >> (start%64)

/*_____________________________________________________*/
/*  *******************___MACRO___*******************  */
//#define CWSetField32(obj, start, val)					((obj)[start/32]) |= ((val) << (start%32))
//#define CWGetField32(obj, start, len)					(((obj)[start/32]) & ((0xFFFFFFFFUL >> (32-(len))) << (start%32)) ) >> (start%32)

#define CWSetField32(src,start,len,val)					src |= ((~(0xFFFFFFFF << len)) & val) << (32 - start - len)
#define CWGetField32(src,start,len)					((~(0xFFFFFFFF<<len)) & (src >> (32 - start - len)))

#define CW_REWIND_BYTES(buf, bytes, type)				(buf) = (type*)(((char*) (buf)) - bytes)
#define CW_SKIP_BYTES(buf, bytes, type)					(buf) = (type*)(((char*) (buf)) + bytes)
#define CW_SKIP_BITS(buf, bits, type)					(buf) = (type*)(((char*) (buf)) + ((bits) / 8))
#define CW_BYTES_TO_STORE_BITS(bits)					((((bits) % 8) == 0) ? ((bits) / 8) : (((bits) / 8)+1))

#define		CW_CREATE_PROTOCOL_MESSAGE(mess, size, err)		do{CW_CREATE_OBJECT_SIZE_ERR(((mess).msg), (size), err);		\
									CW_ZERO_MEMORY(((mess).msg), (size));						\
									(mess).offset = 0;}while(0)

#define 	CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(ar_name, ar_size, on_err) 	do{\
											CW_CREATE_ARRAY_ERR(ar_name, ar_size, CWProtocolMessage, on_err)\
											int _i;\
											for(_i=0;_i<(ar_size); _i++) {\
												(ar_name)[_i].msg = NULL;\
												(ar_name)[_i].offset = 0; \
											}\
										}while(0)

#define		CW_FREE_PROTOCOL_MESSAGE(mess)				do{CW_FREE_OBJECT(((mess).msg));								\
									(mess).offset = 0;}while(0)

#define		CW_PARSE_MSG_ELEMMENT_START()				int oldOffset;												\
									if(msgPtr == NULL || valPtr == NULL) return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);	\
									oldOffset = msgPtr->offset

#define		CW_PARSE_MSG_ELEMENT_END()				CWDebugLog(NULL);											\
									return ((msgPtr->offset) - oldOffset) == len ? CW_TRUE :	\
									CWErrorRaise(CW_ERROR_INVALID_FORMAT, "Message Element Malformed");


/*_________________________________________________________*/
/*  *******************___CONSTANTS___*******************  */

// to be defined
#define     MAX_UDP_PACKET_SIZE					65536
#define     CW_DEF_MULTICAST_ADDRESS            "239.255.1.3"
#define		CW_CONTROL_PORT						1234
#define		CW_PROTOCOL_VERSION					CW_SINGLE_PACKET//0: for single packe 1:multi-packet
#define		CW_IANA_ENTERPRISE_NUMBER				13277
#define 	CW_CONTROL_HEADER_OFFSET_FOR_MSG_ELEMS			3		//Offset "Seq Num" - "Message Elements"
#define		CW_MAX_SEQ_NUM						255
#define 	CW_MAX_FRAGMENT_ID					65535
#define 	CLEAR_DATA                          1
#define		DTLS_ENABLED_DATA					2
/* Header Type */
#define		CW_PACKET_HEADER_TYPE_MASK          15
#define		CW_PACKET_PLAIN						0
#define		CW_PACKET_CRYPT						1
#define		CW_PACKET_CID_TAG                   2
#define		CW_CID_HEADER_SIZE                  4
#define		CW_DTLS_HEADER_SIZE                 4

#define 	CW_DATA_MSG_FRAME_TYPE					1
#define		CW_DATA_MSG_STATS_TYPE					2
#define     CW_DATA_MSG_FREQ_STATS_TYPE             3 /* 2009 Update */
#define     CW_PMTU_DEFAULT						1468
// <TRANSPORT_HEADER_FIELDS>
// CAPWAP version (currently 0)
#define 	CW_TRANSPORT_HEADER_VERSION_START			0
#define 	CW_TRANSPORT_HEADER_VERSION_LEN				4

// Mauro
#define		CW_TRANSPORT_HEADER_TYPE_START				4
#define		CW_TRANSPORT_HEADER_TYPE_LEN				4

// Radio ID number (for WTPs with multiple radios)
#define 	CW_TRANSPORT_HEADER_RID_START				13
#define 	CW_TRANSPORT_HEADER_RID_LEN				5

// Length of CAPWAP tunnel header in 4 byte words
#define 	CW_TRANSPORT_HEADER_HLEN_START				8
#define 	CW_TRANSPORT_HEADER_HLEN_LEN				5

// Wireless Binding ID
#define 	CW_TRANSPORT_HEADER_WBID_START				18
#define 	CW_TRANSPORT_HEADER_WBID_LEN				5

// Format of the frame
#define 	CW_TRANSPORT_HEADER_T_START				23
#define 	CW_TRANSPORT_HEADER_T_LEN				1

// Is a fragment?
#define 	CW_TRANSPORT_HEADER_F_START				24
#define 	CW_TRANSPORT_HEADER_F_LEN				1

// Is NOT the last fragment?
#define 	CW_TRANSPORT_HEADER_L_START				25
#define 	CW_TRANSPORT_HEADER_L_LEN				1

// Is the Wireless optional header present?
#define 	CW_TRANSPORT_HEADER_W_START				26
#define 	CW_TRANSPORT_HEADER_W_LEN				1

// Is the Radio MAC Address optional field present?
#define 	CW_TRANSPORT_HEADER_M_START				27
#define 	CW_TRANSPORT_HEADER_M_LEN				1

// Is the message a keep alive?
#define 	CW_TRANSPORT_HEADER_K_START				28
#define 	CW_TRANSPORT_HEADER_K_LEN				1

// Is the last Fragment of packet, 
#define 	CW_TRANSPORT_HEADER_LAST_FRAGMENT_START				29
#define 	CW_TRANSPORT_HEADER_LAST_FRAGMENT_LEN				1

//Is the first Fragment of packet
#define 	CW_TRANSPORT_HEADER_FIRST_FRAGMENT_START				30
#define 	CW_TRANSPORT_HEADER_FIRST_FRAGMENT_LEN				1


// Set to 0 in this version of the protocol
#define 	CW_TRANSPORT_HEADER_FLAGS_START				31
#define 	CW_TRANSPORT_HEADER_FLAGS_LEN				1

// ID of the group of fragments
#define 	CW_TRANSPORT_HEADER_FRAGMENT_ID_START			0
#define 	CW_TRANSPORT_HEADER_FRAGMENT_ID_LEN			16

// Position of this fragment in the group
#define 	CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_START		16
#define 	CW_TRANSPORT_HEADER_FRAGMENT_OFFSET_LEN			13

// Set to 0 in this version of the protocol
#define 	CW_TRANSPORT_HEADER_RESERVED_START			29
#define 	CW_TRANSPORT_HEADER_RESERVED_LEN			3

//Next Fragment
#define 	CW_TRANSPORT_HEADER_NEXT_FRAGMENT_ID_START			0
#define 	CW_TRANSPORT_HEADER_NEXT_FRAGMENT_ID_LEN			16


// </TRANSPORT_HEADER_FIELDS>

// Position of this fragment in the group
#define 	CW_CONTROL_HEADER_ELEMENT_LEN_OFFSET		(13)

// Message Type Values
#define		CW_MSG_TYPE_VALUE_UNUSED			        0
#define		CW_MSG_TYPE_VALUE_DISCOVERY_REQUEST			1
#define		CW_MSG_TYPE_VALUE_DISCOVERY_RESPONSE			2
#define		CW_MSG_TYPE_VALUE_JOIN_REQUEST				3
#define		CW_MSG_TYPE_VALUE_JOIN_RESPONSE				4
#define		CW_MSG_TYPE_VALUE_CONFIGURE_REQUEST			5
#define		CW_MSG_TYPE_VALUE_CONFIGURE_RESPONSE			6
#define		CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_REQUEST		7
#define		CW_MSG_TYPE_VALUE_CONFIGURE_UPDATE_RESPONSE		8
#define		CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST			9
#define		CW_MSG_TYPE_VALUE_WTP_EVENT_RESPONSE			10
#define		CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_REQUEST		11
#define		CW_MSG_TYPE_VALUE_CHANGE_STATE_EVENT_RESPONSE		12
#define		CW_MSG_TYPE_VALUE_ECHO_REQUEST				13
#define		CW_MSG_TYPE_VALUE_ECHO_RESPONSE				14
#define		CW_MSG_TYPE_VALUE_IMAGE_DATA_REQUEST			15
#define		CW_MSG_TYPE_VALUE_IMAGE_DATA_RESPONSE			16
#define		CW_MSG_TYPE_VALUE_RESET_REQUEST				17
#define		CW_MSG_TYPE_VALUE_RESET_RESPONSE			18
#define		CW_MSG_TYPE_VALUE_PRIMARY_DISCOVERY_REQUEST		19
#define		CW_MSG_TYPE_VALUE_PRIMARY_DISCOVERY_RESPONSE		20
#define		CW_MSG_TYPE_VALUE_DATA_TRANSFER_REQUEST			21
#define		CW_MSG_TYPE_VALUE_DATA_TRANSFER_RESPONSE		22
#define		CW_MSG_TYPE_VALUE_CLEAR_CONFIGURATION_REQUEST		23
#define		CW_MSG_TYPE_VALUE_CLEAR_CONFIGURATION_RESPONSE		24
#define		CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_REQUEST		25
#define		CW_MSG_TYPE_VALUE_STATION_CONFIGURATION_RESPONSE	26
/* Message Type for SENAO extension */
#define		CW_MSG_TYPE_VALUE_CERT_RESET_REQUEST		    27
#define		CW_MSG_TYPE_VALUE_CERT_RESET_RESPONSE	        28
#define		CW_MSG_TYPE_VALUE_SITESURVEY_REQUEST		    29
#define		CW_MSG_TYPE_VALUE_SITESURVEY_RESPONSE   		30
#define		CW_MSG_TYPE_VALUE_KICKMAC_REQUEST 				31
#define		CW_MSG_TYPE_VALUE_KICKMAC_RESPONSE				32
#define     CW_MSG_TYPE_VALUE_SIMPLE_DISCOVERY_REQUEST      33
#define     CW_MSG_TYPE_VALUE_SIMPLE_DISCOVERY_RESPONESE    34
#define     CW_MSG_TYPE_VALUE_CHANGE_AC_REQUEST             35
#define     CW_MSG_TYPE_VALUE_CHANGE_AC_RESPONSE            36
#define     CW_MSG_TYPE_VALUE_KEEP_ALIVE_REQUEST            37
#define     CW_MSG_TYPE_VALUE_KEEP_ALIVE_RESPONSE           38
#define     CW_MSG_TYPE_VALUE_SHELL_CMD_REQUEST             39
#define     CW_MSG_TYPE_VALUE_SHELL_CMD_RESPONSE            40
#define     CW_MSG_TYPE_VALUE_RUN_UPG_REQUEST               41
#define     CW_MSG_TYPE_VALUE_RUN_UPG_RESPONSE              42
#define     CW_MSG_TYPE_VALUE_APPLY_CONFIG_REQUEST          43
#define     CW_MSG_TYPE_VALUE_APPLY_CONFIG_RESPONSE         44
#define     CW_MSG_TYPE_VALUE_STANDALONE_MODE_REQUEST       45
#define     CW_MSG_TYPE_VALUE_STANDALONE_MODE_RESPONSE      46
#define     CW_MSG_TYPE_VALUE_IMAGE_DOWNLOAD_STATUS_REQUEST 47
#define     CW_MSG_TYPE_VALUE_IMAGE_DOWNLOAD_STATUS_RESPONSE 48

// Message Elements Type Values
#define 	CW_MSG_ELEMENT_AC_DESCRIPTOR_CW_TYPE		1
#define 	CW_MSG_ELEMENT_AC_IPV4_LIST_CW_TYPE			2
#define 	CW_MSG_ELEMENT_AC_IPV6_LIST_CW_TYPE			3
#define 	CW_MSG_ELEMENT_AC_NAME_CW_TYPE				4
#define 	CW_MSG_ELEMENT_AC_NAME_WITH_PRI_CW_TYPE		5
#define 	CW_MSG_ELEMENT_TIMESTAMP_CW_TYPE            6
#define 	CW_MSG_ELEMENT_ADD_MAC_ACL_CW_TYPE			7
#define 	CW_MSG_ELEMENT_ADD_STATION_CW_TYPE			8
#define 	CW_MSG_ELEMENT_WTP_RESERVED_9_CW_TYPE		9
#define 	CW_MSG_ELEMENT_CW_CONTROL_IPV4_ADDRESS_CW_TYPE		10
#define 	CW_MSG_ELEMENT_CW_CONTROL_IPV6_ADDRESS_CW_TYPE		11
#define 	CW_MSG_ELEMENT_CW_TIMERS_CW_TYPE			12
#define 	CW_MSG_ELEMENT_DATA_TRANSFER_DATA_CW_TYPE		13
#define 	CW_MSG_ELEMENT_DATA_TRANSFER_MODE_CW_TYPE		14
#define 	CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_CW_TYPE		15
#define 	CW_MSG_ELEMENT_CW_DECRYPT_ER_REPORT_PERIOD_CW_TYPE	16
#define 	CW_MSG_ELEMENT_DELETE_MAC_ACL_CW_TYPE			17
#define 	CW_MSG_ELEMENT_DELETE_STATION_CW_TYPE			18
#define 	CW_MSG_ELEMENT_WTP_RESERVED_19_CW_TYPE		19
#define 	CW_MSG_ELEMENT_DISCOVERY_TYPE_CW_TYPE			20
#define 	CW_MSG_ELEMENT_DUPLICATE_IPV4_ADDRESS_CW_TYPE		21
#define 	CW_MSG_ELEMENT_DUPLICATE_IPV6_ADDRESS_CW_TYPE		22
#define 	CW_MSG_ELEMENT_IDLE_TIMEOUT_CW_TYPE			23
#define 	CW_MSG_ELEMENT_IMAGE_DATA_CW_TYPE			24
#define 	CW_MSG_ELEMENT_IMAGE_IDENTIFIER_CW_TYPE			25
#define 	CW_MSG_ELEMENT_IMAGE_INFO_CW_TYPE			26
#define 	CW_MSG_ELEMENT_INITIATED_DOWNLOAD_CW_TYPE		27
#define 	CW_MSG_ELEMENT_LOCATION_DATA_CW_TYPE			28
#define 	CW_MSG_ELEMENT_MAX_MSG_LEN_CW_TYPE			29
#define 	CW_MSG_ELEMENT_CW_LOCAL_IPV4_ADDRESS_CW_TYPE		30
#define 	CW_MSG_ELEMENT_RADIO_ADMIN_STATE_CW_TYPE		31
#define 	CW_MSG_ELEMENT_RADIO_OPERAT_STATE_CW_TYPE		32
#define 	CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE			33
#define 	CW_MSG_ELEMENT_RETURNED_MSG_ELEM_CW_TYPE		34
#define 	CW_MSG_ELEMENT_SESSION_ID_CW_TYPE			35
#define 	CW_MSG_ELEMENT_STATISTICS_TIMER_CW_TYPE			36
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE		37
#define 	CW_MSG_ELEMENT_WTP_BOARD_DATA_CW_TYPE			38
#define 	CW_MSG_ELEMENT_WTP_DESCRIPTOR_CW_TYPE			39
#define 	CW_MSG_ELEMENT_WTP_FALLBACK_CW_TYPE			40
#define 	CW_MSG_ELEMENT_WTP_FRAME_TUNNEL_MODE_CW_TYPE		41
#define 	CW_MSG_ELEMENT_WTP_RESERVED_42_CW_TYPE			42
#define 	CW_MSG_ELEMENT_WTP_RESERVED_43_CW_TYPE			43
#define 	CW_MSG_ELEMENT_WTP_MAC_TYPE_CW_TYPE			44
#define 	CW_MSG_ELEMENT_WTP_NAME_CW_TYPE				45
#define 	CW_MSG_ELEMENT_WTP_RESERVED_46_CW_TYPE		46
#define 	CW_MSG_ELEMENT_WTP_RADIO_STATISTICS_CW_TYPE		47
#define 	CW_MSG_ELEMENT_WTP_REBOOT_STATISTICS_CW_TYPE	48
#define 	CW_MSG_ELEMENT_WTP_STATIC_IP_CW_TYPE			49
#define 	CW_MSG_ELEMENT_CW_TRANSPORT_PROTOCOL_CW_TYPE		51
#define 	CW_MSG_ELEMENT_CW_LOCAL_IPV6_ADDRESS_CW_TYPE		50
#define 	CW_MSG_ELEMENT_MTU_DISCOVERY_PADDING_CW_TYPE		52
/*20130924 sigma added: */
#define 	CW_MSG_ELEMENT_WTP_CERT_RESET_REQ_TYPE_CW_TYPE      53
#define 	CW_MSG_ELEMENT_WTP_CERT_TYPE_INFO_CW_TYPE		    54
#define 	CW_MSG_ELEMENT_WTP_CERTREQ_DATA_CW_TYPE		        55
#define 	CW_MSG_ELEMENT_AC_CERT_RESET_ACTION_CW_TYPE		    56
#define 	CW_MSG_ELEMENT_AC_CERT_INFO_CW_TYPE		            57
#define 	CW_MSG_ELEMENT_AC_CA_DATA_CW_TYPE		            58
#define 	CW_MSG_ELEMENT_AC_CERT_DATA_CW_TYPE		            59
#define 	CW_MSG_ELEMENT_WTP_FACTORY_RESET_INTERVAL_CW_TYPE   60

/*Added by larger: for sent event to AC*/
#define 	CW_MSG_ELEMENT_WTP_EVENT_CLIENT_CHANGE_CW_TYPE      61
#define 	CW_MSG_ELEMENT_WTP_EVENT_SITESURVEY_CW_TYPE         62
#define 	CW_MSG_ELEMENT_AC_BOARD_DATA_CW_TYPE			    63
#define 	CW_MSG_ELEMENT_WTP_EVENT_AUTOCHANNEL_CW_TYPE        64
#define 	CW_MSG_ELEMENT_IPV4_ADDR_CW_TYPE                    65
#define 	CW_MSG_ELEMENT_AC_MANAGEMENT_STATUS_CW_TYPE         66
#define 	CW_MSG_ELEMENT_WTP_EVENT_LOG_MSG_CW_TYPE            67
#define 	CW_MSG_ELEMENT_HOST_NAME_CW_TYPE                    68
#define 	CW_MSG_ELEMENT_ADDR_PORT_CW_TYPE                    69
#define 	CW_MSG_ELEMENT_CONNECTION_ID                        70
#define 	CW_MSG_ELEMENT_UPG_FLOW                             71
#define 	CW_MSG_ELEMENT_WAIT_APPLY                           72
#define 	CW_MSG_ELEMENT_IMAGE_DOWNLOAD_STATUS                73
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CW_TYPE_32_LEN   74
#if 0
/*Update 2009:
		Message type to return a payload together with the
		configuration update response*/
#define 	CW_MSG_ELEMENT_RESULT_CODE_CW_TYPE_WITH_PAYLOAD			49
#endif
#define 	CW_MSG_ELEMENT_WTP_RADIO_INFO_CW_TYPE		1048

// CAPWAP Protocol Variables
#define     CW_AC_WAIT_HANDSHAKE_DEFAULT      20
#define     CW_WTP_WAIT_HANDSHAKE_DEFAULT     10
#define     CW_WAIT_JOIN_DEFAULT              30
#define     CW_WAIT_IMAGE_STATE_DEFAULT       30
#define     CW_AC_IMAGE_DATA_INTERVAL_DEFAULT     10
#define     CW_WTP_IMAGE_DATA_INTERVAL_DEFAULT    30
#define     CW_AC_WAIT_CONFIGURE_STATE_DEFAULT    180
#define     CW_WTP_WAIT_CONFIGURE_STATE_DEFAULT   30
#define     CW_AC_CHANGE_STATE_INTERVAL_DEFAULT   60
#define     CW_WTP_CHANGE_STATE_INTERVAL_DEFAULT  30
#define     CW_REPORT_INTERVAL_DEFAULT            120
#define     CW_STATISTIC_TIMER_DEFAULT            60
#define     CW_STATISTIC_POLL_TIMER_DEFAULT       5
#define     CW_RETRANSMIT_INTERVAL_DEFAULT        10
#define     CW_WTP_RETRANSMIT_INTERVAL_DEFAULT    10
#define     CW_WTP_PACKET_INTERVEL_DEFAULT        0 /*microseconds*/

#define     CW_MAX_RETRANSMIT_DEFAULT             3
#define     CW_ECHO_INTERVAL_DEFAULT          20 /* no longer than 60 sec which would greater than NAT UDP session time */
#define     CW_SITESURVEY_INTERVAL_DEFAULT    300
#define     CW_AUTO_TXPOWER_INTERVAL_DEFAULT  500
#define     CW_AUTO_CHANNEL_SELECTION_INTERVAL_DEFAULT  500
#define     CW_FRAGMENTS_INTERVAL_DEFAULT     5
#define     CW_NEIGHBORDEAD_INTERVAL(echo)    ((echo * 3) + 10) // should be longer than 3 times of echo interval
#define     CW_NEIGHBORDEAD_INTERVAL_DEFAULT  CW_NEIGHBORDEAD_INTERVAL(CW_ECHO_INTERVAL_DEFAULT)
#define     CW_WAIT_CERT_RESET_DEFAULT        30
#define     CW_WAIT_DTLS_HANDSHAKE_TIMEOUT    80
#define     CW_UNEXECEPTED_REJOIN_INTERVAL(echo) (echo + 1)
#define     CW_IMAGE_ECHO_INTERVAL_DEFAULT    (10)
#define     CW_IMAGE_ECHO_TIMEOUT(echo)       ((2 * echo) + 1)

/* WTP Discovery related */
#define     CW_DISCOVERY_INTERVAL_DEFAULT     10
#define     CW_MAX_DISCOVERY_INTERVAL_DEFAULT 20
#define     CW_MAX_DISCOVERY_RETRY            2
#define     CW_MAX_REJOIN_DISCOVERY_RETRY     2
#define     CW_SLUCNKING_INTERVEAL            5
#define     CW_DISCOVERY_STATE_TIMEOUT        CW_SLUCNKING_INTERVEAL+CW_MAX_DISCOVERY_INTERVAL_DEFAULT+6
#define     CW_WTP_MIN_REBOOT_TIME            60

/*20131003 Sigma*/
#define CW_AC_CA_NAME_DEFAULT               "defaultRoot.pem"
#define CW_AC_CA_NAME_CURRENT               "currentRoot.pem"
#define CW_AC_CERT_NAME_DEFAULT             "defaultAC.pem"
#define CW_AC_CERT_NAME_CURRENT             "currentAC.pem"
#define CW_AC_PRIVATE_KEY_DEFAULT           "defaultACKey.pem"
#define CW_AC_PRIVATE_KEY_CURRENT           "currentACKey.pem"
#define CW_AC_PKEY_PASSWORD_DEFAULT         CW_CERT_PASSWORD
#define CW_AC_PKEY_PASSWORD_CURRENT         CW_AC_PKEY_PASSWORD_DEFAULT

#define CW_WTP_CA_NAME_DEFAULT              "defaultWTPRoot.pem"
#define CW_WTP_CA_NAME_CURRENT              "currentWTPRoot.pem"
#define CW_WTP_CERT_NAME_DEFAULT            "defaultWTP.pem"
#define CW_WTP_CERT_NAME_CURRENT            "currentWTP.pem"
#define CW_WTP_PRIVATE_KEY_DEFAULT          "defaultWTPKey.pem"
#define CW_WTP_PRIVATE_KEY_CURRENT          "currentWTPKey.pem"
#define CW_WTP_PKEY_PASSWORD_DEFAULT        CW_AC_PKEY_PASSWORD_DEFAULT
#define CW_WTP_PKEY_PASSWORD_CURRENT        CW_AC_PKEY_PASSWORD_CURRENT

#define CW_WTP_FILE_CERT_REQ                "WTP.req"
#define CW_WTP_FILE_CERT_NEW                "newWTP.pem"
#define CW_WTP_PRIVATE_KEY_NEW              "newWTPKey.pem"         /*For test*/

/*_________________________________________________________*/
/*  *******************___VARIABLES___*******************  */

/*_____________________________________________________*/
/*  *******************___TYPES___*******************  */

typedef enum
{
    CW_WTP_WBID_802_11     = 1,
    CW_WTP_WBID_EPC_GLOBAL = 3,
    CW_WTP_WBID_802_3      = 4 /* added by senao */
} CWWbidType;

typedef enum
{
    CW_SINGLE_PACKET     = 0,
    CW_MULTIC_PACKET     = 1
} CWVersionType;


typedef struct
{
    int type;
    union
    {
        int value;
        char *string;
        void *data;
    };
} CWMsgElemData;

/************************************************************
      0  Success

      1  Failure (AC List Message Element MUST Be Present)

      2  Success (NAT Detected)

      3  Join Failure (Unspecified)

      4  Join Failure (Resource Depletion)

      5  Join Failure (Unknown Source)

      6  Join Failure (Incorrect Data)

      7  Join Failure (Session ID Already in Use)

      8  Join Failure (WTP Hardware Not Supported)

      9  Join Failure (Binding Not Supported)

      10 Reset Failure (Unable to Reset)

      11 Reset Failure (Firmware Write Error)

      12 Configuration Failure (Unable to Apply Requested Configuration
         - Service Provided Anyhow)

      13 Configuration Failure (Unable to Apply Requested Configuration
         - Service Not Provided)

      14 Image Data Error (Invalid Checksum)

      15 Image Data Error (Invalid Data Length)

      16 Image Data Error (Other Error)

      17 Image Data Error (Image Already Present)

      18 Message Unexpected (Invalid in Current State)

      19 Message Unexpected (Unrecognized Request)

      20 Failure - Missing Mandatory Message Element

      21 Failure - Unrecognized Message Element

      22 Data Transfer Error (No Information to Transfer)

****************************************************************/

typedef enum
{
    CW_PROTOCOL_SUCCESS				= 0, //	Success
    CW_PROTOCOL_FAILURE_AC_LIST			= 1, // AC List message MUST be present
    CW_PROTOCOL_SUCCESS_NAT				= 2, // NAT detected
    CW_PROTOCOL_FAILURE				= 3, // unspecified
    CW_PROTOCOL_FAILURE_RES_DEPLETION		= 4, // Resource Depletion
    CW_PROTOCOL_FAILURE_UNKNOWN_SRC			= 5, // Unknown Source
    CW_PROTOCOL_FAILURE_INCORRECT_DATA		= 6, // Incorrect Data
    CW_PROTOCOL_FAILURE_ID_IN_USE			= 7, // Session ID Alreadyin Use
    CW_PROTOCOL_FAILURE_WTP_HW_UNSUPP		= 8, // WTP Hardware not supported
    CW_PROTOCOL_FAILURE_BINDING_UNSUPP		= 9, // Binding not supported
    CW_PROTOCOL_FAILURE_UNABLE_TO_RESET		= 10, // Unable to reset
    CW_PROTOCOL_FAILURE_FIRM_WRT_ERROR		= 11, // Firmware write error
    CW_PROTOCOL_FAILURE_SERVICE_PROVIDED_ANYHOW	= 12, // Unable to apply requested configuration
    CW_PROTOCOL_FAILURE_SERVICE_NOT_PROVIDED	= 13, // Unable to apply requested configuration
    CW_PROTOCOL_FAILURE_INVALID_CHECKSUM		= 14, // Image Data Error: invalid checksum
    CW_PROTOCOL_FAILURE_INVALID_DATA_LEN		= 15, // Image Data Error: invalid data length
    CW_PROTOCOL_FAILURE_OTHER_ERROR			= 16, // Image Data Error: other error
    CW_PROTOCOL_FAILURE_IMAGE_ALREADY_PRESENT	= 17, // Image Data Error: image already present
    CW_PROTOCOL_FAILURE_INVALID_STATE		= 18, // Message unexpected: invalid in current state
    CW_PROTOCOL_FAILURE_UNRECOGNIZED_REQ		= 19, // Message unexpected: unrecognized request
    CW_PROTOCOL_FAILURE_MISSING_MSG_ELEM		= 20, // Failure: missing mandatory message element
    CW_PROTOCOL_FAILURE_UNRECOGNIZED_MSG_ELEM	= 21,  // Failure: unrecognized message element
    CW_PROTOCOL_FAILURE_DATA_TRANS_ERROR	= 22  // Failure: unrecognized message element

} CWProtocolResultCode;

typedef struct
{
    char *msg;
    int offset;
    int data_msgType;
    int combine_len;
} CWProtocolMessage;

typedef struct
{
    int payloadType;
    int type;
    int isFragment;
    int last;
    int fragmentID;
    int fragmentOffset;
    int keepAlive;
    CWWTPType wtpType;
    int nextFragmentID;
    int fragEnd;
    int firstFrag;
    int protocolVersion;
    int totalLen;
} CWProtocolTransportHeaderValues;

typedef struct
{
    unsigned int messageTypeValue;
    unsigned char seqNum;
    int msgElemsLen;
    //	unsigned int timestamp;
} CWControlHeaderValues;

typedef struct
{
    char *data;
    int dataLen;
    CWProtocolTransportHeaderValues transportVal;
} CWProtocolFragment;

typedef enum
{
    /*************** Board data ****************/
    CW_BOARD_MODEL_NAME = 0,
    CW_BOARD_SERIAL_NUMBER = 1,
    CW_BOARD_ID = 2,
    CW_BOARD_REVISION = 3,
    CW_BOARD_BASE_MAC = 4,
    /* SENAO ADD */
    CW_BOARD_SENAO_CAP_CODE = 100,
    CW_BOARD_SENAO_SKU = 101,

    /*************** WTP Descriptor ****************/
    CW_WTP_HARDWARE_VERSION = 0,
    CW_WTP_SOFTWARE_VERSION = 1,
    CW_WTP_BOOT_VERSION 	= 2,
    /* SENAO ADD */
    CW_WTP_SENAO_CAPWAP_VERSION = 100,
    CW_WTP_SENAO_DEVICE_NAME 	= 101,
    CW_WTP_SENAO_UPGRADABLE_VERSION  = 102,
    CW_WTP_SENAO_SWITCH_PORT_HW_INFO = 103,
} CWVendorInfoType;

typedef struct
{
    int vendorIdentifier;
    CWVendorInfoType type;
    int length;
    int *valuePtr;
} CWWTPVendorInfoValues;

typedef struct
{
    int vendorInfosCount;
    CWWTPVendorInfoValues *info;
} CWWTPVendorInfos;

typedef enum
{
    CW_ENC_CAP_AES = 4,
    CW_ENC_CAP_TKIP = 8,
    CW_ENC_CAP_TKIP_AES = (CW_ENC_CAP_AES | CW_ENC_CAP_TKIP)
} CWEncryptCapType;

typedef struct
{
    CWWTPType wtpType;
    int maxRadios;
    int radiosInUse;
    CWEncryptCapType encryptCap;
    CWWTPVendorInfos vendorInfos;
} CWWTPDescriptor;

typedef enum
{
    CW_LOCAL_BRIDGING = 1,
    CW_802_DOT_3_BRIDGING = 2,
    CW_NATIVE_BRIDGING = 4,
    CW_ALL_ENC = 7
} CWframeTunnelMode;

typedef enum
{
    CW_LOCAL_MAC = 0,
    CW_SPLIT_MAC = 1,
    CW_BOTH = 2
} CWMACType;

typedef enum
{
    CW_802_DOT_11b = 1,
    CW_802_DOT_11a = 2,
    CW_802_DOT_11g = 4,
    CW_802_DOT_11n = 8,
    CW_802_DOT_11ac = 16, /* SENAO ADD */
    CW_802_DOT_11ax = 32,
    CW_ALL_RADIO_TYPES = 0x3F
} CWRadioType;

typedef struct
{
    CWMacAddress bssid;
} CWWlanInformationValues;

typedef struct
{
    int version; /* version of the information */
    int ID;
    CWRadioType type;
    int maxTxPower;
    CWMacAddress mac;
    int maxWlans;
    CWWlanInformationValues *wlans;
} CWRadioInformationValues;

typedef struct
{
    int radiosCount;
    CWRadioInformationValues *radios;
} CWRadiosInformation;

typedef enum
{
    CW_MSG_ELEMENT_DISCOVERY_TYPE_UNKNOWN = 0,
    CW_MSG_ELEMENT_DISCOVERY_TYPE_CONFIGURED = 1,
    CW_MSG_ELEMENT_DISCOVERY_TYPE_DHCP = 2,
    CW_MSG_ELEMENT_DISCOVERY_TYPE_DNS = 3,
    CW_MSG_ELEMENT_DISCOVERY_TYPE_AC_REFER = 4,
    CW_MSG_ELEMENT_DISCOVERY_TYPE_TEST = 5,
    CW_MSG_ELEMENT_DISCOVERY_TYPE_EZCOM = 6
} CWDiscoveryType;

typedef struct
{
    int vendorIdentifier;
    CWMacAddress mac;
} CWDiscoveryAcData;

typedef struct
{
    CWDiscoveryType type;
    CWWTPVendorInfos WTPBoardData;
    CWWTPDescriptor WTPDescriptor;
    CWframeTunnelMode frameTunnelMode;
    CWMACType MACType;
    CWRadiosInformation radiosInfo;
    CWIPCfgInfo *ipCfgInfo;
    CWWtpCfgCapInfo *cfgCapInfo;
    CWNetworkLev4Address *proxyAddr;
    CWDiscoveryAcData AcBoardData;
} CWDiscoveryRequestValues;

typedef enum
{
    CW_X509_CERTIFICATE = 1,
    CW_PRESHARED = 0
} CWAuthSecurity;

typedef struct
{
    CWNetworkLev4Address addr;
    CWNetworkLev4Address mask;
    struct sockaddr_in addrIPv4;
    int WTPCount;
} CWProtocolNetworkInterface;

typedef struct
{
    int WTPCount;
    struct sockaddr_in addr;
} CWProtocolIPv4NetworkInterface;

typedef struct
{
    int WTPCount;
    struct sockaddr_in6 addr;
} CWProtocolIPv6NetworkInterface;

typedef struct
{
    int vendorIdentifier;
    enum
    {
        CW_AC_HARDWARE_VERSION	= 4,
        CW_AC_SOFTWARE_VERSION	= 5
    } type;
    int length;
    int *valuePtr;
} CWACVendorInfoValues;

typedef struct
{
    int vendorInfosCount;
    CWACVendorInfoValues *vendorInfos;
} CWACVendorInfos;

typedef struct
{
    int rebootCount;
    int ACInitiatedCount;
    int linkFailurerCount;
    int SWFailureCount;
    int HWFailuireCount;
    int otherFailureCount;
    int unknownFailureCount;
    enum
    {
        NOT_SUPPORTED = 0,
        AC_INITIATED = 1,
        LINK_FAILURE = 2,
        SW_FAILURE = 3,
        HD_FAILURE = 4,
        OTHER_FAILURE = 5,
        UNKNOWN = 255
    } lastFailureType;
} WTPRebootStatisticsInfo;

typedef struct
{
    int radioID;
    int reportInterval;
} WTPDecryptErrorReportValues;

typedef struct
{
    int radiosCount;
    WTPDecryptErrorReportValues *radios;
} WTPDecryptErrorReport;

typedef struct
{
    int priority;
    char *ACName;
} CWACNameWithPriorityValues;

typedef struct
{
    int count;
    CWACNameWithPriorityValues *ACs;
} CWACNamesWithPriority;

typedef enum
{
    ENABLED = 1,
    DISABLED = 2
} CWRadioState;

typedef enum
{
    AD_NORMAL = 1,
    AD_RADIO_FAILURE = 2,
    AD_SOFTWARE_FAILURE = 3,
    AD_RADAR_DETECTION = 4
} CWAdminCause;

typedef enum
{
    OP_NORMAL = 0,
    OP_RADIO_FAILURE = 1,
    OP_SOFTWARE_FAILURE = 2,
    OP_ADMINISTRATIVELY_SET = 3
} CWOperationalCause;

typedef struct
{
    int ID;
    CWRadioState state;
} CWRadioAdminInfoValues;

typedef struct
{
    int radiosCount;
    CWRadioAdminInfoValues *radios;
} CWRadiosAdminInfo;

typedef struct
{
    int ID;
    CWRadioState state;
    CWOperationalCause cause;
} CWRadioOperationalInfoValues;

typedef struct
{
    int radiosCount;
    CWRadioOperationalInfoValues *radios;
} CWRadiosOperationalInfo;

typedef struct
{
    int ID;
    unsigned char numEntries;
    unsigned char length;
    CWMacAddress *errorMACAddressList;
} CWDecryptErrorReportValues;

typedef struct
{
    int radiosCount;
    CWDecryptErrorReportValues *radios;
} CWDecryptErrorReportInfo;

typedef struct
{
    enum
    {
        STATISTICS_NOT_SUPPORTED = 0,
        SW_FAILURE_TYPE = 1,
        HD_FAILURE_TYPE = 2,
        OTHER_TYPES = 3,
        UNKNOWN_TYPE = 255
    } lastFailureType;
    short int resetCount;
    short int SWFailureCount;
    short int HWFailuireCount;
    short int otherFailureCount;
    short int unknownFailureCount;
    short int configUpdateCount;
    short int channelChangeCount;
    short int bandChangeCount;
    short int currentNoiseFloor;
} CWRadioStatisticsInfo;

typedef struct
{
    CWMacAddress bssid;
    CWWtpCfgCap cfgCap;
    int stationCount;
    CWStation *station;
    CWWlanStatistics statistics;
} CWWTPWlanInfo;

typedef struct
{
    unsigned int radioID;
    int type;
    int maxTxPower;
    CWWtpCfgCap cfgCap;
    int curChannel;
    int curTxPower;
    int curAutoTxPowerStrength; //%
    unsigned char curChanCount;
    unsigned char *curChanList;
    int curStaCount;
    int maxWlans;
    CWWTPWlanInfo *wlanInfo;

    CWMacAddress mac;
    CWRadioMeshInfo mesh;
    //Station Mac Address List
    CWList decryptErrorMACAddressList;

    unsigned int reportInterval;

    CWRadioState adminState;
    CWRadioState operationalState;
    CWOperationalCause operationalCause;

    CWRadioStatisticsInfo statistics;

    //    void* bindingValuesPtr;
} CWWTPRadioInfoValues;

typedef struct
{
    int radioCount;
    CWWTPRadioInfoValues *info;
} CWWTPRadiosInfo;

typedef struct
{
    unsigned int vendorId;
    unsigned short vendorPayloadType;
    void *payload;
} CWProtocolVendorSpecificValues;

typedef struct
{
    int dataLen;
    char *data;
    CWBool failed;
    CWBool eof;
} CWImageData;

#define CW_MAX_IMAGE_NAME_LEN 256

typedef struct
{
    CWBool necessary;
    char imageName[CW_MAX_IMAGE_NAME_LEN];
} CWImageIdentifier;

typedef struct
{
    CWBool initDownload;
    CWImageIdentifier imageId;
} CWWTPImageDataRequestValues;

typedef struct
{
    CWImageData imageData;
    int wait;
    int upgFlow;
} CWACImageDataRequestValues;

typedef struct
{
    int fileSize;
    unsigned char hash[16];
} CWImageInformation;

typedef struct
{
    CWProtocolResultCode resultCode;
    int waitSeconds;
} CWWTPImageDataResponseValues;

typedef struct
{
    int size; /* how many size WTP has downloaded */
    CWBool end; /* download is end */
    CWProtocolResultCode resultCode;
} CWWTPImageDownloadStatus;

typedef struct
{
    CWWTPImageDownloadStatus status;
    int waitSeconds;
} CWWTPImageDownloadRequestValues;

typedef struct
{
    int upgFlow;
} CWACImageDownloadReponseValues;

typedef struct
{
    CWProtocolResultCode resultCode;
    CWImageInformation imageInfo;
} CWACImageDataResponseValues;

typedef struct
{
    CWProtocolResultCode resultCode;
    int waitSeconds;
} CWACRunUpgResponseValues;

typedef enum
{
    CW_WTP_CERT_REQUEST_TYPE_QUERY,             /*ask AC that does WTP need to reset cert*/
    CW_WTP_CERT_REQUEST_TYPE_CERT_REQ,          /*ask AC for the new certificate*/
    CW_WTP_CERT_REQUEST_TYPE_FACTORY_RESET,     /*tell AC that WTP needs to do factory reset*/
} CWWTPCertRequestType;

typedef enum
{
    CW_WTP_CERT_TYPE_DEFAULT,           /*Indicate that WTP is using default cert to handshake*/
    CW_WTP_CERT_TYPE_CURRENT,           /*Indicate that WTP is using current cert to handshake*/
    CW_WTP_CERT_TYPE_CURRENT_DEFAULT,   /*Indicate that WTP's current and default certificate is the same*/
} CWWTPCertType;

typedef enum
{
    CW_CERT_RESET_ACTION_NONE,              /*notify WTP no need to take any action*/
    CW_CERT_RESET_ACTION_FACTORY_RESET,     /*notify WTP to factory reset current device settings*/
    CW_CERT_RESET_ACTION_CERT_UPDATE,       /*notify WTP that cert needs to update*/
} CWCertResetAction;

typedef struct
{
    int dataLen;
    unsigned char certreqHash[16];
    char *data;
} CWWTPCertReqData;

typedef struct
{
    int caFileSize;
    unsigned char caHash[16];
    int certFileSize;
    unsigned char certHash[16];
} CWCertInformation;

typedef struct
{
    int dataLen;
    char *data;
    CWBool eof;
} CWACCaData;

typedef struct
{
    int dataLen;
    char *data;
    CWBool eof;
} CWACCertData;

typedef struct
{
    int outputType;
    int timeout;
    int cmdLen;
    char *cmd;
} CWShellCmdInfo;

typedef struct
{
    int group;
    int categoryLen;
    char *category;
    int level;
    int msgLen;
    char *msg;
} CWWTPLogMsg;

typedef struct
{
    CWProtocolResultCode resultCode;
    CWCertInformation certInfo;
    CWACCaData caData;
    CWACCertData certData;
} CWACNewCertResponseValues;

typedef struct
{
    CWProtocolResultCode resultCode;
    CWCertResetAction   certAction;
} CWACCertResetResponseValues;

typedef struct
{
    CWProtocolResultCode resultCode;
} CWACFactoryResetResponseValues;

typedef struct
{
    CWProtocolResultCode resultCode;
} CWWTPCertResetResponseValues;

typedef struct
{
    CWProtocolResultCode resultCode;
} CWWTPCertResetFactoryResetValues;

typedef struct
{
    CWWTPCertRequestType certReqType;
    CWWTPCertType certType;
    CWWTPCertReqData certReqData;
    int certFactoryResetInterval;
} CWWTPCertResetRequestValues;

#include "CWList.h"

/*__________________________________________________________*/
/*  *******************___PROTOTYPES___*******************  */
int CWGetFragmentID(); // provided by the user of CWProtocol lib

void CWWTPResetRadioStatistics(CWRadioStatisticsInfo *radioStatistics);

void CWProtocolDestroyMsgElemData(void *f);
void CWFreeMessageFragments(CWProtocolMessage *messages, int fragmentsNum);

void CWProtocolStore8(CWProtocolMessage *msgPtr, unsigned char val);
void CWProtocolStore16(CWProtocolMessage *msgPtr, unsigned short val);
void CWProtocolStore32(CWProtocolMessage *msgPtr, unsigned int val);
void CWProtocolStoreStr(CWProtocolMessage *msgPtr, const char *str);
void CWProtocolStoreMessage(CWProtocolMessage *msgPtr, CWProtocolMessage *msgToStorePtr);
void CWProtocolStoreRawBytes(CWProtocolMessage *msgPtr, const char *bytes, int len);
void CWProtocolStoreIPv4Address(CWProtocolMessage *msgPtr, unsigned int addr);

unsigned char CWProtocolRetrieve8(CWProtocolMessage *msgPtr);
unsigned short CWProtocolRetrieve16(CWProtocolMessage *msgPtr);
unsigned int CWProtocolRetrieve32(CWProtocolMessage *msgPtr);
char *CWProtocolRetrieveStr(CWProtocolMessage *msgPtr, int len);
char *CWProtocolRetrieveStrNoCreate(CWProtocolMessage *msgPtr, char *str, int len);
char *CWProtocolRetrieveRawBytes(CWProtocolMessage *msgPtr, int len);
char *CWProtocolRetrieveRawBytesNoCreate(CWProtocolMessage *msgPtr, char *bytes, int len);
char *CWProtocolRetrieveRawBytesPtr(CWProtocolMessage *msgPtr, char **bytes, int len);
unsigned int CWProtocolRetrieveIPv4Address(CWProtocolMessage *msgPtr);

CWBool CWProtocolParseFragment(char *buf, int readBytes, CWList *fragmentsListPtr, CWProtocolMessage *reassembledMsg, CWBool *dataFlag);
void CWProtocolDestroyFragment(void *f);

CWBool CWParseTransportHeader(CWProtocolMessage *msgPtr, CWProtocolTransportHeaderValues *valuesPtr, CWBool *dataFlag);
CWBool CWParseControlHeader(CWProtocolMessage *msgPtr, CWControlHeaderValues *valPtr);
CWBool CWParseFormatMsgElem(CWProtocolMessage *completeMsg, unsigned short int *type, unsigned int *len);

CWBool CWAssembleTransportHeader(CWProtocolMessage *transportHdrPtr, CWProtocolTransportHeaderValues *valuesPtr);
CWBool CWAssembleControlHeader(CWProtocolMessage *controlHdrPtr, CWControlHeaderValues *valPtr);
CWBool CWAssembleMessage(CWProtocolMessage **completeMsgPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int msgTypeValue, CWProtocolMessage *msgElems, const int msgElemNum, CWProtocolMessage *msgElemsBinding, const int msgElemBindingNum, int is_crypted);
CWBool CWAssembleMsgElem(CWProtocolMessage *msgPtr, unsigned int type);
CWBool CWAssembleUnrecognizedMessageResponse(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, int msgType);

CWBool CWAssembleMsgElemRadioAdminState(CWProtocolMessage *msgPtr);			//29
CWBool CWAssembleMsgElemRadioOperationalState(int radioID, CWProtocolMessage *msgPtr);	//30
CWBool CWAssembleMsgElemResultCode(CWProtocolMessage *msgPtr, CWProtocolResultCode code);//31
CWBool CWAssembleMsgElemUpgFlow(CWProtocolMessage *msgPtr, CWUpgFlow flow);
CWBool CWAssembleMsgElemImageDownloadStatus(CWProtocolMessage *msgPtr, CWWTPImageDownloadStatus *status);
CWBool CWAssembleMsgElemImageInitDownload(CWProtocolMessage *msgPtr);
CWBool CWAssembleMsgElemImageData(CWProtocolMessage *msgPtr, CWImageData *imageData);
CWBool CWAssembleMsgElemImageIdentifier(CWProtocolMessage *msgPtr, CWImageIdentifier *imageId);
CWBool CWAssembleMsgElemImageInformation(CWProtocolMessage *msgPtr, CWImageInformation *imageInfo);
//CWBool CWAssembleVendorMsgElemResultCodeWithPayload(CWProtocolMessage *msgPtr,CWProtocolResultCode code, CWProtocolVendorSpecificValues *payload);//49
CWBool CWAssembleMsgElemSessionID(CWProtocolMessage *msgPtr, int sessionID);		//32

CWBool CWParseACName(CWProtocolMessage *msgPtr, int len, char **valPtr);
CWBool CWParseWTPRadioOperationalState(CWProtocolMessage *msgPtr, int len, CWRadioOperationalInfoValues *valPtr);	//30
CWBool CWParseResultCode(CWProtocolMessage *msgPtr, int len, CWProtocolResultCode *valPtr);			//31
CWBool CWParseUpgFlow(CWProtocolMessage *msgPtr, int len, int *valPtr);
CWBool CWParseImageDownloadStatus(CWProtocolMessage *msgPtr,  int len, CWWTPImageDownloadStatus *valPtr);
CWBool CWParseImageData(CWProtocolMessage *msgPtr, int len, CWImageData *imageData);
CWBool CWParseImageIdentifier(CWProtocolMessage *msgPtr, int len, CWImageIdentifier *imageId);
CWBool CWParseImageInformation(CWProtocolMessage *msgPtr, int len, CWImageInformation *imageInfo);
CWBool CWGetVendorInfoValue(CWWTPVendorInfos *vendorInfo, int type, void **ptr, int *len);

/* SENAO specific vendor element */
CWBool CWAssembleMsgElemVendorPayloadWtpCfg(CWProtocolMessage *msgPtr, CWWtpCfgMsgList *cfgList);
CWBool CWAssembleMsgElemVendorPayloadWtpCfgResult(CWProtocolMessage *msgPtr, CWWtpCfgResult *cfgResult);
CWBool CWAssembleMsgElemVendorPayloadValue32(CWProtocolMessage *msgPtr, int value, int payloadType);
#define CWAssembleMsgElemVendorPayloadWaitingTime(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue32(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WAITING_TIME)
#define CWAssembleMsgElemVendorPayloadSystemUpTime(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue32(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SYSTEM_UPTIME)
#define CWAssembleMsgElemVendorPayloadStationStatsInterval(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue32(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_INTERVAL)
#define CWAssembleMsgElemVendorPayloadStationStatsPollInterval(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue32(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_POLL_INTERVAL)
#define CWAssembleMsgElemVendorPayloadStationStatsMaxClients(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue32(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_MAX_CLIENTS)
#define CWAssembleMsgElemVendorPayloadWtpPacketIntervl(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue32(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_PACKET_INTERVEL)		
#define CWAssembleMsgElemVendorPayloadMTU(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue32(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MTU)
CWBool CWAssembleMsgElemVendorPayloadValue8(CWProtocolMessage *msgPtr, unsigned char value, int payloadType);
#define CWAssembleMsgElemVendorPayloadDebugLog(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue8(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_DEBUG_LOG)
#define CWAssembleMsgElemVendorPayloadUTCTime(_msgPtr, _value) \
			CWAssembleMsgElemVendorPayloadValue32(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_UTC_TIME)

#define CWAssembleMsgElemVendorPayloadClientStateChangeEventEnable(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue8(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CLIENT_STATE_CHANGE_EVENT_ENABLE)
CWBool CWAssembleMsgElemVendorPayloadString(CWProtocolMessage *msgPtr, const char *str, int payloadType);
CWBool CWAssembleMsgElemVendorPayloadShellCmd(CWProtocolMessage *msgPtr, CWShellCmdInfo *valPtr);
#define CWAssembleMsgElemVendorPayloadShellCmdOutput(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadString(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD_OUTPUT)
#define CWAssembleMsgElemVendorPayloadMemoryLogThreshold(_msgPtr, _value) \
		CWAssembleMsgElemVendorPayloadValue8(_msgPtr, _value, CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MEMORY_LOG_THRESHOLD)

CWBool CWAssembleMsgElemVendorPayloadIPCfgInfo(CWProtocolMessage *msgPtr, CWIPCfgInfo *valPtr);
CWBool CWAssembleMsgElemVendorPayloadCapwapTimer(CWProtocolMessage *msgPtr, CWTimer *valPtr);
CWBool CWAssembleMsgElemVendorPayloadWtpCfgCapInfo(CWProtocolMessage *msgPtr, CWWtpCfgCapInfo *valPtr);
CWBool CWAssembleMsgElemVendorPayloadAcCfgCapInfo(CWProtocolMessage *msgPtr, CWWtpCfgCapInfo *valPtr);

CWBool CWAssembleMsgElemVendorPayloadProxyRespWtpInfo(CWProtocolMessage *msgPtr, CWProxyRespWTPInfo *wtpInfo);
CWBool CWAssembleMsgElemVendorPayloadSitesurvey(CWProtocolMessage *msgPtr, unsigned char *radio);
CWBool CWAssembleMsgElemBackgroundSitesurveyValue(CWProtocolMessage *msgPtr, CWBackgroundSitesurveyValues *bgStsrvyVal);
CWBool CWAssembleMsgElemAutoTxPowerHealingValue(CWProtocolMessage *msgPtr, CWAutoTxPowerHealingValues *txPwHealingVal);
CWBool CWAssembleMsgElemVendorPayloadKickmac(CWProtocolMessage *msgPtr, CWWTPKickmacInfo *kicks);
CWBool CWAssembleMsgElemVendorPayloadMemoryInfo(CWProtocolMessage *msgPtr, CWMemoryInfo *valPtr);
CWBool CWAssembleMsgElemVendorPayloadApplyConfigInfo(CWProtocolMessage *msgPtr, CWWTPApplyConfigInfo *valPtr);
CWBool CWParseWtpCfgPayload(CWProtocolMessage *msgPtr, CWWtpCfgMsgList *cfgList);
CWBool CWParseBackgroundSitesurveyPayload(CWProtocolMessage *msgPtr, CWBackgroundSitesurveyValues *bgStSvyVal);
CWBool CWParseAutoHealTxPowerPayload(CWProtocolMessage *msgPtr, CWAutoTxPowerHealingValues *txPwHealingVal);
CWBool CWParseKickmacInfoPayload(CWProtocolMessage *msgPtr, CWWTPKickmacInfo *kickmac);
CWBool CWParseWtpCfgResultPayload(CWProtocolMessage *msgPtr, CWWtpCfgResult *cfgRes);
CWBool CWParseShellCmdPayload(CWProtocolMessage *msgPtr, CWShellCmdInfo *valPtr);
CWBool CWParseMemoryInfoPayload(CWProtocolMessage *msgPtr, CWMemoryInfo *valPtr);
CWBool CWParseApplyConfigInfoPayload(CWProtocolMessage *msgPtr, CWWTPApplyConfigInfo *valPtr);
CWBool CWParseValue32(CWProtocolMessage *msgPtr, int *value);
CWBool CWParseValue8(CWProtocolMessage *msgPtr, unsigned char *value);
CWBool CWParseString(CWProtocolMessage *msgPtr, CWStringValue *value);
CWBool CWParseWtpStationInfo(CWProtocolMessage *msgPtr, CWWTPStationInfo *staInfo, int payloadType);
CWBool CWParseWtpStatisticsInfo(CWProtocolMessage *msgPtr, CWWTPStatisticsInfo *statInfo);
CWBool CWParseWtpSwitchPortInfo(CWProtocolMessage *msgPtr, CWWTPSwitchPortInfo *statInfo);
CWBool CWParseWtpSwitchPoeInfo(CWProtocolMessage *msgPtr, CWWTPSwitchPoeInfo *statInfo);
CWBool CWParseWtpSwitchTopology(CWProtocolMessage *msgPtr, CWWTPSwitchTopologyInfo *devList);
CWBool CWParseWtpSwitchTrunkInfo(CWProtocolMessage *msgPtr, CWWTPSwitchTrunkInfo *statInfo);
CWBool CWParseWtpSitesurveyInfo(CWProtocolMessage *msgPtr, CWWTPSitesurveyInfo *sitesurveyInfo);
CWBool CWParseIpCfg(CWProtocolMessage *msgPtr, CWIPCfgInfo *valPtr);
CWBool CWParseCapwapTimer(CWProtocolMessage *msgPtr, CWTimer *valPtr);
CWBool CWParseWtpCfgCapInfo(CWProtocolMessage *msgPtr, CWWtpCfgCapInfo *valPtr);
CWBool CWParseWtpCurrentCfgInfo(CWProtocolMessage *msgPtr, CWWTPCurrentCfgInfo *valPtr);
CWBool CWParseProxyAddress(CWProtocolMessage *msgPtr, CWNetworkLev4Address *addr);
CWBool CWParseProxyRespWTPInfo(CWProtocolMessage *msgPtr, CWProxyRespWTPInfo *wtpInfo);
CWBool CWParseCertInformation(CWProtocolMessage *msgPtr, int len, CWCertInformation *valPtr);
CWBool CWParseCertType(CWProtocolMessage *msgPtr, int len, CWWTPCertType *valPtr);
CWBool CWParseCertResetReqData(CWProtocolMessage *msgPtr, int len, CWWTPCertReqData *valPtr);
CWBool CWParseCertResetReqType(CWProtocolMessage *msgPtr, int len, CWWTPCertRequestType *valPtr);
CWBool CWParseCertData(CWProtocolMessage *msgPtr, int len, CWACCertData *valPtr);
CWBool CWParseCertResetAction(CWProtocolMessage *msgPtr, int len, CWCertResetAction *valPtr);
CWBool CWParseCaData(CWProtocolMessage *msgPtr, int len, CWACCaData *valPtr);
CWBool CWParseFactoryResetInterval(CWProtocolMessage *msgPtr, int len, int *valPtr);
CWBool CWParseHostName(CWProtocolMessage *msgPtr, int len, CWHostName host);
CWBool CWAssembleMsgElemCertType(CWProtocolMessage *msgPtr, CWWTPCertType certType);
CWBool CWAssembleMsgElemCertResetReqType(CWProtocolMessage *msgPtr, CWWTPCertRequestType reqType);
CWBool CWAssembleMsgElemFactoryResetInterval(CWProtocolMessage *msgPtr, int seconds);
CWBool CWAssembleMsgElemCertResetAction(CWProtocolMessage *msgPtr, CWCertResetAction *valPtr);
CWBool CWAssembleMsgElemCaData(CWProtocolMessage *msgPtr, CWACCaData *caData);
CWBool CWAssembleMsgElemCertInformation(CWProtocolMessage *msgPtr, CWCertInformation *certInfo);
CWBool CWAssembleMsgElemCertData(CWProtocolMessage *msgPtr, CWACCertData *certData);
CWBool CWAssembleMsgElemWaitApply(CWProtocolMessage *msgPtr);
CWBool CWParseMeshInfoPayload(CWProtocolMessage *msgPtr, CWWTPMeshInfo *valPtr);
#endif
