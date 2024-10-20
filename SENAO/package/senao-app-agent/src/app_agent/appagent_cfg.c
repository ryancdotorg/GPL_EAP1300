#include "appagent_cfg.h"
#include "appagentd.h"
#include "apiport.h"

#define HTTP_CFG_FILE_NAME      "/tmp/http_cfg"

/**********************************/
/* Data sturcture definition      */
/**********************************/
static  KNL_SEM_ID  HttpCfg_Sem;
static  int         HttpCfg_PortNum;
static  HTTP_CFG_T  *pHttpCfg_Info;

/*--------------------------------------------------------------
* ROUTINE NAME - HttpCfg_Init                                  
*---------------------------------------------------------------
* FUNCTION: Init the http's variables
* INPUT:   None
* OUTPUT:  None 
* RETURN:  None 
* NOTE:     
---------------------------------------------------------------*/
INT32 HttpCfg_Init(HTTP_CFG_DATA_T *map)
{
    INT32   update;
    int i;

    FILE    *pFile;
    INT32   file_size;

    //******************************
    // Initialize the variables
	//******************************

    HttpCfg_PortNum = map->num;

    // One for status, other for the ports
    file_size = sizeof(HTTP_CFG_T) + (map->num - 1) * sizeof(UINT16);
        
    if ((pHttpCfg_Info = (HTTP_CFG_T *)malloc(file_size)) == 0)
    {
        KNL_SEM_UNLOCK(HttpCfg_Sem);
        return ERROR;
    }

    // Do factory default settings
    pHttpCfg_Info->status = map->status;
    for (i=0; i<HttpCfg_PortNum; i++)
        pHttpCfg_Info->port[i] = map->ports[i];

	//******************************
    // Open user group file in
	// NV file system
	//******************************
	if ((pFile = fopen(HTTP_CFG_FILE_NAME, "r+b")) == (FILE *)0)
	{
        update = TRUE;
	}
	else
	{
        //*************************
        // Check file size, 
        // read and close the file
        //*************************
        fseek(pFile, 0, SEEK_END);
        if (ftell(pFile) != file_size)
            update = TRUE;
        else
            update = FALSE;

        rewind(pFile);   
        fread(pHttpCfg_Info, 1, file_size, pFile);       // Read to buffer
        fclose(pFile);
       
        //check range
        switch (pHttpCfg_Info->status)
        {
        case HTTP_CFG_ENABLED:
        case HTTP_CFG_DISABLED:
            break;

        default:    
            pHttpCfg_Info->status = map->status;  
            update = TRUE;
            break;
        }
        
        
        for (i=0; i<HttpCfg_PortNum; i++)
        {
            if (pHttpCfg_Info->port[i] == HTTP_CFG_PORT_TELNET)
            {
                pHttpCfg_Info->port[i] = map->ports[i];
                update = TRUE;
            }
        }
    }

    if (update == TRUE)
    {
		pFile = fopen(HTTP_CFG_FILE_NAME, "w+b");
		fwrite(pHttpCfg_Info, 1, file_size, pFile);
        if(pFile) fclose(pFile);
    }

    /*************
     Take action
     *************/
    if (pHttpCfg_Info->status == HTTP_CFG_ENABLED)
    {
        httpd_Start();

        //for (i=0; i<HttpCfg_PortNum; i++)
        for (i=0; i<1; i++) //cfho 0331
        {
            httpd_AddListenPort(pHttpCfg_Info->port[i]);
            //printf("====>add listen port %d\n",pHttpCfg_Info->port[i]);
        }
    }

    return OK;
}

