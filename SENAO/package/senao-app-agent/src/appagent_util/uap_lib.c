
#include <string.h>
#include "uap_lib.h"
#include <ctype.h>
#include <stdlib.h>
#include "ostypes.h"

//#define strlen(s) strlen ((char*)s)
//#define strcpy(a,b) strcpy((char*)a, (char*)b)
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
UINT32  UapLib_IpAddrCheck(UINT32 addr)
{
    register UINT32  ipaddr = addr;
    register UINT32  netid;
    register UINT32  hostid;
    
    /************************************************/
    /* The following ip address is not legal:       */
    /* 0.0.0.0 (This host),                         */
    /* 255.255.255.255 (Limit broadcast)            */
    /* 127.xxx.xxx.xxx (Loopback)                   */
    /* netid is 0 (Host on this net)                */
    /* hostid is all 1s (Directed broadcast for net */
    /************************************************/
    
    if ((ipaddr & 0x80000000) == 0)         /* Check class A and loopback */
    {
        netid = ipaddr & 0x7f000000;
        hostid = ipaddr & 0x00ffffff;
        
        if (netid == 0x7f000000 || netid == 0 ||
            hostid == 0x00ffffff || hostid == 0)
        {
            return (FALSE);
        }
    }
    else if ((ipaddr & 0xc0000000) == 0x80000000)   /* Check class B */
    {
        netid = ipaddr & 0x3fff0000;
        hostid = ipaddr & 0x0000ffff;
        
        if (netid == 0 ||
            hostid == 0x0000ffff || hostid == 0)
        {
            return (FALSE);
        }
    }
    else if ((ipaddr & 0xe0000000) == 0xc0000000)   /* Check class C */
    {
        netid = ipaddr & 0x1fffff00;
        hostid = ipaddr & 0x000000ff;
        
        if (netid == 0 ||
            hostid == 0x000000ff || hostid == 0)
        {
            return (FALSE);
        }
    }
    else  
    {
        return (FALSE);     /* Limit broadcast, Multicast net */
    }
    
    return (TRUE);
}


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
UINT32  UapLib_IpNetCheck(UINT32 net)
{
    register UINT32  ip_net = net;
    register UINT32  netid;
    register UINT32  hostid;
    
    /************************************************/
    /* The following ip net is not legal:           */
    /* 0.0.0.0 (This host),                         */
    /* 255.255.255.255 (Limit broadcast)            */
    /* 127.xxx.xxx.xxx (Loopback)                   */
    /* hostid is all 1s (Directed broadcast for net */
    /************************************************/
    
    if ((ip_net & 0x80000000) == 0)         /* Check class A and loopback */
    {
        netid = ip_net & 0x7f000000;
        hostid = ip_net & 0x00ffffff;
        
        if (netid == 0x7f000000 || hostid == 0x00ffffff)
        {
            return (FALSE);
        }
    }
    else if ((ip_net & 0xc0000000) == 0x80000000)   /* Check class B */
    {
        hostid = ip_net & 0x0000ffff;
        
        if (hostid == 0x0000ffff)
        {
            return (FALSE);
        }
    }
    else if ((ip_net & 0xe0000000) == 0xc0000000)   /* Check class C */
    {
        hostid = ip_net & 0x000000ff;
        
        if (hostid == 0x000000ff)
        {
            return (FALSE);
        }
    }
    else  
    {
        return (FALSE);     /* Limit broadcast, Multicast net */
    }
    
    return (TRUE);
}


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
UINT32  UapLib_NetmaskCheck(UINT32 mask)
{
    register UINT32  netmask = mask;
    register int     i;
    register UINT32  j = 0xffffffff;
    
    for (i = 0; i <= 32; i++)
    {
        if (netmask == j)
        {
            return (TRUE);
        }

        j <<= 1;
    }   
    return (FALSE);
}


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
UINT32  UapLib_SubnetCheck(UINT32 addr, UINT32 netmask)
{
    register UINT32 ipaddr = addr;
    register UINT32 host_mask = ~netmask;
    register UINT32 host_id = ipaddr & host_mask;

    /* Check if ALL 0 or ALL 1 */
    if (host_mask != 0 &&           // Skip host route        
        (host_id == host_mask ||    // Subnet broadcast
         host_id == 0))             // This subnet
    {
        return (FALSE);
    }  
    return (TRUE);
}


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
UINT32  UapLib_DefaultSubnet(UINT32 addr)
{
    register UINT32 ipaddr = addr;
    
    if ((ipaddr & 0x80000000) == 0)         /* Check class A and loopback */
    {
        return 0xff000000;
    }
    else if ((ipaddr & 0xc0000000) == 0x80000000)   /* Check class B */
    {
        return 0xffff0000;
    }
    else if ((ipaddr & 0xe0000000) == 0xc0000000)   /* Check class C */
    {
        return 0xffffff00;
    }
    
    return 0;   /* Default no subnet mask */ 
}


/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_GatewayCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   Check if the gateway is in the same subnet of ip_addr, or the 
*   gateway value is 0.  
* 
* INPUT:    
*   ipaddr      : IP address to check
*   netmask     : Netmask to mask the ipaddr
*   gateway     : The gateway value
* RETURN:
*   TRUE        : gateway is 0, or in the same net of the ip address.
*   FALSE       : gateway is not in the same net of the ip address.
*----------------------------------------------------------------------*/
UINT32 UapLib_GatewayCheck(UINT32 addr, UINT32 gw, UINT32 mask)
{
    register UINT32 ipaddr = addr;
    register UINT32 gateway = gw;
    register UINT32 netmask = mask;

    if (gateway != 0)
    {
        if ((ipaddr & netmask) != (gateway & netmask))
        {
            return FALSE;
        }
    }  
    return (TRUE);
}


/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_NonspacePrintableStringCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a printable string with no
*   space character inside.
* 
* INPUT:
*   pString    : string to check
* RETURN:
*   TRUE        : The string is a printable string with no space 
*                 character inside.
*   FALSE       : Some non-printable characters or space characters
*                 inside.
*----------------------------------------------------------------------*/
UINT32  UapLib_NonSpacePrintableStringCheck(INT8 *string)
{
    register INT8 *pString = string;
    register int i, j, len;

    if (pString == 0)
        return FALSE;
 
    len = strlen(pString);
    for (i = 0; i < len; i++)
    {
        j = pString[i];
        if (j < 33 || j > 126)
        {
            return FALSE;
        }
    }
    return TRUE;   
}


/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_PrintableStringCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a printable string.                                   |
*
* INPUT:
*   pString    : string to check
* RETURN:
*   TRUE        : The string is a printable string.
*   FALSE       : Some non-printable characters inside.
*----------------------------------------------------------------------*/
UINT32  UapLib_PrintableStringCheck(INT8 *string)
{
    register INT8 *pString = string;
    register int i, j, len;

    if (pString == 0)
    {
        return FALSE;
    }

    len = strlen(pString);
    for (i = 0; i < len; i++)
    {
        j = pString[i];
        if (j < 32 || j > 126)
        {
            return FALSE;
        }
    }
    return TRUE;   
}


/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_PrintableOctetStringCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a printable string.                                   |
*
* INPUT:
*   ostr        : octet string to check
* RETURN:
*   TRUE        : The string is a printable string.
*   FALSE       : Some non-printable characters inside.
*----------------------------------------------------------------------*/
UINT32  UapLib_PrintableOctetStringCheck(UAP_OCTET_T ostr)
{
    register INT8 *pString = ostr.pString;
    register int i, j, len;

    if (pString == 0)
    {
        return FALSE;
    }

    len = ostr.len;
    for (i = 0; i < len; i++)
    {
        j = pString[i];
        if (j < 32 || j > 126)
        {
            return FALSE;
        }
    }
    return TRUE;   
}


/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_NumericStringCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a numeric string.                                   |
*
* INPUT:
*   pString    : string to check
* RETURN:
*   TRUE        : The string is a numeric string.
*   FALSE       : Some non-numeric characters inside.
*----------------------------------------------------------------------*/
UINT32  UapLib_NumericStringCheck(INT8 *string)
{
    register INT8 *pString = string;
    register int i, j, len;

    if (pString == 0)
    {
        return FALSE;
    }

    len = strlen(pString);
    if (pString[0] == '0' && (pString[1] == 'x' || pString[1] == 'X'))
    {
        // Check 
        for (i = 2; i < len; i++)
        {
            j = pString[i];
            if ((j < '0' || j > '9') &&
                (j < 'A' || j > 'F') &&
                (j < 'a' || j > 'f'))
            {
                return TRUE;
            }
        }
    }
    else
    {
        for (i = 0; i < len; i++)
        {
            j = pString[i];

            if (i == 0 && j == '-')
                continue;

            if (j < '0' || j > '9')
            {
                return FALSE;
            }
        }
    }

    return TRUE;   
}


/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_HexByteCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a numeric string.
*
* INPUT:
*   pString    : string to check
* RETURN:
*   TRUE        : The string is a numeric string.
*   FALSE       : Some non-numeric characters inside.
*----------------------------------------------------------------------*/
INT32   UapLib_AsciiToHexByte(INT8 *string)
{
    char c1, c2;

    if (string == 0)
        return (-1);

    if (string[2] != 0)
        return (-1);

    c1 = string[0];
    c2 = string[1];
    if (!isxdigit(c1) || !isxdigit(c2))
        return (-1);

    c1 = tolower(c1);
    c2 = tolower(c2);

    c1 = (c1 < 'a')? (c1 - '0') : (c1 - 'a' + 10);
    c2 = (c2 < 'a')? (c2 - '0') : (c2 - 'a' + 10);

    return (c1 * 16 + c2);   
}


/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_IpStringCheck
*-----------------------------------------------------------------------
* DESCRIPTION: 
*   The routine checks if the input string is a numeric string. 
*
* INPUT:
*   pString    : string to check
* RETURN:
*   TRUE        : The string is a numeric string.
*   FALSE       : Some non-numeric characters inside.
*----------------------------------------------------------------------*/
UINT32  UapLib_IpStringCheck(INT8 *pString)
{
    char buf[20];
    register char *working = buf;
    register char *start = buf;
    register char ch;
    register int i, j;

    if (strlen(pString) > 15)
        return FALSE;

    strcpy(buf, pString);

    // Parsing here
    for (i=0, j=0;; working++)
    {
        ch = *working;

        // Check delimeter
        if (ch == '.' || ch == 0)
        {
            *working = 0;       // Null ended
            if (strlen(start) == 0 || atoi(start) > 255)
                return FALSE;

            working++;
            start = working;
            i = 0;

            if (++j >= 4)
            {
                if (ch == '.')
                    return FALSE;
                else
                    return TRUE;
            }
        }

        // Check numbric range
        if (*working < '0' || *working > '9')
            return FALSE;

        // Check whether more than three digit
        if (++i > 3)
            return FALSE;
    }

    return FALSE;   
}


/*----------------------------------------------------------------------
* ROUTINE NAME - UapLib_MakeOidFromText
*-----------------------------------------------------------------------
* DESCRIPTION:     
*   Make an object id from a given text string.  The text string comply
*   with the following format, 1.3.6. ... .259.022....; The numbers must
*   be decimal
*
* INPUT:
*   pOid       : Buffer to store the transferred OID.
*   pText      : The object id tree traverse string.
* OUTPUT:
*   pOid       : Object ID transferred.
* RETURN:
*   TRUE        : OK
*   FALSE       : Input format error
*----------------------------------------------------------------------*/
UINT32  UapLib_MakeOidFromText(UAP_OID_T *pOid, INT8 *pText )
{
    register UINT8   *pTemp;
    register UINT32  i, j, dot_count;
    register long    ival;

    /* */
    /* Allocate the object id tree traverse */
    /* array                                */
    /* */
    for (i = 0, dot_count = 0; pText[i] != '\0'; i++)
    {
        if (pText[i] == '.')
        {
            dot_count++;    /* get the dot number   */
        }   /* in the test string   */
    }
    

    if (pOid->num_components < dot_count + 1)
    {
        return FALSE;
    }

    if (pOid->component_list == 0)
    {
        return FALSE;
    }

    /* */
    /* Parse the object id in the test string       */
    /* Can accept hex, oct and decimal format       */
    /* */

    pOid->num_components = dot_count + 1;
    for (i = 0, pTemp =(UINT8 *) pText; i < dot_count + 1; i++)
    {
        /* */
        /* Now check the decimal number.  The   */
        /* first character should be digit.     */
        /* */
        if (isdigit(*pTemp))
        {
            ival = 0;
            for (j = 0; j < 11 && *pTemp != '.' && *pTemp != '\0'; j++)
            {
                if (!isdigit(*pTemp))
                {
                    return FALSE;
                }

                ival = (ival * 10) + ((*pTemp - '0') & 0x0f);
                pTemp++;
            }
            pOid->component_list[i] = ival;
        }
        else
        {
            return FALSE;
        }

        /* */
        /* Next character expacted is a delimiter(.) or */
        /* end of the text string.                      */
        /* */
        switch (*pTemp)
        {
            case '.':
                pTemp++;

            case '\0':
                break;

            default:
                return FALSE;
        }
    }

    return (TRUE);
}

//cfho 1226
//check whether ip1 and ip2 are in the same subnet
UINT32 UapLib_Check_SameSubnet(UINT32 ip1, UINT32 ip2, UINT32 netmask) 
{
       if ((ip1 & netmask) != (ip2 & netmask))
        {
            return FALSE;
        }  
    return TRUE;
}

//cfho 03-0108
UINT32  UapLib_OctectStringCheck(INT8 *string)
{
    register INT8 *pString = string;
    register int i, j, len;

    if (pString == 0)
    {
        return FALSE;
    }

    len = strlen(pString);
    
        for (i = 0; i < len; i++)
        {
            j = pString[i];

            if (i == 0 && j == '-')
                continue;

            if ((j < '0' || j > '9') &&
                (j < 'A' || j > 'F') &&
                (j < 'a' || j > 'f'))
            {
                return FALSE;
            }

  	}

    return TRUE;   
}
