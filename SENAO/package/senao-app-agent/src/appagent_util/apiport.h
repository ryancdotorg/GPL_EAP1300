#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h> 
#include <sys/types.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h> 
#include "ostypes.h"

//for message queue

enum KNL_WAIT_E
{
    KNL_WAIT = 1,
    KNL_NOWAIT
};

#define OPTION int
#define STATUS int
#define QU_QUEUE_ID 0x51554555UL
#define NU_QUEUE_SIZE 18

//#define KNL_MSGQ_ID int

//int APCfg_Sem;

typedef struct NU_QUEUE_STRUCT
{
    UINT16 words[NU_QUEUE_SIZE];
}NU_QUEUE;


typedef struct knl_massageq_t
{
    NU_QUEUE msgQ; // Pointer to queue block
    void    *start_P;   //  start address for queue
}KNL_MSGQ_DATA;

