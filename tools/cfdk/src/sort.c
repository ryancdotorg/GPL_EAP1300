/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort.h"
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
#define MAX_LINE_SIZE  2048
#define MAX_TABLE_SIZE 3072
/*****************************************************************
 * NAME: comp
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int comp (const void * elem1, const void * elem2) {
    int f = *((char*)elem1);
    int s = *((char*)elem2);
    int i = 0;

    while (f == s)
    {
        i++;
        f=*((char*)(elem1+i));
        s=*((char*)(elem2+i));
        if(f=='\0' || s=='\0')
        {
            break;
        }
    }
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}
/*****************************************************************
 * NAME: compNum
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int compNum (const void * elem1, const void * elem2) {
    int f = atoi((char*)elem1);
    int s = atoi((char*)elem2);

    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

/*****************************************************************
 * NAME: sorttable
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int sorttable(char *filename, int type) {
    FILE *fp_in, *fp_out;
    int i=0, cnt=0;
    char hugetable[MAX_TABLE_SIZE][MAX_LINE_SIZE]={0};
    char buf[MAX_LINE_SIZE]={0};
    char tmpname[64]={0};


    if((fp_in = fopen(filename, "r"))==NULL)
    {
        printf("file open error[%s]\n",fp_in);
        return -1;
    }
    strcpy(tmpname, filename);
    strcat(tmpname, "_sort_tmp");
    while(fgets(buf, sizeof(buf), fp_in))
    {
        strcpy(hugetable[cnt], buf);
        cnt++;
    }
    fclose(fp_in);


    if(type==1)
    {
        qsort(hugetable, cnt, sizeof(*hugetable), compNum);
    }
    else
    {
        qsort(hugetable, cnt, sizeof(*hugetable), comp);
    }

    if((fp_out = fopen(tmpname, "w"))==NULL)
    {
        return -1;
    }
    for (i = 0 ; i < cnt ; i++)
    {
        fprintf(fp_out, "%s", hugetable[i]);
    }
    fclose(fp_out);

    remove(filename);
    rename(tmpname, filename);

    return 0;
}
