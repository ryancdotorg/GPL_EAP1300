#if __cplusplus
extern	"C" {
#endif

#ifndef _ENVCFG_H
#define _ENVCFG_H

struct envfield 
{
	char *name;                 /* name of environmental variable      */
	char *value;                /* value of the environmental variable */
	struct envfield *next;      /* ptr to next envfield in linked list */
	struct envfield *prev;      /* ptr to previous envfield in linked list */
};
typedef struct envfield envfield;

struct envcfg_t
{
	envfield    *head;          /* ptr to first field in linked list */
	envfield    *tail;          /* ptr to last field in linked list  */
};
typedef struct envcfg_t envcfg_t;

/*-----------------------------------------------------------------------*/
/* Function Prototypes                                                   */
/*-----------------------------------------------------------------------*/
void    init_env(envcfg_t *);
int     free_env(envcfg_t *);
int     set_env(envcfg_t *, char *, char *);
char    *get_env(envcfg_t *, char *);

#endif //_ENVCFG_H

#if __cplusplus
}
#endif

