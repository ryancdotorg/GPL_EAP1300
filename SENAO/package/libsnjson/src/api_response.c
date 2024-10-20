#include <api_response.h>
#include <unistd.h>

/*-----------------------------------------------------------------------*/
/*                        GLOBAL VARIABLES                               */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                            FUNCTION                              */
/*-----------------------------------------------------------------------*/

#if 0
const ResponseStatus* keyvalue_get_value(ResponseStatus *kv, int k) {
    int i;
    for (i = 0; kv[i].errCode; i++) {
        if (kv[i].errCode == k) return &kv[i];
    }

    return NULL;
}

const char *get_http_status_name(int i) {
    return keyvalue_get_value(http_status, i);
}
#endif

/*****************************************************************
* NAME:    Response_create
* ---------------------------------------------------------------
* FUNCTION: create stack
* INPUT:    void
* OUTPUT:   Stack*
******************************************************************/
Stack* Response_stackCreate()
{
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return stackCreate();
}

/*****************************************************************
* NAME:     Response_stackDestroy 
* ---------------------------------------------------------------
* FUNCTION: destroy stack and free stack item
* INPUT:    Stack *stack
* OUTPUT:   void
******************************************************************/
void Response_stackDestroy(Stack *stack)
{
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj = NULL;
    StackStruct *stackItem = NULL;
    while(!stackIsEmpty(stack)) {
debug_print("Jason DEBUG %s[%d] [%d]\n", __FUNCTION__, __LINE__, stackSize(stack));
       if(stackItem = (StackStruct *) stackPop(stack)){
debug_print("Jason DEBUG %s[%d] [%d]\n", __FUNCTION__, __LINE__, stackSize(stack));
            if ( stackItem->type == STACKTYPE_JSON )
            {
                if ( jobj = (struct json_object *)stackItem->item )
                    json_object_put(jobj);
            }
            else if ( stackItem->type == STACKTYPE_STR )
            {
                if ( stackItem->item )
                    free(stackItem->item);
            }
       }
    }
    free(stack);
}

/*****************************************************************
* NAME:     Response_strStackDestroy
* ---------------------------------------------------------------
* FUNCTION: destroy stack and free stack item
* INPUT:    Stack *stack
* OUTPUT:   void
******************************************************************/
void Response_strStackDestroy(Stack *stack)
{
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    void *str = NULL;
    int i = 0;
    while(!stackIsEmpty(stack)) {
        if(str = stackPop(stack)){
debug_print("Jason DEBUG %s[%d] [%s]\n", __FUNCTION__, __LINE__, str);
            free(str);
        }
    }
    free(stack);
}

/*****************************************************************
* NAME:    Response_create
* ---------------------------------------------------------------
* FUNCTION: RepsonseEntry create and init
* INPUT:    void
* OUTPUT:   ResponseEntry*
******************************************************************/
ResponseEntry* Response_create()
{
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    ResponseEntry *rep;
    if((rep = (ResponseEntry *)malloc(sizeof(ResponseEntry))) == NULL) {
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        goto err;
    }

    if((rep->res = (ResponseStatus *)malloc(sizeof(ResponseStatus))) == NULL) {
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        goto err;
    }

    if((rep->stack = Response_stackCreate()) == NULL){
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        goto err;
    }

    if((rep->strStack = Response_stackCreate()) == NULL){
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        goto err;
    }

    rep->jobj = NULL;

    memset(rep->res->reasonPhrase, 0, sizeof(rep->res->reasonPhrase));
    memset(rep->res->errMsg, 0, sizeof(rep->res->errMsg));

    rep->res->statusCode = HTTP_STATUS_CODE_DEFAULT;
    rep->res->errCode = HTTP_ERROR_CODE_DEFAULT;

    return  rep;
err:
    if(rep) {
        Response_destroy(rep);
    }
    return NULL;

    }

/*****************************************************************
* NAME:    Response_destrory
* ---------------------------------------------------------------
* FUNCTION: RepsonseEntry create and init
* INPUT:    void
* OUTPUT:   int
******************************************************************/
int Response_destroy(ResponseEntry *rep)
{
    if(rep == NULL) {
        return -1;
    }
    if(rep->strStack)
        Response_strStackDestroy(rep->strStack);
    if(rep->stack)
        Response_stackDestroy(rep->stack);
    if(rep->res)
        free(rep->res);
    if ( rep->jobj )
        json_object_put((struct json_object *)rep->jobj);
    if(rep)
        free(rep);
    return 0;
}

/*****************************************************************
* NAME:    getResponseStatusDefine
* ---------------------------------------------------------------
* FUNCTION: get ResponseStatus from  erroor_code_mapping table
* INPUT:    error_code_t
* OUTPUT:   const ResponseStatus*
******************************************************************/
const  ResponseStatus* getResponseStatusDefine(error_code_t k) {
    int len = T_NUM_OF_ELEMENTS(error_code_mapping);
    int i;
    for (i = 0; i < len; i++) {
        if (error_code_mapping[i].errCode == k)
            return &error_code_mapping[i];
    }

    return NULL; 
}

/*****************************************************************
* NAME:    genErrorMessage
* ---------------------------------------------------------------
* FUNCTION: generate HttpResponse error message
* INPUT:    ResponseStatus, int, char* 
* OUTPUT:   int
******************************************************************/
int genErrorMessage(ResponseStatus *res, int error_code, char* msg)
{
    const ResponseStatus *e = getResponseStatusDefine(error_code);
    if (NULL != e) 
    {
    debug_print("Jason DEBUG %s[%d] e->errCode=[%d]\n", __FUNCTION__, __LINE__,e->errCode);
    debug_print("Jason DEBUG %s[%d] e->statusCode=[%d]\n", __FUNCTION__, __LINE__,e->statusCode);
    debug_print("Jason DEBUG %s[%d] e->reasonPhrase=[%s]\n", __FUNCTION__, __LINE__,e->reasonPhrase);
    debug_print("Jason DEBUG %s[%d] e->errMsg=[%s]\n", __FUNCTION__, __LINE__,e->errMsg);
        res->errCode = error_code;
        res->statusCode = e->statusCode;

        memset(res->reasonPhrase, 0, sizeof(res->reasonPhrase));
        memset(res->errMsg, 0, sizeof(res->errMsg));

        snprintf(res->reasonPhrase, sizeof(res->reasonPhrase), "%s", e->reasonPhrase);

        if(msg == NULL)
            snprintf(res->errMsg, sizeof(res->errMsg), "%s", e->errMsg);
        else
            snprintf(res->errMsg, sizeof(res->errMsg), "%s:%s", e->errMsg, msg);

        return 0;
    }
    return 1;
}


/*****************************************************************
* NAME:    newObjectFromStack
* ---------------------------------------------------------------
* FUNCTION: create object and store
* INPUT:    ResponseEntry *rep
* OUTPUT:   json_object*
******************************************************************/
void* newObjectFromStack(ResponseEntry *rep)
{
    struct json_object *jobj;
    StackStruct *stackItem;
    stackItem=malloc(sizeof(StackStruct));
    stackItem->type=STACKTYPE_JSON;
    if(rep->stack) {
        jobj = json_object_new_object();
debug_print("Jason DEBUG %s[%d] [%d]\n", __FUNCTION__, __LINE__, stackSize(rep->stack));
        if(jobj){
            stackItem->item=jobj;
debug_print("Jason DEBUG %s[%d] [%x]\n", __FUNCTION__, __LINE__, jobj);
            if(stackPush(rep->stack, stackItem)) {
debug_print("Jason DEBUG %s[%d] [%d]\n", __FUNCTION__, __LINE__, stackSize(rep->stack));
                return jobj;
            }else {
                json_object_put(jobj);
            }
        }
    }
    else {
        free(stackItem);
    }
    return NULL;
}

/*****************************************************************
* NAME:    pushStringToStack
* ---------------------------------------------------------------
* FUNCTION: push created string to stack
* INPUT:    ResponseEntry *rep, char* string
* OUTPUT:   bool
******************************************************************/
bool pushStringToStack(ResponseEntry *rep, char *string)
{
    StackStruct *stackItem;
    stackItem=malloc(sizeof(StackStruct));
    stackItem->type=STACKTYPE_STR;
    if(rep->stack) {
        if(string){
            stackItem->item=string;
            if(stackPush(rep->stack, stackItem)) {
                return true;
            }else {
                return false;
            }
        }
    }
    else {
        free(stackItem);
    }
    return false;
}

/*****************************************************************
* NAME:    newObjectArrayFromStack
* ---------------------------------------------------------------
* FUNCTION: create object array and store
* INPUT:    ResponseEntry *rep
* OUTPUT:   json_object*
******************************************************************/
void* newObjectArrayFromStack(ResponseEntry *rep)
{
    struct json_object *jobj;
    StackStruct *stackItem;
    stackItem=malloc(sizeof(StackStruct));
    stackItem->type=STACKTYPE_JSON;
    if(rep->stack) {
        jobj = json_object_new_array();
debug_print("Jason DEBUG %s[%d] [%d]\n", __FUNCTION__, __LINE__, stackSize(rep->stack));
        if(jobj){
debug_print("Jason DEBUG %s[%d] [%x]\n", __FUNCTION__, __LINE__, jobj);
            stackItem->item=jobj;
            if(stackPush(rep->stack, jobj)) {
debug_print("Jason DEBUG %s[%d] [%d]\n", __FUNCTION__, __LINE__, stackSize(rep->stack));
                return jobj;
            }else {
                json_object_put(jobj);
            }
        }
    }
    else {
        free(stackItem);
    }
    return NULL;
}
/*****************************************************************
* NAME:    jsonTokenerParseFromStack
* ---------------------------------------------------------------
* FUNCTION: create object and store
* INPUT:    ResponseEntry *rep
* OUTPUT:   json_object*
******************************************************************/
void* jsonTokenerParseFromStack(ResponseEntry *rep, char* jsonStr)
{
    struct json_object *jobj;
    if(rep->stack) {
        jobj = json_tokener_parse(jsonStr);
debug_print("Jason DEBUG %s[%d] [%d]\n", __FUNCTION__, __LINE__, stackSize(rep->stack));
        if(jobj){
debug_print("Jason DEBUG %s[%d] [%x]\n", __FUNCTION__, __LINE__, jobj);
          if(stackPush(rep->stack, jobj)) {
debug_print("Jason DEBUG %s[%d] [%d]\n", __FUNCTION__, __LINE__, stackSize(rep->stack));
            return jobj;
          }else {
            json_object_put(jobj);
          }
        }
    }
    return NULL;
}
/*****************************************************************
* NAME:    printResponseResult
* ---------------------------------------------------------------
* FUNCTION: print HttpResponse
* INPUT:    ResponseEntry *rep
* OUTPUT:   int
******************************************************************/
char* printResponseResult(ResponseEntry *rep)
{
    struct json_object *jobj;

debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    jobj = newObjectFromStack(rep);
    json_object_object_add(jobj, "status_code", json_object_new_int(rep->res->statusCode));
    json_object_object_add(jobj, "reason_phrase", json_object_new_string(rep->res->reasonPhrase));
    json_object_object_add(jobj, "error_code", json_object_new_int(rep->res->errCode));
    json_object_object_add(jobj, "error_message", json_object_new_string(rep->res->errMsg));
    if ( rep->jobj != NULL )
    {
        //json_object_object_add(jobj, "data", rep->jobj);
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        json_object_object_add(jobj, "data",(struct json_object*) rep->jobj);
    }
    else
    {
        json_object_object_add(jobj, "data", NULL);
    }

    return json_object_to_json_string(jobj);
}