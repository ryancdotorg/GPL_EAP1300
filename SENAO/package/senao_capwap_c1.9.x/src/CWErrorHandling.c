/*******************************************************************************************
 * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	       *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.                *
 *                                                                                         *
 * You should have received a copy of the GNU General Public License along with this       *
 * program; if not, write to the:                                                          *
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,                    *
 * MA  02111-1307, USA.                                                                    *
 *                                                                                         *
 * --------------------------------------------------------------------------------------- *
 * Project:  Capwap                                                                        *
 *                                                                                         *
 * Author :  Ludovico Rossi (ludo@bluepixysw.com)                                          *
 *           Del Moro Andrea (andrea_delmoro@libero.it)                                    *
 *           Giovannini Federica (giovannini.federica@gmail.com)                           *
 *           Massimo Vellucci (m.vellucci@unicampus.it)                                    *
 *           Mauro Bisson (mauro.bis@gmail.com)                                            *
 *******************************************************************************************/


#include "CWCommon.h"

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWThreadSpecific gLastError;

CWBool CWErrorHandlingInitLib()
{
    if(!CWThreadCreateSpecific(&gLastError, NULL))
    {
        CWLog("Critical Error, closing the process...");
        return CW_FALSE;
    }

    return CW_TRUE;
}

CWBool _CWErrorRaise(CWErrorCode code, const char *fileName, int line, const char *msg, ...)
{
    CWErrorHandlingInfo *infoPtr;
    va_list args;

    infoPtr = CWThreadGetSpecific(&gLastError);
    if(infoPtr == NULL)
    {
        CW_CREATE_OBJECT_ERR(infoPtr, CWErrorHandlingInfo,
        {
        	CWLog("Out of Memory, closing the process...");
        	exit(EXIT_FAILURE);
        });

        if(!CWThreadSetSpecific(&gLastError, infoPtr))
        {
            CWLog("Critical Error, closing the process...");
            exit(EXIT_FAILURE);
        }
    }

    infoPtr->code = code;

    if(msg != NULL)
    {
        va_start(args, msg);

        vsnprintf(infoPtr->message, sizeof(infoPtr->message) - 1, msg, args);

        infoPtr->message[sizeof(infoPtr->message) - 1] = '\0';

        va_end(args);
    }
    else
    {
        infoPtr->message[0] = '\0';
    }

    if(fileName != NULL)
    {
        strncpy(infoPtr->fileName, fileName, sizeof(infoPtr->fileName) - 1);
        infoPtr->fileName[sizeof(infoPtr->fileName) - 1] = '\0';
    }
    else
    {
        infoPtr->fileName[0] = '\0';
    }

    infoPtr->line = line;

    return CW_FALSE;
}

void _CWErrorFree()
{
    CWErrorHandlingInfo *infoPtr;

    infoPtr = CWThreadGetSpecific(&gLastError);
    if(infoPtr)
    {
        CW_FREE_OBJECT(infoPtr);
        if(!CWThreadSetSpecific(&gLastError, NULL))
        {
            CWLog("Critical Error, closing the process...");
            exit(EXIT_FAILURE);
        }
    }
}

void CWErrorPrint(CWErrorHandlingInfo *infoPtr, const char *desc, const char *fileName, int line)
{
    if(infoPtr == NULL)
    {
        return;
    }

    if(infoPtr->message[0] != '\0')
    {
        CWLog("Error: %s. %s .", desc, infoPtr->message);
    }
    else
    {
        CWLog("Error: %s", desc);
    }
    CWLog("(occurred at line %d in file %s, catched at line %d in file %s).",
          infoPtr->line, infoPtr->fileName, line, fileName);
}

CWErrorCode CWErrorGetLastErrorCode()
{
    CWErrorHandlingInfo *infoPtr;

    infoPtr = CWThreadGetSpecific(&gLastError);

    if(infoPtr == NULL)
    {
        return CW_ERROR_NONE;
    }

    return infoPtr->code;
}

const char *CWErrorGetLastErrorMsg()
{
    CWErrorHandlingInfo *infoPtr;

    infoPtr = CWThreadGetSpecific(&gLastError);

    if(infoPtr == NULL)
    {
        return NULL;
    }

    return infoPtr->message;
}

CWBool _CWErrorHandleLast(const char *fileName, int line)
{
    CWErrorHandlingInfo *infoPtr;

    infoPtr = CWThreadGetSpecific(&gLastError);
    if(infoPtr == NULL)
    {
        CWLog("No Error Pending %s %u", fileName, line);
        return CW_FALSE;
    }

#define __CW_ERROR_PRINT(str)	CWErrorPrint(infoPtr, (str), fileName, line)

    switch(infoPtr->code)
    {
        case CW_ERROR_SUCCESS:
        case CW_ERROR_NONE:
            return CW_TRUE;
            break;

        case CW_ERROR_OUT_OF_MEMORY:
            __CW_ERROR_PRINT("Out of Memory");
            break;

        case CW_ERROR_WRONG_ARG:
            __CW_ERROR_PRINT("Wrong Arguments in Function");
            break;

        case CW_ERROR_NEED_RESOURCE:
            __CW_ERROR_PRINT("Missing Resource");
            break;

        case CW_ERROR_GENERAL:
            __CW_ERROR_PRINT("Error Occurred");
            break;

        case CW_ERROR_CREATING:
            __CW_ERROR_PRINT("Error Creating Resource");
            break;

        case CW_ERROR_SENDING:
            __CW_ERROR_PRINT("Error Sending");
            break;

        case CW_ERROR_RECEIVING:
            __CW_ERROR_PRINT("Error Receiving");
            break;

        case CW_ERROR_INVALID_FORMAT:
            __CW_ERROR_PRINT("Invalid Format");
            break;

        case CW_ERROR_NOT_SUPPORTED:
            __CW_ERROR_PRINT("Not Support");
            break;

        case CW_ERROR_OUT_OF_INDEX:
            __CW_ERROR_PRINT("Out of Index");
            break;

        case CW_ERROR_INTERRUPTED:
        default:
            break;
    }

    return CW_FALSE;
}

const char *CWErrorCodeString(CWErrorCode error)
{
    static char buf[16];
    static char *error_str[] =
    {
        "CW_ERROR_NONE",
        "CW_ERROR_SUCCESS",
        "CW_ERROR_OUT_OF_MEMORY",
        "CW_ERROR_WRONG_ARG",
        "CW_ERROR_INTERRUPTED",
        "CW_ERROR_NEED_RESOURCE",
        "CW_ERROR_COMUNICATING",
        "CW_ERROR_CREATING",
        "CW_ERROR_GENERAL",
        "CW_ERROR_OPERATION_ABORTED",
        "CW_ERROR_SENDING",
        "CW_ERROR_RECEIVING",
        "CW_ERROR_INVALID_FORMAT",
        "CW_ERROR_TIME_EXPIRED",
        "CW_ERROR_NOT_SUPPORTED",
        "CW_ERROR_OUT_OF_INDEX"
    };

    if(error >= sizeof(error_str) / sizeof(error_str[0]))
    {
        sprintf(buf, "Unknown(%d)", error);
        return buf;
    }
    return error_str[error];
}


