/*****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project :
;    Creator : Yolin
;    File    :
;    Abstract:
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       Yolin            2014-0408
;*****************************************************************************/
/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "sort.h"
#if SUPPORT_WEB_DEF_CONFIG
#include "webDefConfig.h"
#endif
#if SUPPORT_WEB_CONFIG
#include "webConfig.h"
#endif
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
#define WEB_CGI_FOLDER          "web_cgi"
#define WEB_SCR_FOLDER          "web_scr"
#define WEB_SCR_MINI_FOLDER     "web_scr_html"

// TOP dir:win32/web_html
// we move win32/web_cgi forder to win32/ forder by igetweb.sh
#define WEB_RES_TABLE_FILE      WEB_CGI_FOLDER"/web_res_table.c"
#define WEB_SUBMIT_TABLE_FILE   WEB_CGI_FOLDER"/web_submit_table.c"
#define WEB_DISP_TABLE2_FILE    WEB_CGI_FOLDER"/web_disp_table2.c"
#define WEB_DISP_TABLE_FILE     WEB_CGI_FOLDER"/web_disp_table.c"
#define _WEB_RES_TABLE_FILE     WEB_CGI_FOLDER"/_web_res_table.c"
#define _WEB_SUBMIT_TABLE_FILE  WEB_CGI_FOLDER"/_web_submit_table.c"
#define _WEB_DISP_TABLE_FILE    WEB_CGI_FOLDER"/_web_disp_table.c"
#define _DUP_FILE               WEB_CGI_FOLDER"/dupcheck"

#define CGI_LIST_FILE           "../.cgilist"
#define SCR_LIST_FILE           "../.scrlist"
#define LANG_LIST_FILE_NAME     ".langlist"
#define LANG_LIST_FILE          "../"LANG_LIST_FILE_NAME
#define LANG_LIST_FILE_FORCE    "../langlist"
#define LANG_DATABASE_FILE      "../langdb"

#define OPT_TYPE_CGI (1<<0)
#define OPT_TYPE_DATA (1<<1)

#define MAX_LINE_BUFF_SIZE 2048
#define MAX_FIELNAME_SIZE 64

//20130607 Jason: add Debug define for print debug msg.
#define PRINT_DEBUG 0
#if PRINT_DEBUG
#define CFDK_DEBUG(fmt, args...) printf(fmt, ## args)
#else
#define CFDK_DEBUG(fmt, args...)
#endif

/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/
enum
{
    //0 is default value, donot use;
    CGI_TYP_NONE=0,
    CGI_TYP_STR,
    CGI_TYP_INT,
    CGI_TYP_IPA,
    CGI_TYP_DEFINE
};

int gentype=0;
int pageperm=1;
char homepage[64]={0};

typedef struct fileinfo_  /* input:web_html/index.htm */
{
    char filename[MAX_FIELNAME_SIZE];    /* index */
    char dirname[MAX_FIELNAME_SIZE];     /* web_html */
    char cginame[MAX_FIELNAME_SIZE];     /* web_cgi/web_index_htm.c */
    char hname[MAX_FIELNAME_SIZE];       /* web_cgi/h/web_index.h */
    char scrname[MAX_FIELNAME_SIZE];     /* web_scr/webindex_htm.c */
    char scrmininame[MAX_FIELNAME_SIZE]; /* web_scr_html/index.htm */
    char dataname[MAX_FIELNAME_SIZE];    /* web_cgi/webf_index_htm.c */
    char comname[MAX_FIELNAME_SIZE];     /* index_htm */
} fileinfo_t;
fileinfo_t fileinfo;

typedef struct queryname_
{
    char *name;
    struct queryname_ *next;
} queryname_t;

queryname_t *pHead=NULL;
queryname_t *pItem=NULL;
queryname_t *pPrev=NULL;
queryname_t *pTemp=NULL;

/*****************************************************************
 * NAME: genNames
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int genNames(char *inputname)
{
    char *ep=NULL, *sp=NULL, *tp=NULL;
    char tmp[MAX_LINE_BUFF_SIZE]={0};
    char postname[MAX_FIELNAME_SIZE]={0};

    strcpy(tmp, inputname);
    sp=tp=tmp;
    while(sp=strchr(tp,'/'))
    {
        tp=sp+1;
    }

    ep = tp = tmp;
    ep += strlen(tmp);
    if(strchr(tp, '.'))
    {
        while(*ep != '.') ep--;
        strcpy(postname, ep+1);
        *ep='\0';
    }
    else
    {
        strcpy(postname, "");
    }
    //get .XXX
    //printf("----->%s\n",tmp);

    memset(fileinfo.cginame, 0, sizeof(fileinfo.cginame));
    memset(fileinfo.hname, 0, sizeof(fileinfo.hname));
    memset(fileinfo.scrname, 0, sizeof(fileinfo.scrname));
    memset(fileinfo.scrmininame, 0, sizeof(fileinfo.scrmininame));
    memset(fileinfo.dataname, 0, sizeof(fileinfo.dataname));
    memset(fileinfo.dirname, 0, sizeof(fileinfo.dirname));

    tp=tmp;
    sp = ep;
    while(*sp != '/' && sp!=tp) sp--;
    if(*sp == '/')
    {
        *sp='\0';
        sp++;
        strcpy(fileinfo.dirname, tp);
    }
    strcpy(fileinfo.filename, sp);
    strcpy(fileinfo.cginame, WEB_CGI_FOLDER"/");
    strcpy(fileinfo.hname, WEB_CGI_FOLDER"/h/");
    strcpy(fileinfo.scrmininame, WEB_SCR_MINI_FOLDER"/");
    strcpy(fileinfo.scrname, WEB_SCR_FOLDER"/");
    strcpy(fileinfo.dataname, WEB_CGI_FOLDER"/");
    strcat(fileinfo.cginame, "web_");
    strcat(fileinfo.scrname, "web");
    if(gentype & OPT_TYPE_DATA)
    {
        strcat(fileinfo.hname, "webf_");
    }
    else
    {
        strcat(fileinfo.hname, "web_");
    }
    strcat(fileinfo.dataname, "webf_");
    if(strlen(fileinfo.dirname))
    {
        if(gentype & OPT_TYPE_DATA)
        {
            strcat(fileinfo.hname, fileinfo.dirname);
            strcat(fileinfo.hname, "_");
        }
        strcat(fileinfo.scrname, fileinfo.dirname);
        strcat(fileinfo.scrname, "_");
        strcat(fileinfo.scrmininame, fileinfo.dirname);
        strcat(fileinfo.scrmininame, "/");
        strcat(fileinfo.dataname, fileinfo.dirname);
        strcat(fileinfo.dataname, "_");
    }
    strcat(fileinfo.cginame, fileinfo.filename);
    strcat(fileinfo.hname, fileinfo.filename);
    strcat(fileinfo.scrname, fileinfo.filename);
    strcat(fileinfo.scrmininame, fileinfo.filename);
    strcat(fileinfo.dataname, fileinfo.filename);
    if(strlen(postname))
    {
        strcat(fileinfo.scrmininame, ".");
        strcat(fileinfo.scrmininame, postname);
        strcat(fileinfo.scrname, "_");
        strcat(fileinfo.scrname, postname);   // txt or htm or ...
        strcat(fileinfo.dataname, "_");
        strcat(fileinfo.dataname, postname);  // txt or htm or ...
        if(gentype & OPT_TYPE_DATA)           //h file add postfix
        {
            strcat(fileinfo.hname, "_");
            strcat(fileinfo.hname, postname); // txt or htm or ...
        }
    }
    strcat(fileinfo.cginame, ".c");
    strcat(fileinfo.hname, ".h");
    strcat(fileinfo.scrname, ".c");
    strcat(fileinfo.dataname, ".c");
    memset(fileinfo.comname, 0 ,sizeof(fileinfo.comname));
    if(strlen(fileinfo.dirname))
    {
        strcat(fileinfo.comname,fileinfo.dirname);
        strcat(fileinfo.comname,"_");
    }
    strcat(fileinfo.comname,fileinfo.filename);
    strcat(fileinfo.comname,"_");
    strcat(fileinfo.comname,postname);

    CFDK_DEBUG("cginame:%s\n", fileinfo.cginame);
    CFDK_DEBUG("hname:%s\n", fileinfo.hname);
    CFDK_DEBUG("scrname:%s\n", fileinfo.scrname);
    CFDK_DEBUG("scrmininame:%s\n", fileinfo.scrmininame);
    CFDK_DEBUG("dataname:%s\n", fileinfo.dataname);
    CFDK_DEBUG("comname:[%s] \n\n", fileinfo.comname);

    return 0;
}
/*****************************************************************
 * NAME: getHiddenValue
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int getHiddenValue(char *string, char *hname, char *hvalue)
{
    char tmp[MAX_LINE_BUFF_SIZE]={0};
    char *sp=NULL,*ep=NULL,*tp=NULL;

    strcpy(tmp, string);
    if(tp=strstr(tmp, "name"))
    {
        if(sp=strchr(tp,'\"'))
        {
            sp=sp+1;
            tp=sp;
            if(ep=strchr(tp, '\"'))
            {
                *ep='\0';
                strcpy(hname, sp);
            }
        }
    }
    strcpy(tmp, string);
    if(tp=strstr(tmp, "value"))
    {
        if(sp=strchr(tp,'\"'))
        {
            sp=sp+1;
            tp=sp;
            if(ep=strchr(tp,'\"'))
            {
                *ep='\0';
                strcpy(hvalue, sp);
            }
        }
    }

}
/*****************************************************************
 * NAME: skipType
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author: Yolin
 ****************************************************************/
int skipType(char *string)
{
    int type=0;

    if(!strncmp("STR", string, 3))
    {
        type=CGI_TYP_STR;
    }
    else if(!strncmp("IPA", string, 3))
    {
        type=CGI_TYP_IPA;
    }
    else if(!strncmp("INT", string, 3))
    {
        type=CGI_TYP_INT;
    }
#if SUPPORT_WEB_DEF_CONFIG
    else if(!strncmp("define", string, 6))
    {
        type=CGI_TYP_DEFINE;
    }
#endif
    return type;
}
#if SUPPORT_WEB_DEF_CONFIG
/*****************************************************************
 * NAME: addEscape
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author: Yolin
 ****************************************************************/
int addEscape(char *srcStr, char* destStr)
{
    int len;
    int i;
    int cnt = 0;
    int ret = 0;

    // protect
    if(!destStr)
    {
        return 0;
    }

    // protect
    if(srcStr == NULL || (len = strlen(srcStr))==0)
    {
        destStr[0] = '\0';
        return 0;
    }

    for(i=0; i<=len; i++)
    {
        if(srcStr[i] == '\"' || srcStr[i] == '\\')
        {
            ret++;
            destStr[cnt++] = '\\';
        }
        destStr[cnt++] = srcStr[i];
    }
    return ret;
}
/*****************************************************************
 * NAME: checkWebConfigTable
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author: Yolin
 ****************************************************************/
#define WEB_DEF_CONFIG_TOTOAL_COUNT (sizeof(webDefConfig)/sizeof(webDefConfig[0]))
int checkWebConfigTable(char *filename, char *funcname, char *targetString)
{
    int i=0;
    int find = 0;

#if SUPPORT_WEB_CONFIG
#define WEB_CONFIG_TOTOAL_COUNT (sizeof(webConfig)/sizeof(webConfig[0]))
    for (i=0;i<WEB_CONFIG_TOTOAL_COUNT;i++)
    {
        if (strcmp(funcname, webConfig[i].defineName) == 0)
        {
            if(targetString)
            {
                strcpy(targetString, webConfig[i].targetString);
            }
            find=1;
            break;
        }
    }
#endif

#if SUPPORT_WEB_CONFIG
    if(!find)
#endif
    {
        for (i=0;i<WEB_DEF_CONFIG_TOTOAL_COUNT;i++)
        {
            if (strcmp(funcname, webDefConfig[i].defineName) == 0)
            {
                if(targetString)
                {
                    strcpy(targetString, webDefConfig[i].targetString);
                }
                find=1;
                break;
            }
        }
    }

    if(!find)
    {
        printf("\nError: %s[%s] not declared in webDefConfig.h or webConfig.h\n", filename, funcname);
    }
    return find;
}
#endif
/*****************************************************************
 * NAME: noDupWrite
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author: Yolin
 ****************************************************************/
int noDupWrite(int type, char *tmpfile, char *inputstring, char *filename)
{
    FILE *fp=NULL;
    char buf[MAX_LINE_BUFF_SIZE]={0};
    char *line;
    char *sp=NULL, *ep=NULL;
    int dup=0, len=0;
#if SUPPORT_WEB_DEF_CONFIG
    if(type==CGI_TYP_DEFINE)
    {
        //Yolin: if use "#define XXXXX#", we needn't genCGI and genResTable
        dup = 1;
        return dup;
    }
#endif

    if((fp = fopen(tmpfile, "r"))==NULL)
    {
        return -1;
    }

    while(fgets(buf, sizeof(buf), fp))
    {
        dup=0;
        sp=buf;
        len=strlen(buf);
        buf[len-1]='\0';
        if(filename)
        {
            ep=strchr(buf, ':');
            *ep='\0';
        }
        if(!strcmp(sp, inputstring))
        {
            dup=1;
            break;
        }
    }
    fclose(fp);

    if(!dup)
    {
        if((fp = fopen(tmpfile, "a+"))==NULL)
        {
            return -1;
        }
        //update dupcheck file
        if(filename)
        {
            fprintf(fp, "%s:%s\n", inputstring, filename);
        }
        else
        {
            fprintf(fp, "%s\n", inputstring);
        }
        fclose(fp);
    }
    return dup;
}
/*****************************************************************
 * NAME: genWEB
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author: Yolin
 ****************************************************************/
int genWEB(char *filename, char *input, char *output, int isReal, int escape)
{
    char *sp=NULL, *ep=NULL, *tp=NULL, *end=NULL;
#if SUPPORT_WEB_DEF_CONFIG
    char _replaceString[MAX_LINE_BUFF_SIZE]={0};
    char replaceString[MAX_LINE_BUFF_SIZE]={0};
#endif
    char funcname[MAX_LINE_BUFF_SIZE]={0};
    char buffer[MAX_LINE_BUFF_SIZE]={0};
    int escapeCount=0;

    memset(output, 0, sizeof(output));
    strcpy(buffer, input);
    sp = ep = buffer;
    end = sp + strlen(buffer);

    while(ep <= end)
    {
        if(escape && *ep=='\\')
        {
            *ep='\0';
            strcat(output, sp);
            strcat(output, "\\\\");
            escapeCount++;
            sp=ep+1;
        }
        else if(escape && *ep=='\"')
        {
            *ep='\0';
            strcat(output, sp);
            strcat(output, "\\\"");
            escapeCount++;
            sp=ep+1;
        }
        else if((!strncmp(ep, "#STR ", 5)) || (!strncmp(ep, "#INT ", 5)) || (!strncmp(ep, "#IPA ", 5)))
        {
            *ep='\0';
            strcat(output, sp);
            strcat(output, "#");
            ep+=4;
            sp=ep+1;
            for(sp; isspace(*sp); sp++)
            {
                ep++;
            }
        }
#if SUPPORT_WEB_DEF_CONFIG
        else if(isReal && (!strncmp(ep, "#define ", 8)))
        {
            *ep='\0';
            strcat(output, sp);
            sp=ep+7;
            for(sp; isspace(*sp); sp++)
                ;
            tp=ep=strchr(sp, '#');
            *tp='\0';
            for(ep;ep>=sp;ep--)
            {
                if(*ep == '(')
                {
                    *ep='\0';
                }
            }
            strcpy(funcname, sp);
            if(checkWebConfigTable(filename, funcname, replaceString))
            {
                if(escape)
                {
                    escapeCount += addEscape(replaceString, _replaceString);
                    strcat(output, _replaceString);
                }
                else
                {
                    strcat(output, replaceString);
                }
            }
            else
            {
                strcat(output, "Error!");
            }
            ep=tp;
            sp=ep+1;
        }
#endif
        ep++;
    }
    strcat(output, sp);
    return escapeCount;
}
/*****************************************************************
 * NAME: genCGI
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int genCGI(char *inputstr, int type, int index)
{

    FILE *fp=NULL;
    char typestr[64]={0};

    switch(type)
    {
        case CGI_TYP_STR:
            strcpy(typestr, "char");
            break;
        case CGI_TYP_IPA:
            strcpy(typestr, "UINT32");
            break;
        default://case CGI_TYP_INT:
            strcpy(typestr, "INT32");
            break;
    }

    /* gen cgi file */
    if((fp = fopen(fileinfo.cginame, "a"))==NULL)
    {
        return -1;
    }

    fprintf(fp, "UINT32 web_%s(%s *ival, envcfg_t *envcfg%s)\n", inputstr, typestr, index==1?", INT32 index1":"");
    fprintf(fp, "{\n");
    fprintf(fp, "\treturn 0;");
    fprintf(fp, "\n}\n\n");
    fclose(fp);

    /* gen header file */
    if((fp = fopen(fileinfo.hname, "a"))==NULL)
    {
        return -1;
    }
    fprintf(fp, "UINT32 web_%s(%s *ival, envcfg_t *envcfg%s);\n", inputstr, typestr, index==1?", INT32 index1":"");
    fclose(fp);

    return 0;
}
/*****************************************************************
 * NAME: genResTable
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int genResTable(char *inputstr, int type, int index)
{
    FILE *fp=NULL;
    char funcstr[MAX_LINE_BUFF_SIZE]={0};
    char typestr[64]={0};
    //{"GetColorInfo",              CGI_TYP_STR,        0,  (P_UAP_FUNC_T)&web_GetColorInfo},
    if((fp = fopen(_WEB_RES_TABLE_FILE, "a"))==NULL)
    {
        return -1;
    }
    strcpy(funcstr, inputstr);
    strcat(funcstr,"\",");
    switch(type)
    {
        case CGI_TYP_STR:
            strcpy(typestr, "CGI_TYP_STR");
            break;
        case CGI_TYP_IPA:
            strcpy(typestr, "CGI_TYP_IPADDR");
            break;
        default:
            strcpy(typestr, "CGI_TYP_INT");
            break;
    }
    fprintf(fp, "\t{\"%-27s\t%s,      \t%d,\t(P_UAP_FUNC_T)&web_%s},\n",funcstr,typestr,index,inputstr);
    fclose(fp);

    return 0;
}
/*****************************************************************
 * NAME: genDispTable
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int genDispTable(char *filename)
{
    FILE *fp=NULL;

    if((fp = fopen(_WEB_DISP_TABLE_FILE, "a"))==NULL)
    {
        return -1;
    }

    if(strlen(homepage))
    {
        fprintf(fp, "{\"/\", &cgi_pFileData_%s, 1, &cgi_nFileLen_%s, %d},\n",fileinfo.comname, fileinfo.comname, 0-pageperm);
    }

    if(gentype & OPT_TYPE_DATA)
    {
        fprintf(fp, "{\"/%s\", &cgi_pFileData_%s, 3, &cgi_nFileLen_%s, %d, (CGI_FILE_FUNC)&web_%s, 0, 0x4fa0f9db},\n",
                filename, fileinfo.comname, fileinfo.comname, pageperm, fileinfo.comname);
    }
    else
    {
        fprintf(fp, "{\"/%s\", &cgi_pFileData_%s, 1, &cgi_nFileLen_%s, %d, 0, 0, 0x4fa0f9db},\n",
                filename, fileinfo.comname, fileinfo.comname, pageperm);
    }
    fclose(fp);

}
/*****************************************************************
 * NAME: genSubmitTable
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int genSubmitTable(char *submitname)
{
    FILE *fp=NULL;
    char outputstr[MAX_LINE_BUFF_SIZE]={0};
    int index=0;

    if((fp = fopen(_WEB_SUBMIT_TABLE_FILE, "a"))==NULL)
    {
        return -1;
    }
    //{"arpproxy",                      2 ,     &web_submit_arpproxy},
    strcpy(outputstr, submitname);
    strcat(outputstr,"\",");

    if(strlen(homepage))
    {
        index=1;
    }
    else
    {
        index=pageperm;
    }

    fprintf(fp, "\t{\"%-32s%d ,\t\t&web_submit_%s},\n", outputstr, index, submitname);
    fclose(fp);

    return 0;
}
/*****************************************************************
 * NAME: genTableHeader
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
void genTableHeader(int length)
{
    FILE *fp = NULL;
    char tmp[MAX_FIELNAME_SIZE]={0};
    //gen res table and restable_tmp

    if((fp = fopen(WEB_RES_TABLE_FILE, "a"))==NULL)
    {
        return;
    }
    if(gentype & OPT_TYPE_CGI)
    {
        fprintf(fp, "#include <web_%s.h>\n",fileinfo.filename);
    }
    fclose(fp);


    if((fp = fopen(WEB_SUBMIT_TABLE_FILE, "a"))==NULL)
    {
        return;
    }
    if(gentype & OPT_TYPE_CGI)
    {
        fprintf(fp, "#include <web_%s.h>\n",fileinfo.filename);
    }
    fclose(fp);

    //gen disp table extern
    if((fp = fopen(WEB_DISP_TABLE_FILE, "a"))==NULL)
    {
        return;
    }
    fprintf(fp, "extern unsigned long cgi_nFileLen_%s;\n",fileinfo.comname);
    fprintf(fp, "extern char *cgi_pFileData_%s;\n",fileinfo.comname);
    if(gentype & OPT_TYPE_DATA)
    {
        fprintf(fp, "extern int web_%s();\n",fileinfo.comname);
    }
    fclose(fp);

    //gen disp table 2
    if((fp = fopen(WEB_DISP_TABLE2_FILE, "a"))==NULL)
    {
        return;
    }
    fprintf(fp, "unsigned long cgi_nFileLen_%s = %d;\n",fileinfo.comname, (gentype & OPT_TYPE_DATA)?1:length);

    sscanf(fileinfo.scrmininame, WEB_SCR_MINI_FOLDER"%s", tmp);
    fprintf(fp, "char *cgi_pFileData_%s = \"%s\";\n", fileinfo.comname, (gentype & OPT_TYPE_DATA)?" ":tmp);
    if(gentype & OPT_TYPE_DATA)
    {
        fprintf(fp, "extern int web_%s();\n",fileinfo.comname);
    }
    fclose(fp);
}
/*****************************************************************
 * NAME: isInt
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author: Yolin
 ****************************************************************/
int isInt(const char *str)
{
    if (!*str)
        return 0;

    while (*str)
    {
        if (!isdigit(*str))
            return 0;
        else
            ++str;
    }

    return 1;
}
/*****************************************************************
 * NAME: parsingFile
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author: Yolin
 ****************************************************************/
void parsingFile(char *filename)
{
    FILE *fin=NULL, *fout=NULL, *fmini=NULL;
    char *sp=NULL, *ep=NULL, *tp=NULL, *end=NULL;
    char buf[MAX_LINE_BUFF_SIZE]={0};
    char buf2[MAX_LINE_BUFF_SIZE]={0};
    char lowbuf[MAX_LINE_BUFF_SIZE]={0};
    char funcname[MAX_LINE_BUFF_SIZE]={0};
    char tmp[MAX_LINE_BUFF_SIZE]={0};
    char miniLine[MAX_LINE_BUFF_SIZE]={0};
    char scrLine[MAX_LINE_BUFF_SIZE]={0};

    char hiddenname[64]={0};
    char hiddenvalue[64]={0};
    char submitname[64]={0};

    int fakeflag = 0, realflag = 0, formflag = 0, cgiflag = 0;
    int miniTotalChar=0, scrTotalChar=0, scrSpecialChar=0;
    int index=0, type=CGI_TYP_NONE;

    genDispTable(filename);

    if(gentype & OPT_TYPE_CGI)
    {
        fout = fopen(fileinfo.cginame, "w+");
        if(!fout)
        {
            return;
        }
        fprintf(fout, "#include <stdlib.h>\n");
        fprintf(fout, "#include <stdio.h>\n");
        fprintf(fout, "#include <string.h>\n");
        fprintf(fout, "#include <cgi.h>\n");
        fprintf(fout, "#include <web_submit_table.h>\n");
        fprintf(fout, "#include <%s%s.h>\n\n", "web_", fileinfo.filename);
        fclose(fout);
    }

    /* gen scr prefix */
    fout = fopen(fileinfo.scrname, "w");
    if(!fout)
    {
        return;
    }
    fprintf(fout, "char *cgi_pFileData_%s =\n", fileinfo.comname);

    if(gentype & OPT_TYPE_CGI)
    {
        fmini = fopen(fileinfo.scrmininame, "w");
        if(!fmini)
        {
            fclose(fout);
            return;
        }
    }
    /* end scr prefix */

    if(gentype & OPT_TYPE_DATA)
    {
        fprintf(fout, "\"\\x031\"");
    }
    else
    {
        /* gen scr file*/
        fin = fopen(filename, "r");
        if(!fin)
        {
            fclose(fout);
            fclose(fmini);
            return;
        }

        /* start parsing*/
        while(fgets(buf2, sizeof(buf2), fin))
        {
            //20140210 Jason: Fix index bug for web_IODATA_EN cfdk.
            index=0;//reset

            // buf2 just use here.
            sp = tp = buf2;
            for(sp; isspace(*sp); sp++)
                ; /* skip white space */
            if (ep = strchr(buf2, '\r'))
            {
                *ep = '\0';
            }
            if (ep = strchr(buf2, '\n'))
            {
                *ep = '\0';
            }
            // end buf2 just use here.
            strcpy(buf, sp);
            /* check have form? */
            if (tp = strstr(buf, "<form"))
            {
                formflag++;
            }
            if(strlen(buf))
            {
                /* ignore fake*/
                if (!(strncmp(buf, "/*--$fake*/", 11) && strncmp(buf, "<!--$fake-->", 12)))
                {
                    fakeflag ++;
                    continue;
                }
                if (!(strncmp(buf, "/*--$endfake*/", 14) && strncmp(buf, "<!--$endfake-->", 15)))
                {
                    fakeflag --;
                    continue;
                }

                // language list parsing
                int i=0;
                sp=buf;

                while (*sp != '\0') {
                    lowbuf[i] = tolower(*sp);
                    sp++;
                    i++;
                }
                lowbuf[i]='\0';
                sp = strstr(lowbuf, "showtext(");
                while (sp != NULL)
                {
                    if(ep = strchr(sp+9, ')'))
                    {
                        *ep = '\0';
                        if(isInt(sp+9))
                        {
                            if(noDupWrite(CGI_TYP_NONE, LANG_LIST_FILE, sp+9, NULL)==0)
                            {
                                //Debug
                                //printf("[%s][%s]\n", sp+9, filename);
                            }
                        }
                        else
                        {
                            printf("\nwarning: showtext(x), x SHOULD integer [%s][%s]\n", sp+9, filename);
                        }
                        tp=ep+1;
                    }
                    else
                    {
                        printf("error break\n");
                        break;
                    }
                    sp = strstr(tp, "showtext(");
                }
                // end language list parsing

                if (!(strncmp(buf, "<input", 6)))
                {
                    memset(hiddenname, 0, sizeof(hiddenname));
                    memset(hiddenvalue, 0, sizeof(hiddenvalue));
                    getHiddenValue(buf, hiddenname, hiddenvalue);
                    if(strcmp(hiddenname,"page")==0)
                    {
                        memset(submitname, 0, sizeof(submitname));
                        strcpy(submitname, hiddenvalue);
                    }
                    else
                    {
                        int findsame=0;
                        pTemp=pHead;
                        while(pTemp)
                        {
                            if(!strcmp(pTemp->name,hiddenname))
                            {
                                findsame=1;
                                break;
                            }
                            pTemp=pTemp->next;
                        }

                        if(!findsame)
                        {
                            pItem=(queryname_t *) malloc(sizeof(queryname_t));
                            pItem->name=(char *) malloc(sizeof(char) * (strlen(hiddenname) + 1));
                            pItem->next=NULL;
                            strcpy(pItem->name, hiddenname);
                            if(!pHead)
                            {
                                pPrev=pHead=pItem;
                            }
                            else
                            {
                                pPrev->next=pItem;
                            }
                        }
                    }
                }
                // <!--$real>
                // #IPA duplicateip()#
                // <!--$endreal-->
                if (!(strncmp(buf, "/*--$real", 9)) || !(strncmp(buf, "<!--$real>", 10)))
                {
                    realflag ++;
                }
                if (!(strncmp(buf, "*//*$endreal*/", 14)) || !(strncmp(buf, "<!--$endreal-->", 15)))
                {
                    realflag --;
                }
                if((fakeflag == 0))
                {
                    //1. generate scr_mini (web_scr_html)
                    if(gentype & OPT_TYPE_CGI)
                    {
                        genWEB(filename, buf, miniLine, realflag, 0);
                        miniTotalChar += fprintf(fmini, "%s\n", miniLine);
                    }
                    //2. generate scr (web_scr)
                    scrSpecialChar += genWEB(filename, buf, scrLine, realflag, 1);
                    scrTotalChar += strlen(scrLine) + 1;
                    fprintf(fout, "\"%s\\n\"\n", scrLine);
                }
                if (realflag == 1)
                {
                    if(!(strncmp(buf, "$loop(", 6)))//loop
                    {
                        //Format 1.$loop(websitesurveyGetItemInit(),int i=2; i < webGetsitesurveyMax(); i++)
                        //Format 2.$loop(int i=2; i < webGetsitesurveyMax(); i++)
                        //check loop init
                        //$loop(int i=1; i <= webGetStDHCPListMax(); i++)
                        //#STR webGetStDHCPListSTR(i)#
                        //$endloop
                        sp = strchr(buf, '(');
                        sp++;
                        for(sp; isspace(*sp); sp++)
                            ;
                        strcpy(funcname, sp);
                        ep = sp = funcname;
                        //Format 1
                        if(ep = strchr(funcname, ','))
                        {
                            for(ep; *ep!='('; ep--)
                                ;
                            *ep = '\0';
                            if(type = skipType(sp))
                            {
#if SUPPORT_WEB_DEF_CONFIG
                                if(type == CGI_TYP_DEFINE)
                                {
                                    sp+=6;
                                }
                                else
#endif
                                {
                                    sp+=3;
                                }
                                for(sp;isspace(*sp);sp++)
                                    ;
                            }
                            strcpy(funcname, sp);
                            if(noDupWrite(type, _DUP_FILE, funcname, filename)==0)
                            {
                                genCGI(funcname, type, 0);
                                genResTable(funcname, type, 0);
                                cgiflag++;
                            }
                        }
                        //Format 2
                        /*check loop limit function*/
                        if(!ep)
                            ep = funcname;
                        tp = ep+1;
                        if(strtok(tp, ";"))
                        {
                            if(sp = strtok(NULL, ";><= "))
                            {
                                if(sp = strtok(NULL, ";><= ("))
                                {
                                    /*
                                       20140210 Jason: fix if $loop format is follow(for web_IODATA_EN cfdk):
                                       $loop(int i=1; i <= webGetWLANRADIOSSIDNUM(0); i++)
                                       */
                                    tp = strtok(NULL, ");");
                                    if(strlen(tp)==1 && strcmp(tp, " ")!=0)
                                    {
                                        index=1;
                                    }
                                    else
                                    {
                                        index=0;
                                    }

                                    if(type = skipType(sp))
                                    {
#if SUPPORT_WEB_DEF_CONFIG
                                        if(type == CGI_TYP_DEFINE)
                                        {
                                            sp+=6;
                                        }
                                        else
#endif
                                        {
                                            sp+=3;
                                        }
                                        for(sp;isspace(*sp);sp++)
                                            ;
                                    }
                                    strcpy(funcname, sp);
                                    if(noDupWrite(type, _DUP_FILE, funcname, filename)==0)
                                    {
                                        genCGI(funcname, CGI_TYP_INT, index);
                                        genResTable(funcname, CGI_TYP_INT, index);
                                        cgiflag++;
                                    }
                                }
                            }
                        }
                        //restable
                    } //end parsing loop
                    else if(sp = strchr(buf, '#')) //standard
                    {
                        //var lan_info={dhcpS:'#webGetAddrValue("dhcpd_start_1")#',dhcpE:'#webGetAddrValue("dhcpd_end_1")#',isDhcpEnable:'#webGetStrValue("dhcpd_enable")#'};
                        sp++;
                        while(ep = strchr(sp, '#'))
                        {
                            tp = sp;
                            for(tp;tp<=ep;tp++)
                            {
                                if(*tp == '(')
                                {
                                    *tp = '\0';
                                    tp++;
                                    for(tp;isspace(*tp);tp++)
                                        ;
                                    if(*tp==')')
                                    {
                                        index=0;
                                    }
                                    else
                                    {
                                        index=1;
                                    }
                                    break;
                                }
                            }
                            if(type = skipType(sp))
                            {
                                if(type == CGI_TYP_DEFINE)
                                {
                                    sp+=6;
                                }
                                else
                                {
                                    sp+=3;
                                }
                                for(sp;isspace(*sp);sp++)
                                    ;
                            }
                            memset(funcname, 0, sizeof(funcname));
                            strcpy(funcname, sp);
                            if(noDupWrite(type, _DUP_FILE, funcname, filename)==0)
                            {
                                genCGI(funcname, type, index);
                                genResTable(funcname, type, index);
                                cgiflag++;
                            }
                            ep++;
                            sp = strchr(ep, '#');
                            if(!(sp))
                                break;
                            else
                                sp++;
                        }
                    } // end parsing starand
                    else if (*buf == '$' && *(buf+1) != '.') //special format $aaa=aaa(), and skip $.ajax
                    {
                        tp=buf;
                        if(ep = strstr(tp, "()"))
                        {
                            *ep='\0';
                            sp=ep-1;
                            while(*sp != '=')
                            {
                                sp--;
                                if(sp==tp)
                                    break;
                            }
                            sp++;
                            for(sp;isspace(*sp);sp++)
                                ;
                            if(sp)
                            {
                                strcpy(funcname, sp);
                                if(noDupWrite(type, _DUP_FILE, funcname, filename)==0)
                                {
                                    genCGI(funcname, CGI_TYP_INT, index);
                                    genResTable(funcname, CGI_TYP_INT, index);
                                    cgiflag++;
                                }
                            }
                        }
                    }
                } //realflage=1
            }
        } //end read file

        if(fin)
            fclose(fin);
    }

    if(fmini)
        fclose(fmini);

    genTableHeader(miniTotalChar);
    /* gen scr postfix */
    fprintf(fout, ";\n\n");
    fprintf(fout, "unsigned long cgi_nFileLen_%s = %d;\n", fileinfo.comname, (gentype & OPT_TYPE_DATA)?1:scrTotalChar - scrSpecialChar);
    fclose(fout);
    /* end scr postfix */

    if((fout = fopen(SCR_LIST_FILE, "a"))==NULL)
    {
        return;
    }
    sscanf(fileinfo.scrname, WEB_SCR_FOLDER"/%s", tmp);
    if(tp=strstr(tmp,".c"))
    {
        *(tp+1)='o';
    }
    fprintf(fout, "%s\n", tmp);
    fclose(fout);
    //end scrlist

    if (gentype & OPT_TYPE_CGI)
    {
        if(formflag==0)
        {
            CFDK_DEBUG("[no form]\n");
            fout = fopen(fileinfo.hname, "a"); //h
            //int web_pictures_off2_fig(char *path, int *dflag);
            fprintf(fout, "int web_%s(char *path, int *dflag);\n", fileinfo.comname);
            if(!fout)
                return;
            fclose(fout);
            //remove(fileinfo.cginame);
            //remove(fileinfo.hname);
        }
        else
        {
            genSubmitTable(submitname);
            fout = fopen(fileinfo.cginame, "a");
            fprintf(fout, "int web_submit_%s(int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery, char **error)\n", submitname);
            fprintf(fout, "{\n");
            while(pHead)
            {
                fprintf(fout, "    INT8 *p%s = get_env (envQuery, \"%s\");\n", pHead->name, pHead->name);
                pTemp=pHead;
                pHead=pHead->next;
                free(pTemp);
            }
            fprintf(fout, "\n#define send_error(code)        {*error=code; return CGI_SUBMIT_REQUEST_RELOAD;}\n\n");
            fprintf(fout, "\n\treturn CGI_SUBMIT_OK;");
            fprintf(fout, "\n}");
            if(!fout)
                return;
            fclose(fout);
            fout = fopen(fileinfo.hname, "a"); //h
            //int web_submit_index( int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery, char **error);
            fprintf(fout, "int web_submit_%s(int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery, char **error);\n", submitname);
            if(!fout)
                return;
            fclose(fout);

        }
    }
    if (gentype & OPT_TYPE_DATA)
    {
        fout = fopen(fileinfo.hname, "a"); //h
        //int web_pictures_off2_fig(char *path, int *dflag);
        fprintf(fout, "int web_%s(char *path, int *dflag);\n", fileinfo.comname);
        if(!fout)
            return;
        fclose(fout);
        /*gen data's cgi file*/
        fout = fopen(fileinfo.dataname, "w");

        //int web_submit_index( int sock, int auth, envcfg_t *envcfg, envcfg_t *envQuery, char **error);
        fprintf(fout, "#include <stdlib.h>\n");
        fprintf(fout, "#include <stdio.h>\n");
        fprintf(fout, "#include <string.h>\n");
        fprintf(fout, "#include <cgi.h>\n");
        fprintf(fout, "#include <%s%s.h>\n","webf_",fileinfo.comname);
        fprintf(fout, "\n");
        fprintf(fout, "int web_%s(char *path, int *dflag)\n", fileinfo.comname);
        fprintf(fout, "{\n");
        fprintf(fout, "}\n");
        if(!fout)
            return;
        fclose(fout);
    }

    //cgilist
    if((fout = fopen(CGI_LIST_FILE, "a"))==NULL)
    {
        return;
    }
    if ((gentype & OPT_TYPE_CGI) && (cgiflag || formflag))
    {
        sscanf(fileinfo.cginame, WEB_CGI_FOLDER"/%s", tmp);
        if(tp=strstr(tmp, ".c"))
        {
            *(tp+1)='o';
        }
        fprintf(fout, "%s\n", tmp);
    }
    if (gentype & OPT_TYPE_DATA)
    {

        sscanf(fileinfo.dataname, WEB_CGI_FOLDER"/%s", tmp);
        if(tp=strstr(tmp,".c"))
        {
            *(tp+1)='o';
        }
        fprintf(fout, "%s\n", tmp);
    }
    fclose(fout);
    //end cgi list

    return;
}
/*****************************************************************
 * NAME: init_cfdk
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int init_cfdk()
{
    FILE *fp=NULL;

    system("touch "_WEB_RES_TABLE_FILE);

    fp = fopen(WEB_RES_TABLE_FILE, "w");
    if(!fp)
    {
        return -1;
    }
    fprintf(fp, "#include <stdlib.h>\n");
    fprintf(fp, "#include <string.h>\n");
    fprintf(fp, "#include <cgi.h>\n");
    fprintf(fp, "#include <web_submit_table.h>\n");
    fclose(fp);

    fp = fopen(WEB_SUBMIT_TABLE_FILE, "w");
    if(!fp)
    {
        return -1;
    }

    fprintf(fp, "#include <stdlib.h>\n");
    fprintf(fp, "#include <string.h>\n");
    fprintf(fp, "#include <cgi.h>\n");
    fprintf(fp, "#include <web_submit_table.h>\n");
    fclose(fp);

    fp = fopen(WEB_DISP_TABLE_FILE, "w");
    if(!fp)
    {
        return -1;
    }
    fprintf(fp, "#include <stdlib.h>\n");
    fprintf(fp, "#include <string.h>\n");
    fprintf(fp, "#include <cgi.h>\n");
    fprintf(fp, "#include <envcfg.h>\n");
    fclose(fp);

    fp = fopen(WEB_DISP_TABLE2_FILE, "w");
    if(!fp)
    {
        return -1;
    }
    fprintf(fp, "#include <stdlib.h>\n");
    fprintf(fp, "#include <string.h>\n");
    fprintf(fp, "#include <cgi.h>\n");
    fprintf(fp, "#include <envcfg.h>\n");
    fclose(fp);
}
/*****************************************************************
 * NAME: finish_cfdk
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int finish_cfdk()
{
    FILE *fout=NULL, *fin=NULL;
    char buf[MAX_LINE_BUFF_SIZE]={0};
    int len=0;
    char *ep=NULL;

    //append _WEB_SUBMIT_TABLE_FILE to WEB_SUBMIT_TABLE_FILE
    sorttable(_WEB_SUBMIT_TABLE_FILE, 0);
    fout = fopen(WEB_SUBMIT_TABLE_FILE, "a");
    fin = fopen(_WEB_SUBMIT_TABLE_FILE, "r");

    if(!fout) { return -1; }
    if(!fin) { fclose(fout); return -1; }
    fprintf(fout, "CGI_SUBMIT_TABLE_S cgi_submit_table [] = \n{\n");
    while(fgets(buf, sizeof(buf), fin))
    {
        fprintf(fout, "%s", buf);
    }
    fprintf(fout, "};\n");
    fprintf(fout, "CG_SUBMIT_TABLES   cg_submit_tables =\n{\n\t{cgi_submit_table,  sizeof(cgi_submit_table)/sizeof(CGI_SUBMIT_TABLE_S)}\n};\n");

    fclose(fout);
    fclose(fin);

    //append _WEB_RES_TABLE_FILE to WEB_RES_TABLE_FILE
    sorttable(_WEB_RES_TABLE_FILE, 0);
    fout = fopen(WEB_RES_TABLE_FILE, "a");
    fin = fopen(_WEB_RES_TABLE_FILE, "r");

    fprintf(fout, "CGI_TABLE_S cgi_GetTable [] =\n");
    fprintf(fout, "{\n");
    if(!fout) { return -1; }
    if(!fin) { fclose(fout); return -1; }
    while(fgets(buf, sizeof(buf), fin))
    {
        fprintf(fout, "%s", buf);
    }
    fprintf(fout, "};\n");
    fprintf(fout, "CGI_ENUMTABLE_S cgi_EnumTable [] =\n{\n};\n");
    fprintf(fout, "CG_RES_TABLES  cg_res_tables =\n");
    fprintf(fout, "{\n\t{cgi_GetTable,\t\t\tsizeof(cgi_GetTable)/sizeof(CGI_TABLE_S)},\n");
    fprintf(fout, "\t{cgi_EnumTable,\t\t\tsizeof(cgi_EnumTable)/sizeof(CGI_ENUMTABLE_S)}\n};\n");

    fclose(fout);
    fclose(fin);

    //append _WEB_DISP_TABLE_FILE to WEB_DISP_TABLE_FILE
    sorttable(_WEB_DISP_TABLE_FILE, 0);
    fout = fopen(WEB_DISP_TABLE_FILE, "a");
    fin = fopen(_WEB_DISP_TABLE_FILE, "r");

    fprintf(fout, "CGI_FILEINFO_S cgi_arFileList [] =\n");
    fprintf(fout, "{\n");
    if(!fout) { return -1; }
    if(!fin) { fclose(fout); return -1; }
    while(fgets(buf, sizeof(buf), fin))
    {
        fprintf(fout, "%s", buf);
    }
    fprintf(fout, "};\n");
    fprintf(fout, "CG_FILE_TABLES cg_file_tables =\n");
    fprintf(fout, "{\n    {cgi_arFileList, sizeof(cgi_arFileList)/sizeof(CGI_FILEINFO_S)}\n};\n");

    fclose(fout);
    fclose(fin);

    //append _WEB_DISP_TABLE_FILE to WEB_DISP_TABLE2_FILE
    sorttable(_WEB_DISP_TABLE_FILE, 0);
    fout = fopen(WEB_DISP_TABLE2_FILE, "a");
    fin = fopen(_WEB_DISP_TABLE_FILE, "r");

    fprintf(fout, "CGI_FILEINFO_S cgi_arFileList [] =\n");
    fprintf(fout, "{\n");
    if(!fout) { return -1; }
    if(!fin) { fclose(fout); return -1; }
    while(fgets(buf, sizeof(buf), fin))
    {
        fprintf(fout, "%s", buf);
    }
    fprintf(fout, "};\n");
    fprintf(fout, "CG_FILE_TABLES cg_file_tables =\n");
    fprintf(fout, "{\n    {cgi_arFileList, sizeof(cgi_arFileList)/sizeof(CGI_FILEINFO_S)}\n};\n");

    fclose(fout);
    fclose(fin);

    //sort language list file
    fin = fopen(LANG_LIST_FILE_FORCE, "r");
    if(fin)
    {
        while(fgets(buf, sizeof(buf), fin))
        {
            if(buf[0]=='#')
            {
                //skip comment
                continue;
            }
            //write langlist to langtmp
            if (ep = strchr(buf, '\r'))
            {
                *ep = '\0';
            }
            if (ep = strchr(buf, '\n'))
            {
                *ep = '\0';
            }
            if(isInt(buf) &&
                    noDupWrite(CGI_TYP_NONE, LANG_LIST_FILE, buf, NULL)==0)
            {
                //do nothing
            }
        }
        fclose(fin);
    }
    sorttable(LANG_LIST_FILE, 1);

    //end sourt language list file
}
/*****************************************************************
 * NAME: cmpfunc
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int cmpfunc(const void * a, const void * b)
{
    return ( *(int*)a - *(int*)b );
}
/*****************************************************************
 * NAME: genLanguageFile
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int genLanguageFile(char *filenameList)
{
    char namebuf[128]={0}, dbname[64]={0}, langname[64]={0};
    FILE *fout=NULL, *fdb=NULL, *flist=NULL;
    char *plangStr, *ep=NULL;
    char ch, buf[MAX_LINE_BUFF_SIZE]={0};
    int len=0;
    int ret, lines=0, cnt=0, first=1, key, fakelang=0;
    int *listVal, *item;

    ret = sscanf(filenameList, "%[^:]:%[^:]:%d", dbname, langname, &fakelang);
    printf("[%s]->[%s] fake:[%d]\n", dbname ,langname, fakelang);

    //if(ret != 2)
    //{
    //    printf("language database format error\n");
    //    goto error;
    //}

    fdb = fopen(dbname, "r");
    if(!fdb)
    {
        printf("language database error\n");
        goto error;
    }

    fout = fopen(langname, "w");
    if(!fout)
    {
        printf("fout errror\n");
        goto error;
    }

#ifdef WEB_LANG_ARRAY_NAME
    fprintf(fout, "var "WEB_LANG_ARRAY_NAME"= new Array(\n");
#else
    fprintf(fout, "var tekst_array= new Array(\n");
#endif


    if(!fakelang)
    {
        flist = fopen(LANG_LIST_FILE_NAME, "r");
        if(!flist)
        {
            printf("language list file errror\n");
            goto error;
        }


        // scan file line
        while(!feof(flist))
        {
            ch = fgetc(flist);
            if(ch == '\n')
            {
                lines++;
            }
        }
        rewind(flist);
        // end scan file line

        // scan .langlist to big array
        listVal = (int *)malloc(sizeof(int)*lines);
        while(fgets(buf, sizeof(buf), flist))
        {
            sscanf(buf, "%d", &listVal[cnt]);
            //printf("%d\n",listVal[cnt]);
            cnt++;
        }
    }

    // compare
    while(fgets(buf, sizeof(buf), fdb))
    {
        if(!first)
        {
            fprintf(fout, ",\n");
        }
        else
        {
            first=0;
        }

        ret = sscanf(buf, "%d,", &key);
        if(ret)
        {
            if (ep = strchr(buf, '\r'))
            {
                *ep = '\0';
            }
            if (ep = strchr(buf, '\n'))
            {
                *ep = '\0';
            }
            plangStr = strchr(buf, ',');
            if(plangStr)
                plangStr = plangStr+1;

            if(fakelang == 1)
            {
                fprintf(fout, "\"%s\"", plangStr);
            }
            else
            {
                item = (int*) bsearch (&key, listVal, lines, sizeof (int), cmpfunc);
                if( item != NULL )
                {
                    fprintf(fout, "\"%s\"", plangStr);
                }
                else
                {
                    fprintf(fout, "\"\"");
                }
            }
        }
        else
        {
            printf("warning: language database error\n");
            printf("[%s]\n", buf);
            fprintf(fout, "\"\"");
        }
    }
    fprintf(fout, "\n);");

error:
    if(fout)
        fclose(fout);
    if(fdb)
        fclose(fdb);
    if(!fakelang)
    {
        if(flist)
            fclose(flist);
        if(listVal)
            free(listVal);
    }
}
/*****************************************************************
 * NAME: main
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 ****************************************************************/
int main(int argc, char **argv)
{
    int c=0;
    char *fvalue=NULL;

    while ((c = getopt (argc, argv, "iqcdp:f:l:")) != -1)
    {
        switch (c)
        {
            case 'i':
                init_cfdk();
                goto exit;
            case 'q':
                finish_cfdk();
                goto exit;
            case 'f':
                fvalue=optarg;
                if(fvalue)
                {
                    printf(".");
                    CFDK_DEBUG("-->[%s]\n", fvalue);
                }
                break;
            case 'c':
                gentype |= OPT_TYPE_CGI;
                break;
            case 'd':
                gentype |= OPT_TYPE_DATA;
                break;
            case 'p':
                pageperm = atoi(optarg);
                break;
            case 'l':
                fvalue=optarg;
                if(fvalue)
                {
                    //printf("[%s]\n",fvalue);
                    genLanguageFile(fvalue);
                }
                goto exit;
            default:
                abort();
                break;
        }
    }

    if(pageperm<0)
    {
        strcpy(homepage, fvalue);
    }

    if(fvalue)
    {
        genNames(fvalue);
        parsingFile(fvalue);
    }

exit:
    return 0;
}
