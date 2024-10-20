#include "CWWTP.h"

static CWThreadMutex g2gSitesurveyMutex = CW_MUTEX_INITIALIZER;
static CWThreadCondition g2gSitesurveyCond = CW_COND_INITIALIZER;
static unsigned char g2gChanUtil[CW_WIRELESS_RADIO_2G_UTILIZATION_FIELD_SIZE];

static CWThreadMutex g5gSitesurveyMutex = CW_MUTEX_INITIALIZER;
static CWThreadCondition g5gSitesurveyCond = CW_COND_INITIALIZER;
static unsigned char g5gChanUtil[CW_WIRELESS_RADIO_5G_UTILIZATION_FIELD_SIZE];

static CWThreadMutex g5gOneSitesurveyMutex = CW_MUTEX_INITIALIZER;
static CWThreadCondition g5gOneSitesurveyCond = CW_COND_INITIALIZER;
static unsigned char g5gOneChanUtil[CW_WIRELESS_RADIO_5G_UTILIZATION_FIELD_SIZE];



typedef struct {
    const char *radioName;
    CWRadioFreqType radioType;
    CWThreadMutex *mutex;
    CWThreadCondition *cond;
    CWThread thread;
    CWBool enable;
    volatile CWBool doing;
    unsigned int interval;
    unsigned char *chanUtil;
} CWSitesurveyArg;

static CWSitesurveyArg gSiteSurveyArg[] =
{
    {
        .radioName = "2G",
        .radioType = CW_RADIOFREQTYPE_2G,
        .mutex = &g2gSitesurveyMutex,
        .cond = &g2gSitesurveyCond,
        .enable = CW_FALSE,
        .doing = CW_FALSE,
        .interval = CW_SITESURVEY_INTERVAL_DEFAULT,
        .chanUtil = g2gChanUtil,
    },
    {
        .radioName = "5G",
        .radioType = CW_RADIOFREQTYPE_5G,
        .mutex = &g5gSitesurveyMutex,
        .cond = &g5gSitesurveyCond,
        .enable = CW_FALSE,
        .doing = CW_FALSE,
        .interval = CW_SITESURVEY_INTERVAL_DEFAULT,
        .chanUtil = g5gChanUtil,
    }
    ,
    {
        .radioName = "5G1",
        .radioType = CW_RADIOFREQTYPE_5G_1,
        .mutex = &g5gOneSitesurveyMutex,
        .cond = &g5gOneSitesurveyCond,
        .enable = CW_FALSE,
        .doing = CW_FALSE,
        .interval = CW_SITESURVEY_INTERVAL_DEFAULT,
        .chanUtil = g5gOneChanUtil,
    }
};

static CWBool SendSitesurveyEvent(CWWTPSitesurveyInfo *sitesurveyInfo)
{
    CWProtocolMessage *msgElems;

    CW_CREATE_PROTOCOL_MSG_ARRAY_ERR(msgElems,
                                     1,
                                     return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(!CWWTPAssembleMsgElemSitesurveyInfo(&(msgElems[0]), sitesurveyInfo))
    {
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    if(!CWWTPSendPendingRequest(CW_MSG_TYPE_VALUE_WTP_EVENT_REQUEST, CWGetSeqNum(), msgElems, 1))
    {
        CW_FREE_PROTOCOL_MESSAGE(msgElems[0]);
        CW_FREE_OBJECT(msgElems);
        return CW_FALSE;
    }

    return CW_TRUE;
}

static CW_THREAD_RETURN_TYPE CWWTPSitesurveyTask(void *arg)
{
    CWSitesurveyArg *sitesurveyArg = (CWSitesurveyArg*) arg;
    struct timespec timeout;
    int radioIdx;
    int firstChannel, currentChannel;
    CWBool firstScan = CW_TRUE;
    CWWTPSitesurveyInfo sitesurveyInfo;

    CWDebugLog("CWWTPSitesurveyTask %s enter", sitesurveyArg->radioName);

    CWWTPGetRadioIndex(sitesurveyArg->radioType, &radioIdx);

    CWThreadMutexLock(sitesurveyArg->mutex);

    while(sitesurveyArg->doing)
    {
        CWDebugLog("[%s] waiting %d seconds for sitesurvey scan", sitesurveyArg->radioName, sitesurveyArg->interval);

        /* wait for next sitesurvey */
        timeout.tv_sec = time(NULL) + sitesurveyArg->interval; /* seconds */
        timeout.tv_nsec = 0; /* nanoseconds */

        CWWaitThreadConditionTimeout(sitesurveyArg->cond, sitesurveyArg->mutex, &timeout);
        if(!sitesurveyArg->doing)
        {
            break;
        }
/* airodump
        if(firstScan)
        {
            if(CWWTPBoardGetNextFastScanChannel(radioIdx, &firstChannel) ==  CW_TRUE)
            {
                CWDebugLog("[%s] sitesurvey first channel %d", sitesurveyArg->radioName, firstChannel);

                if(!CWWTPBoardSetFastScanDurationTime(radioIdx, sitesurveyArg->interval))
                {
                    CWLog("*** failed to set %s fast scan duration time", sitesurveyArg->radioName);
                    CWErrorHandleLast();
                    continue;
                }

                CWDebugLog("[%s] fast scan duration time: %u", sitesurveyArg->radioName, sitesurveyArg->interval);

                if(!CWWTPBoardSetScanDwellTime(radioIdx, 50, 60))
                {
                    CWLog("*** failed to set %s fast scan Dwell time", sitesurveyArg->radioName);
                    CWErrorHandleLast();
                    continue;
                }

                CWDebugLog("[%s] fast scan Dwell time: 50 60", sitesurveyArg->radioName);

                firstScan = CW_FALSE;
                currentChannel = firstChannel;
            }
            else
            {
                CWLog("*** failed to get next fast scanned %s channel", sitesurveyArg->radioName);
                CWErrorHandleLast();
                continue;
            }
        }
*/
        CW_ZERO_MEMORY(&sitesurveyInfo, sizeof(CWWTPSitesurveyInfo));
        sitesurveyInfo.radio = sitesurveyArg->radioType;

/* airodump
        if(!CWWTPBoardGetScan(CW_FALSE, &sitesurveyInfo))
        {
            CWLog("*** %s sitesurvey on channel %d is failed",  sitesurveyArg->radioName, currentChannel);
            CWErrorHandleLast();
            continue;
        }

        if(!CWWTPBoardGetChannelUtilization(sitesurveyArg->radioType, currentChannel, sitesurveyArg->chanUtil))
        {
            CWLog("*** %s get channel utiliation on channel %d is failed", sitesurveyArg->radioName, currentChannel);
            CWErrorHandleLast();
            continue;
        }
*/
        // Send sitesurvey info to AC only when finish 1 cycle of sitesurvey
        if(!CWWTPBoardGetScan(CW_TRUE, &sitesurveyInfo) ||
           !CWWTPBoardGetRadioCurrentChannel(radioIdx, &sitesurveyInfo.curChannel) ||
           !CWWTPBoardGetRadioCurrentTxPower(radioIdx, &sitesurveyInfo.curTxPower))
        {
            CWErrorHandleLast();
        }
        else
        {
            sitesurveyInfo.chanUtil = sitesurveyArg->chanUtil;
            SendSitesurveyEvent(&sitesurveyInfo);
        }

        CW_FREE_OBJECT(sitesurveyInfo.info);
    }

    CWThreadMutexUnlock(sitesurveyArg->mutex);

    CWDebugLog("CWWTPSitesurveyTask %s exit", sitesurveyArg->radioName);

    CWErrorFree();

    return (void *) 0;
}

void CWWTPStartSitesurvey(CWRadioFreqType radioType)
{
    int radioIdx, channel;

    if(gSiteSurveyArg[radioType].enable && !gSiteSurveyArg[radioType].doing && CWWTPCheckAnySSIDEnable(radioType))
    {
        gSiteSurveyArg[radioType].doing = CW_TRUE;

        if(!CWErr(CWCreateThread(&gSiteSurveyArg[radioType].thread, CWWTPSitesurveyTask, &gSiteSurveyArg[radioType])))
        {
            CWLog("Error starting sitesurvey thread");
            exit(EXIT_FAILURE);
        }

        if(CWWTPGetRadioIndex(radioType, &radioIdx) &&
           CWWTPBoardGetRadioChannelCfg(radioIdx, &channel) &&
           channel == CW_RADIO_CHANNEL_AUTO)
        {
            CWWTPBoardSetRadioAutoChannelSelectionACS(radioIdx, CW_TRUE);
        }
    }
}

void CWWTPStopSitesurvey(CWRadioFreqType radioType)
{
    int radioIdx;

    if(gSiteSurveyArg[radioType].doing)
    {
        gSiteSurveyArg[radioType].doing = CW_FALSE;

        CWSignalThreadCondition(gSiteSurveyArg[radioType].cond);

        /* wait for the end of thread */
        CWThreadJoin(gSiteSurveyArg[radioType].thread);

        if(CWWTPGetRadioIndex(radioType, &radioIdx))
        {
            CWWTPBoardSetRadioAutoChannelSelectionACS(radioIdx, CW_FALSE);
        }
    }
}

CWBool CWWTPCheckSitesurveyDoing(CWRadioFreqType radioType)
{
    return gSiteSurveyArg[radioType].doing;
}

CWBool CWWTPCheckSitesurveyEnable(CWRadioFreqType radioType)
{
    return gSiteSurveyArg[radioType].enable;
}

void CWWTPEnableBackgroundSitesurvey(CWRadioFreqType radioType, CWBool enable)
{
    int radioIdx;

    if(CWWTPGetRadioIndex(radioType, &radioIdx))
    {
        gSiteSurveyArg[radioType].enable = enable;
    }
}

void CWWTPSetBackgroundSitesurveyInterval(CWRadioFreqType radioType, unsigned int interval)
{
    int radioIdx;

    if(CWWTPGetRadioIndex(radioType, &radioIdx))
    {
        gSiteSurveyArg[radioType].interval = interval;
    }
}

void CWWTPStartAllSitesurvey()
{
    int radioIdx;
    CWRadioFreqType radioType;

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        if(CWWTPGetRadioType(radioIdx, &radioType) && gSiteSurveyArg[radioType].enable)
        {
            CWWTPStartSitesurvey(radioType);
        }
    }
}

void CWWTPStopAllSitesurvey()
{
    int radioIdx;
    CWRadioFreqType radioType;

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        if(CWWTPGetRadioType(radioIdx, &radioType) && gSiteSurveyArg[radioType].enable)
        {
            CWWTPStopSitesurvey(radioType);
        }
    }
}

