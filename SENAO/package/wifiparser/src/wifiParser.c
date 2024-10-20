#include <stdio.h>
#include <stdlib.h>
#include <string.h>           
#include <ctype.h>
#include <unistd.h>
#include <assert.h>

#define MAX_PARAMETERS_NUM 100
#define MAX_LAYERS_NUM 100
#define MAX_BUFFER_SIZE 1024
#define TRUE 1
#define FALSE 0
#define OUT_OF_FUNC 1
#define IN_OF_FUNC 0
#define END_OF_FUNC 2

unsigned char DebugEnable=0;

char inpath[]="cat /lib/wifi/[FileName].sh";
char outpath[]="cat > /lib/wifi/[FileName].sh";
char orig_str[]="[FileName]";
char *basic_func[]={"DebugPrint","wlanconfig","iwconfig","iwpriv","delay_cmd","insmod","get_sn_wifi_option","get_beacon_interval"}; // declare basic function
char *basic_return_func[MAX_BUFFER_SIZE]={"\0"};
FILE *fp_read,*fp_write;

typedef struct _InputFile{
    char *in_name[MAX_BUFFER_SIZE];   
}InputFile;

typedef struct _StoreData{
    char store_data[MAX_BUFFER_SIZE]; // all file data
    char store_parse_data[MAX_BUFFER_SIZE]; 
}StoreData;

StoreData sd[4096];

typedef struct _FuncScope{
    int curl_brac_left;
    int curl_brac_right;
}FuncScope;

FuncScope fs[MAX_BUFFER_SIZE]={{.curl_brac_left=0,.curl_brac_right=0}};

typedef struct _ScriptFunc{
    char *func_value[MAX_BUFFER_SIZE];
}ScriptFunc;

// segment string to get function name and parameters 
ScriptFunc segStr(char *str)
{
    ScriptFunc sf;
    char delim[]=" \t\"=()\n";
    char *seg[MAX_BUFFER_SIZE];
    int i=0,j;

    memset(sf.func_value,'\0',sizeof(sf.func_value)); //clear array value in struct, avoiding the previous value to exist
    seg[i] = strtok(str,delim);

    while (seg[i] != NULL)
    {
        i++;
        seg[i] = strtok(NULL,delim);       
    }

    for (j=0;j<i;j++)
    {
        sf.func_value[j] = seg[j];
    }

    return sf;
}

//  replacing the string
void replaceStr(char *str, char *orig_str, char *rep_str)   //string, orignal string, replace string 
{
    char buf_str[MAX_BUFFER_SIZE];
    char *p;

    p = strstr(str,orig_str);
    strncpy(buf_str,str,p-str);
    buf_str[p-str] = '\0';
    sprintf(buf_str+(p-str),"%s%s",rep_str,p+strlen(orig_str));
    strcpy(str,buf_str);   
}

// check if same parameter or not   ex. $var=$var
int checkSamePara(char *buffer)
{
    char compare_str[MAX_BUFFER_SIZE],new_buffer[MAX_BUFFER_SIZE];
    int i;
    ScriptFunc check_sf;

    strcpy(new_buffer,buffer);
    check_sf = segStr(new_buffer);

    for (i=0;check_sf.func_value[i+1] != NULL;i++)
    {        
        strcpy(compare_str,"$");
        strcat(compare_str,check_sf.func_value[i]);
        
        if (strcmp(compare_str,check_sf.func_value[i+1]) == 0)
        {
            return TRUE;
        }
    }  
}

// store multiple file data in the same memory 
void storeFileData(int in_file_amount, struct _InputFile input_file)
{
    int i,j=0;
    char new_inpath[MAX_BUFFER_SIZE],buffer[MAX_BUFFER_SIZE],new_buffer[MAX_BUFFER_SIZE];
    ScriptFunc store_sf;
    
    for(i=0;i<in_file_amount;i++)
    {
        strcpy(new_inpath,inpath);        
        replaceStr(new_inpath,orig_str,input_file.in_name[i]); // get file path

        fp_read = popen(new_inpath,"r");
        
        if(fp_read == NULL) // check input file path is correct or file is not null
        {
            printf("Error Path: %s\n",new_inpath);
            printf("Please check the path or the file if exist!");
        }

        if (DebugEnable)
        {
            printf("input File Path: %s\n",new_inpath);
        }
               
        while ((fgets(buffer,sizeof(buffer),fp_read)) != NULL)
        {
            strcpy(new_buffer,buffer);
            store_sf = segStr(new_buffer);

            if (((int)strlen(store_sf.func_value) > 0 ) && (strstr(store_sf.func_value[0],"#") == NULL)) // determine blank line and comment
            {
                strcpy(sd[j].store_data,buffer);                           
                j++;
            }
        }

        pclose(fp_read);
    }   
}

// determine function scope
int judgeScope(char *buffer, int layer) 
{
    char *sear_buffer,judge_func_name[MAX_BUFFER_SIZE];   

    sear_buffer = buffer;
    
    while ((sear_buffer = strstr(sear_buffer,"{")) != NULL) // use while, data1 { data2... {data3}} , have more { and }
    {
        fs[layer].curl_brac_left++;
        sear_buffer++;
    }
    
    sear_buffer = buffer;

    while ((sear_buffer = strstr(sear_buffer,"}")) != NULL)
    {
        fs[layer].curl_brac_right++;
        sear_buffer++;
    }

    if (strstr(buffer,"()") != NULL)
    {        
        return IN_OF_FUNC;
    }
    
    if (fs[layer].curl_brac_left != fs[layer].curl_brac_right) // IN_OF_FUNC: the data in the function
    {
        return IN_OF_FUNC;
    }

    if ((fs[layer].curl_brac_left == fs[layer].curl_brac_right) && (fs[layer].curl_brac_left > 0)) // END_OF_FUNC: the end of function's scope
    {
        fs[layer].curl_brac_left=0;
        fs[layer].curl_brac_right=0;  
        return END_OF_FUNC;      
    }

    if ((fs[layer].curl_brac_left == 0) && (fs[layer].curl_brac_right == 0)) // OUT_OF_FUNC: the data out of function
    {
        return OUT_OF_FUNC;
    }  
}

// check if function name exist or not 
int checkFuncName(struct _ScriptFunc check_Func) 
{
    int i=0,j,check_repeat=0,check_name=0;
    char delim[]=" {\n",new_check_func[MAX_BUFFER_SIZE],new_store_data[MAX_BUFFER_SIZE];

    strcpy(new_check_func,check_Func.func_value[0]);
    strcat(new_check_func,"()");   

    while (strlen(sd[i++].store_data) > 1) // read the file data in storing memory
    {
        strcpy(new_store_data,sd[i].store_data);
        strtok(new_store_data,delim); // ensure only function()
        
        if (strcmp(new_store_data,new_check_func) == 0)
        {           
            check_name = TRUE; // have same func name and same length 
        }
    
        for (j=0;j<(sizeof(basic_func)/sizeof(basic_func[0]));j++) // avoid parser basic_func again
        {
            if (strstr(check_Func.func_value[0],basic_func[j]) != NULL)
            {
                check_repeat = TRUE;
            }
        }
    }

    return ((check_name == TRUE) && (check_repeat == FALSE)) ? TRUE : FALSE; // TRUE: the function name is searched
}

//check in the function if exist return or not
int checkReturn(struct _ScriptFunc reruen_Func, int layer)
{
    int i=0,func_result,search_result=0,return_result;
    char new_func[MAX_BUFFER_SIZE];

    memset(new_func,'\0',sizeof(new_func));
    strcpy(new_func,reruen_Func.func_value[0]);
    strcat(new_func,"()");

    return_result=0;

    while (strlen(sd[i++].store_data) > 1) 
    {
        func_result = judgeScope(sd[i].store_data,layer);

        if ((strstr(sd[i].store_data,new_func)) != NULL)
        {
            search_result = TRUE;                            
        }            

        if ((search_result == TRUE) && (return_result != 1))
        {
            if (strstr(sd[i].store_data,"return") != NULL)
            {
                return_result = 1;
            }            
        }

        if ((search_result == TRUE) && (func_result == END_OF_FUNC))
        {
            break;
        }       
    }
    return (return_result == 1) ? TRUE : FALSE; // TRUE: the return is exist
}

// get basic function 
void basicFuncs(FILE *wt_fp, int layer)    
{   
    int i,j,basic_result,search_result;
    char new_basic_func[MAX_BUFFER_SIZE];

    for (j=0;j<(sizeof(basic_func)/sizeof(basic_func[0]));j++)
    {
        i=0;
        search_result=FALSE;
        memset(new_basic_func,'\0',sizeof(new_basic_func));
        strcpy(new_basic_func,basic_func[j]);
        strcat(new_basic_func,"()");

        while (strlen(sd[i++].store_data) > 1) 
        {                                    
            basic_result = judgeScope(sd[i].store_data,layer);

            if ((strstr(sd[i].store_data,new_basic_func)) != NULL)
            {
                search_result = TRUE;                            
            }            

            if (search_result == TRUE)
            {
                if (DebugEnable)
                {
                    printf("Line%d: %s",i,sd[i].store_data);
                }

                fprintf(wt_fp,"%s",sd[i].store_data);
            }

            if ((search_result == TRUE) && (basic_result == END_OF_FUNC))
            {
                break;
            }
        }
    }
}

// get basic define
void basicDefine(FILE *wt_fp, int layer) 
{
    int i=0,result=0;

    while (strlen(sd[i++].store_data) > 1) 
    {       
        result = judgeScope(sd[i].store_data,layer);
        
        if (result == OUT_OF_FUNC)
        {   
            if (DebugEnable)
            {
                printf("Line%d: %s",i,sd[i].store_data);
            }

            fprintf(wt_fp,"%s",sd[i].store_data);
        }
    }    
}

// get basic function with the layer
void basicLayerFunc(FILE *wt_fp, char *layer_func_name, int layer)
{
    int i=0,search_result,basic_result;

    search_result = 0;
    strcat(layer_func_name,"()");
    
    while (strlen(sd[i++].store_data) > 1)
    {
        basic_result = judgeScope(sd[i].store_data,layer);
        if ((strstr(sd[i].store_data,layer_func_name)) != NULL)
        {
            search_result = TRUE;
        }  

        if (search_result == TRUE)
        {
            if (DebugEnable)
            {
                printf("Line%d: %s",i,sd[i].store_data);
            }

            fprintf(wt_fp,"%s",sd[i].store_data);
        }

        if ((search_result == TRUE) && (basic_result == END_OF_FUNC))
        {
            break;
        }
    }
}

// check which function needs to be basic function in the layer
void checkLayerFunc(FILE *wt_fp, int layer, char *out_file_name, struct _ScriptFunc parser_func, int n)
{
    static int generate_p=0;
    int i=0,j=0,search_result=0,parser_result=0,check_result=0; 
    char new_out_file_name[MAX_BUFFER_SIZE],new_store_data[MAX_BUFFER_SIZE],new_parser_func[MAX_BUFFER_SIZE];
    ScriptFunc sf;

    strcpy(new_out_file_name,out_file_name);
    strcat(new_out_file_name,"()");

    if (parser_func.func_value[0] != NULL)
    {
        strcpy(new_parser_func,parser_func.func_value[0]);
        strcat(new_parser_func,"()");        
    }

    while (strlen(sd[i++].store_data) > 1) 
    {
        check_result = FALSE; // determine if function exist or not   FALSE:no    TRUE:yes

        parser_result = judgeScope(sd[i].store_data,n); 
        
        if ((search_result == TRUE) && (parser_result == IN_OF_FUNC)) // function name exist and in the function's scope
        {
            strcpy(new_store_data,sd[i].store_data);
            sf = segStr(new_store_data);
            check_result = checkFuncName(sf);

            if ((check_result == TRUE) && (layer != 0)) 
            { 
                checkLayerFunc(wt_fp,layer-1,out_file_name,sf,n+1);                               
            }

            if ((check_result == TRUE) && (layer == 0))
            {
                basicLayerFunc(wt_fp,sf.func_value[0],n+1);
            }
        }

        if ((generate_p == 1) && ((strstr(sd[i].store_data,new_parser_func)) != NULL)) 
        {
            search_result = TRUE;
        }

        if ((generate_p == 0) && ((strstr(sd[i].store_data,new_out_file_name)) != NULL))
        {
            search_result = TRUE;
            generate_p = TRUE;
        }
        
        if ((search_result == TRUE) && (parser_result == END_OF_FUNC)) 
        {
            break;
        }        
    }
}

// get the function having return to be basic function
void basicReturnFuncs(FILE *wt_fp, int layer)
{ 
    int i,j,basic_return_result,search_result;
    char new_return_func[MAX_BUFFER_SIZE];
    ScriptFunc return_sf;

    for (j=0;;j++)
    {
        if (basic_return_func[j] == NULL)
        {
            break;
        }

        i=0;
        search_result=FALSE;

        memset(new_return_func,'\0',sizeof(new_return_func));
        strcpy(new_return_func,basic_return_func[j]);
        return_sf = segStr(new_return_func);
        strcat(return_sf.func_value[0],"()");

        while (strlen(sd[i++].store_data) > 1) 
        {                                    
            basic_return_result = judgeScope(sd[i].store_data,layer);

            if ((strstr(sd[i].store_data,return_sf.func_value[0])) != NULL)
            {
                search_result = TRUE;                            
            }            

            if (search_result == TRUE)
            {
                if (DebugEnable)
                {
                    printf("Line%d: %s",i,sd[i].store_data);
                }

                fprintf(wt_fp,"%s",sd[i].store_data);
            }

            if ((search_result == TRUE) && (basic_return_result == END_OF_FUNC))
            {
                break;
            }
        }
    }
}

// get scripts data
void parserScripts(FILE *wt_fp, int layer, char *out_file_name, struct _ScriptFunc parser_func, int n)
{
    static int generate_phase=0,m=0,x=0;
    int i=0,j,k,l,search_result=0,parser_result=0,check_result=0,generate_var,check_para,return_result=0; 
    char new_store_data[MAX_BUFFER_SIZE],new_parser_func[MAX_BUFFER_SIZE],dollar_sign[MAX_BUFFER_SIZE],annotation[MAX_BUFFER_SIZE];
    char *para[MAX_BUFFER_SIZE],*param,num[MAX_BUFFER_SIZE];
    ScriptFunc sf;

    strcat(out_file_name,"()");

    if (parser_func.func_value[0] != NULL) // first input value is outputname, so avoid parser_func no value to impact processes   
    {
        strcpy(new_parser_func,parser_func.func_value[0]);
        strcat(new_parser_func,"()");        
    }

    while (strlen(sd[i++].store_data) > 1) 
    {
        check_result = FALSE; // determine if function exist or not   FALSE:no    TRUE:yes
        check_para = FALSE;   // determine if same parameter or not   FALSE:no    TRUE:yes
        parser_result = judgeScope(sd[i].store_data,n); // IN_OF_FUNC:in function  OUT_OF_FUNC:out function  END_OF_FUNC:function's scope end

        if ((search_result == TRUE) && (parser_result == IN_OF_FUNC)) // function name exist and in the function's scope
        {
            strcpy(new_store_data,sd[i].store_data);
            sf = segStr(new_store_data);
            check_result = checkFuncName(sf);

            for (k=1;k<MAX_PARAMETERS_NUM;k++)   //default number of parameters: 100
            {   
                strcpy(dollar_sign,"$");                                 
                sprintf(num,"%d",k);
                param = strcat(dollar_sign,num);

                if ((strstr(sd[i].store_data,param) != NULL) && (generate_var == TRUE)) //replace parameter and avoid generate_var to replace    
                {
                    replaceStr(sd[i].store_data,param,para[k]);
                    check_para = checkSamePara(sd[i].store_data); // determine if same parameter or not   FALSE:no    TRUE:yes
                }                                                                       
            }

            if ((check_result == TRUE) && (layer != 0)) 
            { 
                return_result=checkReturn(sf,n+1);//check the function if exist or not

                if (DebugEnable)
                {
                    printf("Line%d layer%d return%d: ====%s",i,n,return_result,sd[i].store_data);
                }

                if (return_result == FALSE)
                {
                    parserScripts(wt_fp, layer - 1, out_file_name, sf, n + 1);
                }
                
                if (return_result == TRUE )
                {
                    basic_return_func[m]=sd[i].store_data;// throw function name of having return to basic function

                    if (DebugEnable)
                    {
                        printf("////////////////%d %s\n", m, sd[i].store_data);
                    }

                    strcpy(sd[x].store_parse_data,sd[i].store_data); //copy the return function to data memory 
                    x++;
                    m++;
                }
            }
            else
            {
                if (DebugEnable)
                {
                    for (l=1;l<n;l++)
                    {
                        printf("\t");
                    }

                    if (strlen(sd[i].store_data) == 2) // if first { under the function() and delete same parameter
                    {
                        printf("Line%d layer%d strlen%d: ////%s",i,n,(int)strlen(sd[i].store_data),sd[i].store_data);
                    }
                    else if (check_para == TRUE)
                    {
                        printf("Line%d layer%d strlen%d: ~~~~%s",i,n,(int)strlen(sd[i].store_data),sd[i].store_data);
                    }
                    else
                    {
                        printf("Line%d layer%d strlen%d: %s",i,n,(int)strlen(sd[i].store_data),sd[i].store_data);
                    }                   
                }

                if ((strlen(sd[i].store_data) > 2) && (check_para == FALSE) ) 
                {
                    for (l=1;l<n;l++) // typesetting with the layer
                    {
                        strcat(sd[x].store_parse_data,"\t");
                    }
                    
                    strcat(sd[x].store_parse_data,sd[i].store_data); // copy to data memory
                    x++;
                }
            }
        }

        if ((generate_phase == TRUE) && ((strstr(sd[i].store_data,new_parser_func)) != NULL)) 
        {
            search_result = TRUE;
            generate_var = TRUE;
            memset(para,0,sizeof(para));
            j=0;

            while (parser_func.func_value[j] != NULL)
            {
                para[j] = parser_func.func_value[j];
                j++;
            }
        }

        if ((generate_phase == FALSE) && ((strstr(sd[i].store_data,out_file_name)) != NULL))
        {
            search_result = TRUE;
            generate_var = FALSE;
            generate_phase = TRUE;
        }
        
        if ((search_result == TRUE) && (parser_result == END_OF_FUNC))// function name exist and the function's scope end 
        {
            break;
        }  
    }
}

//print the parser data
void ScriptsData(FILE *wt_fp)
{
    int i=0;

    while (strlen(sd[i].store_parse_data) > 1)
    {
        fprintf(wt_fp,"%s",sd[i].store_parse_data);
        i++;
    }   
}

int main (int argc, char **argv)
{
    int ch,out_script_num=0,j,in_script_num=0,layer = MAX_LAYERS_NUM,i;
    char *in_name[MAX_BUFFER_SIZE],out_name[MAX_BUFFER_SIZE],new_outpath[MAX_BUFFER_SIZE];
    InputFile input_file;
    ScriptFunc parser_func;
    
    //  linux command + parameter start   
    while((ch = getopt(argc, argv,"i:o:dl:[h || -help]")) != -1)
    {
        switch(ch)
        {
        case 'i':
            in_name[in_script_num] = optarg;
            in_script_num++; // detemine input file amount
            break;
        case 'o':
            strcpy(out_name,optarg);
            out_script_num++; // detemine output file amount
            break;       
        case 'd':
            DebugEnable=1;
            break;
        case 'l':
            layer = atoi(optarg); //atoi() is char to int
            break;
        case 'h':
            printf("***************************************************************\n");
            printf("*****Usage: [-d] [-l Layer] [-i inputFile] [-o outputFile]*****\n");
            printf("*****        -i             [inputFile]   #multiple files *****\n");
            printf("*****        -o             [outputFile]  #one file       *****\n");
            printf("*****        -d             Enable Debug                  *****\n");
            printf("*****        -l             [Layer]       #default 100    *****\n");
            printf("*****        -h             show command                  *****\n");
            printf("*****        --help         show command                  *****\n");
            printf("*****    Note: The file need to store in /lib/wifi/ !!    *****\n");
            printf("***************************************************************\n");
            break;
        default:
            fprintf(stderr,"%s Usage: Error command, please use -h or --help to show command!\n", argv[0]);
            break;
        }     
    }       

    if (out_script_num == 1)    //determining the output file if only one file or not  
    {   
        for (j=0;j<in_script_num;j++)
        {
            input_file.in_name[j] = in_name[j];           
        }

        storeFileData(in_script_num,input_file);
        strcpy(new_outpath,outpath);        
        replaceStr(new_outpath,orig_str,out_name);

        fp_write = popen(new_outpath,"w"); 

        fprintf(fp_write,"## Basic Define ##\n");
        basicDefine(fp_write,0);
        fprintf(fp_write,"\n## Basic Functions ##\n");
        basicFuncs(fp_write,0);
        fprintf(fp_write,"\n## Layer Functions ##\n");
        checkLayerFunc(fp_write,layer,out_name,parser_func,0);
        parserScripts(fp_write,layer,out_name,parser_func,0);

        if ((layer > 0) && ((int)strlen(basic_return_func[0]) > 0)) //avoid basic_return_func no exist value
        {
            fprintf(fp_write,"\n## Basic Return Functions ##\n");
            basicReturnFuncs(fp_write,0);
        }

        fprintf(fp_write,"\n## Scripts Data ##\n");
        ScriptsData(fp_write);

        pclose(fp_write);
    }
    
    if ((out_script_num != 0) && (out_script_num != 1))
    {
        printf("need only one output File, please try again!\n");
    }
}



