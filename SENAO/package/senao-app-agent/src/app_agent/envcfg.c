/*---------------------------------------------------------------------------------
GENERAL DESCRIPTION:
	This is a set of generic routines to work with a linked list of name = value pairs.
	Very effecient memory use.  A few ptrs are statically allocated with the envcfg
	struct, then as variables are added, memory is dynamically allocated for each
	string, and envfield required.  See documentation on how/when to use envcfg.
----------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <netmall.h>
#include "ostypes.h"
#include "envcfg.h"

#define MAX_FIELD_LEN   4096

/*---------------------------------------------------------------------------------
Name:   init_env
Description:    Initialization.  Set initial field values.   
Return: none
----------------------------------------------------------------------------------*/
void init_env(envcfg_t *envcfg)
{
	envcfg->head = NULL;
	envcfg->tail = NULL;
}

/*---------------------------------------------------------------------------------
Name:  free_env 
Description:  Go throught fields and release all memory allocated.  Verify deletion
	of all fields.
Return: 0 on success, -1 on failure
----------------------------------------------------------------------------------*/
int free_env(envcfg_t *envcfg)
{
    envfield *field;
    envfield *tmp;
	
	field = envcfg->head;
	while(field != NULL) 
	{
		tmp = field;


		field = field->next;
		if(tmp->name != NULL)       
			free(tmp->name);
		if(tmp->value != NULL)
			free(tmp->value);

		free(tmp);
	}

	return 0;
}


/*---------------------------------------------------------------------------------
Name:  set_env 
Description:    Initialize an environmental variable.  Verify size of name and value
	are within limits, then allocate memory for each component.  If variable is already
	defined, an error message is displayed and then returns -1.  

Return: 0 on success, error code on failure
----------------------------------------------------------------------------------*/
int set_env(envcfg_t *envcfg, char *name, char *value)
{
    envfield *field;

	/* check for strings too long */
	if ((strlen(name) >= MAX_FIELD_LEN) || (strlen(value) >= MAX_FIELD_LEN))
		return -1;

	field = (envfield *)malloc(sizeof(struct envfield));    /* allocate a new field */
	if(field == NULL)
		return 1;

	if(strlen(name))
		field->name = (char *)malloc(strlen(name) + 1);     /* allocate memory for the field name  */
	else
		field->name = NULL;

	if(field->name == NULL)
    {
		free(field);
		return 2;
	}

	field->value = (char *)malloc(strlen(value) + 1);   /* allocate memory for the field value */
	if(field->value == NULL)
    {
		free(field->name);
		free(field);
		return 3;
	}

	strcpy(field->name, name);
	strcpy(field->value, value);

	/* set pointers to reflect new env variable in linked list */
	if(envcfg->head == NULL)
    {
        /* this must be the first in the list */
		envcfg->head = field;
		field->prev = NULL;
	}
	else
    {
        /* there is at least one env var defined */
		field->prev = envcfg->tail;
		envcfg->tail->next = field;
	}

	envcfg->tail = field;       /* always set the tail to the new env variable */
	field->next = NULL;         /* always set the new field next ptr to NULL   */

    return 0;
}


/*---------------------------------------------------------------------------------
Name:  get_env 
Description:  Search through environmental variables for givin name.  Get value of
	environmental variable specified in name.   
Return: ptr to value if found, or NULL if not found
----------------------------------------------------------------------------------*/
char *get_env(envcfg_t *envcfg, char *name)
{
    envfield *field;

	field = envcfg->head;
	while(field != NULL)
    {
		if (strcmp(field->name, name) == 0)
			return field->value;    /* found value */

		field = field->next;   
	}

	return NULL;  /* not found */
}

