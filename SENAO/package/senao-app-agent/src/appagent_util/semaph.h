#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/sem.h>

#include	<errno.h> 
extern int	errno;

//functions
int sem_create(key_t	key, int	initval);
int sem_open(key_t	key);
int sem_rm(int	id);
int sem_close(int	id);
int sem_wait(int	id);
int sem_signal(int	id);
int sem_op(int	id,int	value);
