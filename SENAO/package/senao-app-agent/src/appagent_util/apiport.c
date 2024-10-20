#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apiport.h"
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/sysinfo.h> //cfho 2006-0419
#include "semaph.h"
#include "knlintf.h"

key_t getkey(char *name_P)
{
	int key=0;
	int i;
	for(i=0;i<strlen(name_P);i++)
	{
		key+=name_P[i];
	}

	//printf("getkey () gen key %d for [%s]\n",key,name_P);
	return key;
}

#define PERMS	0666
#define MAX_MSGQLEN 255

#define err_sys(x) ({ fprintf(stderr, "%s\n", (x)); return -1; })
#define err_dump(s) printf(s); printf("\n");

/***************************************************************************/
//port by cfho 03-0304
int KNL_SEM_CREATE(char *name_P, KNL_SEM_ID *ppsem_id, int type)
{
	int semid;
	if( (semid = sem_create(getkey(name_P), 1))< 0)
	{
		printf("KNL_SEM_CREATE(), cannot create sem!\n");
		return ERROR;
	}
	*ppsem_id=semid;
	return OK;
}
/***************************************************************************/
//port by cfho 03-0304
int KNL_SEM_DELETE(KNL_SEM_ID  sem_id)
{
  return (sem_close(sem_id)==0) ? OK : ERROR;

}
/***************************************************************************/
//port by cfho 03-0304
int KNL_SEM_LOCK(KNL_SEM_ID sem_id)
{
   return (sem_wait(sem_id)==0) ? OK : ERROR;
}
/***************************************************************************/
//port by cfho 03-0304
int KNL_SEM_UNLOCK(KNL_SEM_ID sem_id)
{

   return (sem_signal(sem_id)==0) ? OK : ERROR;
}
KNL_TICK KNL_TICKS()
{
#if 1 //cfho 2006-0418 rewrite using sysinfo, i.e. /proc/uptime
    struct sysinfo info;
    
    
    if (sysinfo(&info)==0)
    {
        return (info.uptime);
    }
    
    return 0;
    
    
#else      
    struct timeval t;
    
    gettimeofday(&t,0);

    return (t.tv_sec);
#endif
}
/***************************************************************************/

