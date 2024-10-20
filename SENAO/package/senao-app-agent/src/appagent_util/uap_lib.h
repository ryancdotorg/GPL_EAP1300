#if __cplusplus
extern "C" {
#endif

#ifndef UAP_LIB_H
#define UAP_LIB_H
#include "ostypes.h"
#include "uaptype.h"


/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_IpAddrCheck
*-----------------------------------------------------------------------
* DESCRIPTION:
*   This function checks if the input IP address is legal or not.  A 
*   legal IP address must not have the following formats:
*   
*   (a) 0.0.0.0 		    -- This host
*   (b) 255.255.255.255 	-- Limit broadcast
*   (c) 127.xxx.xxx.xxx	    -- Loopback
*   (d) netid is 0 		    -- Host on this net
*   (e) hostid is all 1s 	-- Directed broadcast for net
*
* INPUT:
*   ipaddr  : The IP address to check
* RETURN:   
*   TRUE: 	The input IP address is a legal address.
*   FALSE:	Check fault, not a legal IP address.
*----------------------------------------------------------------------*/
UINT32  UapLib_IpAddrCheck(UINT32 ipaddr);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_IpNetCheck
*-----------------------------------------------------------------------
* DESCRIPTION:
*   This function checks if the input IP net is legal or not.  A 
*   legal IP net must not have the following formats:
*   
*   (a) 0.0.0.0 		    -- This host
*   (b) 255.255.255.255 	-- Limit broadcast
*   (c) 127.xxx.xxx.xxx	    -- Loopback
*   (e) hostid is all 1s 	-- Directed broadcast for net
*
* INPUT:
*   ipaddr  : The IP net to check
* RETURN:   
*   TRUE: 	The input IP net is a legal net.
*   FALSE:	Check fault, not a legal IP net.
*----------------------------------------------------------------------*/
UINT32  UapLib_IpNetCheck(UINT32 ip_net);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_NetmaskCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   This function check if the netmask is legal.  A legal IP net mask
*   must satisfy the following criteria:
*
*   (a) Must have consecutive on-bits (1) starting from the most
*       significant bit of the net mask value.
*   (b) When an off-bit (0) encountered, the rest least significant bits
*       must be all zeros(0).
*
* INPUT:
*   netmask     The netmask to check
* RETURN:
*   TRUE: 	    The input IP net mask is a legal address.
*   FALSE:	    Check fault, not a legal IP net mask.
*----------------------------------------------------------------------*/
UINT32  UapLib_NetmaskCheck(UINT32 netmask);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_SubnetCheck
*-----------------------------------------------------------------------
* DESCRIPTION:
*   Check if the ipaddr is okay when mask with the netmask.  A legal
*   subnet check must satisfy the following constraints:
*
*   (a) The rest bits after masking should not be all 0s.
*   (b) The rest bits after masking should not be all 1s.
*
* INPUT:    
*   ipaddr      : IP address to check
*   netmask     : Netmask to mask the ipaddr
* RETURN:
*   TRUE        : subnet mask okay
*   FALSE       : subnet mask fail
*----------------------------------------------------------------------*/
UINT32  UapLib_SubnetCheck(UINT32 ipaddr, UINT32 netmask);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_DefaultSubnet
*-----------------------------------------------------------------------
* DESCRIPTION:
*   This routine get the default subnet mask value based on the incoming
*   IP address:
*
* INPUT:    
*   ipaddr      : IP address to check
* RETURN:
*   (a) For A class IP address, return 0xff000000 (255.0.0.0)
*   (b) For B class IP address, return 0xffff0000 (255.255.0.0)
*   (c) For C class IP address, return 0xffffff00 (255.255.255.0)
*----------------------------------------------------------------------*/
UINT32  UapLib_DefaultSubnet(UINT32 ipaddr);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_GatewayCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   Check if the gateway is in the same subnet of ip_addr, or the 
*   gateway value is 0.  It is special for pSOS PNA, because it cannot
*   support class less IP net mask.
* 
* INPUT:    
*   ipaddr      : IP address to check
*   netmask     : Netmask to mask the ipaddr
*   gateway     : The gateway value
* RETURN:
*   TRUE        : gateway is 0, or in the same net of the ip address.
*   FALSE       : gateway is not in the same net of the ip address.
*----------------------------------------------------------------------*/
UINT32  UapLib_GatewayCheck(UINT32 ipaddr, UINT32 gateway,
                            UINT32 netmask);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_NonspacePrintableStringCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a printable string with no
*   space character inside.
* 
* INPUT:
*   string_P    : string to check
* RETURN:
*   TRUE        : The string is a printable string with no space 
*                 character inside.
*   FALSE       : Some non-printable characters or space characters
*                 inside.
*----------------------------------------------------------------------*/
UINT32  UapLib_NonSpacePrintableStringCheck(INT8 *pString);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_PrintableStringCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a printable string.                                   |
*
* INPUT:
*   string_P    : string to check
* RETURN:
*   TRUE        : The string is a printable string.
*   FALSE       : Some non-printable characters inside.
*----------------------------------------------------------------------*/
UINT32  UapLib_PrintableStringCheck(INT8 *pString);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_PrintableOctetStringCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a printable string.                                   |
*
* INPUT:
*   ostr_P      : octet string to check
* RETURN:
*   TRUE        : The string is a printable string.
*   FALSE       : Some non-printable characters inside.
*----------------------------------------------------------------------*/
UINT32  UapLib_PrintableOctetStringCheck(UAP_OCTET_T ostr);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_NumericStringCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a numeric string.                                   |
*
* INPUT:
*   string_P    : string to check
* RETURN:
*   TRUE        : The string is a numeric string.
*   FALSE       : Some non-numeric characters inside.
*----------------------------------------------------------------------*/
UINT32  UapLib_NumericStringCheck(INT8 *pString);

/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_MakeOidFromText
*-----------------------------------------------------------------------
* DESCRIPTION:     
*   Make an object id from a given text string.  The text string comply
*   with the following format, 1.3.6. ... .259.022....; The numbers must
*   be decimal
*
* INPUT:
*   oid_P       : Buffer to store the transferred OID.
*   text_P      : The object id tree traverse string.
* OUTPUT:
*   oid_P       : Object ID transferred.
* RETURN:
*   TRUE        : OK
*   FALSE       : Input format error
*----------------------------------------------------------------------*/
UINT32  UapLib_MakeOidFromText(UAP_OID_T *pOid, INT8 *pText);


UINT32 UapLib_Check_SameSubnet(UINT32 ip1, UINT32 ip2, UINT32 netmask) ; //cfho 1226

UINT32  UapLib_OctectStringCheck(INT8 *string);

#endif /* UAP_LIB_H */

#if __cplusplus
}
#endif



