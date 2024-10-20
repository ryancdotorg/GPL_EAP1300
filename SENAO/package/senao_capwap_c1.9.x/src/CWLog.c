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
#define WRITE_STD_OUTPUT 1

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

static FILE *gLogFile = NULL;
static char gCurLogFileName[32];
static CWThreadMutex gFileMutex = CW_MUTEX_INITIALIZER;

CWBool CWLogInitFile(char *fileName)
{
    if(fileName == NULL)
    {
        return CW_FALSE;
    }

#ifdef CW_DEBUGGING
    int fileNo = 0;

    /* Find an unused fileNo to create the log file */
    while(1)
    {
        sprintf(gCurLogFileName, "%s.%d", fileName, fileNo);
        if((gLogFile = fopen(gCurLogFileName, "r")) == NULL)
        {
            break;
        }
        fclose(gLogFile);
        fileNo++;
    }
#else
    strcpy(gCurLogFileName, fileName);
#endif

    if((gLogFile = fopen(gCurLogFileName, "w")) == NULL)
    {
        printf("Can't open log file %s: %s", gCurLogFileName, strerror(errno));
        return CW_FALSE;
    }

	return CW_TRUE;
}

void CWLogCloseFile()
{
    CWDestroyThreadMutex(&gFileMutex);

    fclose(gLogFile);
    gLogFile = NULL;
}

void CWVLog(const char *format, va_list args)
{
    time_t now;
    struct tm timeInfo = {0};
    char timeStr[50] = {0};
    char fileLine[512] = {0};
    int len;
    int fileSize = 0;

    if(format == NULL)
    {
        return;
    }

    now = time(NULL);
    localtime_r(&now, &timeInfo);
    strftime(timeStr, sizeof(timeStr), "%F %T", &timeInfo);
    len = snprintf(fileLine, sizeof(fileLine), "%08x %s ", (unsigned int) CWThreadSelf(), timeStr);
    len += vsnprintf(&fileLine[len], sizeof(fileLine) - len - 1, format, args);
    fileLine[len] = '\n';

    if(gLogFile != NULL)
    {
        CWThreadMutexLock(&gFileMutex);

        if((fileSize = ftell(gLogFile)) == -1)
        {
            CWThreadMutexUnlock(&gFileMutex);
            return;
        }

        /* Reset file if the file size exceeds the limitation */
        if(fileSize >= gMaxLogFileSize)
        {
            fclose(gLogFile);
            if((gLogFile = fopen(gCurLogFileName, "w")) == NULL)
            {
                CWThreadMutexUnlock(&gFileMutex);
                return;
            }
        }

        fwrite(fileLine, strlen(fileLine), 1, gLogFile);
        fflush(gLogFile);

        CWThreadMutexUnlock(&gFileMutex);
    }

#ifdef WRITE_STD_OUTPUT
    printf("%s", fileLine);
#endif
}

void CWLog(const char *format, ...)
{
    va_list args;

    if(gEnabledLog)
    {
        va_start(args, format);

        CWVLog(format, args);

        va_end(args);
    }
}

void CWDebugLog(const char *format, ...)
{
#ifdef CW_DEBUGGING
    va_list args;

    if(gEnabledDebugLog)
    {
        va_start(args, format);

        CWVLog(format, args);

        va_end(args);
    }
#endif
}

void CWSystemCallLog(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    CWVLog(format, args);

    va_end(args);
}

