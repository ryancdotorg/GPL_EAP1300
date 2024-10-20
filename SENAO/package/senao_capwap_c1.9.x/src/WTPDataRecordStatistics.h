#ifndef __CAPWAP_WTPRecordStatisics_HEADER__
#define __CAPWAP_WTPRecordStatisics_HEADER__

void CWWTPStartStats(void);
void CWWTPStopStats();
void CWWTPHaltStats(CWBool enable);
void CWWTPSetStatsUploadInterval(int interval);
void CWWTPSetStatsPollInterval(int interval);
void CWWTPSetClientChangeEventEnable(CWBool enable);
void CWWTPSetStatsMaxClients(int maxClients);

#endif

