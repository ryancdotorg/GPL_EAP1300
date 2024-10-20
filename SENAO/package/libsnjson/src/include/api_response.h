/**
 * @file api_response.h
 * @author LEO, Areal
 * @date 8 Feb 2018
 * @brief define rsponse content for API request
 *
 * write more here
 *
 */
#ifndef _API_RESPONSE_H_
#define _API_RESPONSE_H_
#include <stdlib.h>
#include <fcgi_stdio.h>
#include <stdlib.h>
#include <api_tokens.h>
#include <wireless_tokens.h>
#include <variable/api_wireless.h>
#include <stack.h>
#include <json_object.h>
#include <json_tokener.h>
#include <assert.h>


/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/

#define HTTP_STATUS_CODE_DEFAULT 200
#define HTTP_ERROR_CODE_DEFAULT 0
#define REASONPHRASE_LENGTH 64
#define ERRMSG_LENGTH 256
#define T_NUM_OF_ELEMENTS(x) (sizeof(x)/sizeof(x[0]))
#define RET_GEN_ERRORMSG(RES,ERRORCODE, MSG) {assert(genErrorMessage(RES,ERRORCODE, MSG)==0); return ERRORCODE;} // return error code and generate error message

/**
 * @brief Response Staus info
 *
 * write more here
 *
 */
typedef struct _ResponseStatus
{
    int statusCode;                         /**< http status code ResponseStatus#statusCode */
    char reasonPhrase[REASONPHRASE_LENGTH]; /**< description of http status code ResponseStatus#reasonPhrase */
    int errCode;                            /**< senao define error code ResponseStatus#errCode */
    char errMsg[ERRMSG_LENGTH];             /**< description of senao define error code ResponseStatus#errMsg */
} ResponseStatus;


/**
 * @brief Response Entry
 *
 * Resposne entry includes status info and json data.
 *
 */
typedef struct _ResponseEntry
{
    ResponseStatus *res;     /**< Response status info ResponseEntry#res */
    void *jobj;              /**< data content : json object ResponseEntry#jobj */
    Stack *stack;            /**< use for allocate and destroy json object ResponseEntry#stack */
    Stack *strStack;            /**< use for allocate and destroy json object ResponseEntry#stack */
} ResponseEntry;

/**
 * @brief simple key value mapping table
 *
 * write more here
 *
 */
typedef struct {
    int key;
    char *value;
} keyvalue;


/**
 * @brief for api request handling status
 *
 * write more here
 *
 */
typedef enum {
    API_SUCCESS         = 0,        /**< success. */
    API_UNKNOWN_ACTION,             /**< api url not defined yet. */
    API_INVALID_NUMBER_OF_ARGUMENTS,/**< invalid number of arguments. */
    API_INVALID_ARGUMENTS,          /**< invalid value of arguments. */
    API_INVALID_DATA_TYPE,          /**< invalid data type. */
    API_TABLE_FULL,                 /**< over table size. */
    API_INTERNAL_ERROR,             /**< unexpeted programing logic error. */
    API_INVALID_ACCOUNT,            /**< invalid account. */
    API_INVALID_TOKEN,              /**< invalid token. */
    API_PROCESSING,                 /**< processing */
    API_INTERNET_ERROR,             /**< no internet */
    API_NO_PRODUCER,                /**< no producer, must execute post to produce data */
    API_SERVICE_ERROR,              /**< no service */
    API_REMOTE_SERVER_ERROR,
    API_CABLE_UNPLUGGED,            /**< network cable unplugged */
    API_PROXY_TIMEOUT,              /**< server proxy time out. */
    API_FILE_MD5SUM_ERROR = 100,
    API_FILE_NOT_EXIST,
    API_FILE_SIZE_TOO_LARGE,
    API_FILE_SIZE_TOO_SMALL,
    API_FILE_SIZE_NOT_EQUAL,
    API_FILE_AUTH_FAILED,
    API_FILE_PATH_ERROR,
    API_UNKNOWN_ERROR = 520         /**< unknown error. */
}error_code_t;

static keyvalue http_status[] = {
    { 100, "Continue" },
    { 101, "Switching Protocols" },
    { 102, "Processing" }, /* WebDAV */
    { 200, "OK" },
    { 201, "Created" },
    { 202, "Accepted" },
    { 203, "Non-Authoritative Information" },
    { 204, "No Content" },
    { 205, "Reset Content" },
    { 206, "Partial Content" },
    { 207, "Multi-status" }, /* WebDAV */
    { 300, "Multiple Choices" },
    { 301, "Moved Permanently" },
    { 302, "Found" },
    { 303, "See Other" },
    { 304, "Not Modified" },
    { 305, "Use Proxy" },
    { 306, "(Unused)" },
    { 307, "Temporary Redirect" },
    { 400, "Bad Request" },
    { 401, "Unauthorized" },
    { 402, "Payment Required" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
    { 405, "Method Not Allowed" },
    { 406, "Not Acceptable" },
    { 407, "Proxy Authentication Required" },
    { 408, "Request Timeout" },
    { 409, "Conflict" },
    { 410, "Gone" },
    { 411, "Length Required" },
    { 412, "Precondition Failed" },
    { 413, "Request Entity Too Large" },
    { 414, "Request-URI Too Long" },
    { 415, "Unsupported Media Type" },
    { 416, "Requested Range Not Satisfiable" },
    { 417, "Expectation Failed" },
    { 422, "Unprocessable Entity" }, /* WebDAV */
    { 423, "Locked" }, /* WebDAV */
    { 424, "Failed Dependency" }, /* WebDAV */
    { 426, "Upgrade Required" }, /* TLS */
    { 500, "Internal Server Error" },
    { 501, "Not Implemented" },
    { 502, "Bad Gateway" },
    { 503, "Service Not Available" },
    { 504, "Gateway Timeout" },
    { 505, "HTTP Version Not Supported" },
    { 507, "Insufficient Storage" }, /* WebDAV */
    { -1, NULL }
};

static ResponseStatus error_code_mapping[] = {
    { 200, "OK", API_SUCCESS, "" },
    { 202, "Processing", API_PROCESSING, "API IS PROCESSING" },
    { 404, "Not Found", API_UNKNOWN_ACTION, "API URL NOT MATCH" },
    { 400, "Bad Request", API_INVALID_NUMBER_OF_ARGUMENTS, "INVALID NUMBER OF ARGUMENTS" },
    { 400, "Bad Request", API_INVALID_ARGUMENTS, "INVALID VALUE OF ARGUMENTS" },
    { 400, "Bad Request", API_INVALID_DATA_TYPE, "INVALID DATA TYPE" },
    { 400, "Bad Request", API_TABLE_FULL, "TABLE FULL" },
    { 400, "Bad Request", API_NO_PRODUCER, "NO PRODUCER" },
    { 401, "Unauthorized", API_INVALID_ACCOUNT, "WRONG USERNAME OR PASSWORD" },
    { 401, "Unauthorized", API_INVALID_TOKEN, "INVALID TOKEN" },
    { 500, "Internal Server Error", API_INTERNAL_ERROR, "API INTERNAL ERROR" },
    { 503, "Service Not Available", API_SERVICE_ERROR, "SERVICE NOT AVAILABLE" },
    { 504, "Gateway Timeout", API_CABLE_UNPLUGGED   , "CABLE UNPLUGGED" },
    { 504, "Gateway Timeout", API_PROXY_TIMEOUT, "PROXY TIME OUT" },
    { 504, "Gateway Timeout", API_REMOTE_SERVER_ERROR, "REMOTE SERVER ERROR" },
    { 504, "Gateway Timeout", API_INTERNET_ERROR, "INTERNET ERROR" },
    { 520, "Unknown Error", API_FILE_MD5SUM_ERROR, "FILE MD5SUM ERROR" },
    { 520, "Unknown Error", API_FILE_NOT_EXIST, "FILE NOT EXIST" },
    { 520, "Unknown Error", API_FILE_SIZE_NOT_EQUAL, "FILE SIZE NOT EQUAL" }
};

/**
 * @brief Create Response object
 *
 * @return  a pointer to the new create ResponseEntry object if succeeded, NULL if fail.
 *
 * Allocate require object include ResponseStatus, ResponseEntry, Stack...
 *
 * @code
 * ResponseEntry *rep = Response_create();
 * @endcode
 *
 * @see Response_destroy
 */
ResponseEntry* Response_create();

/**
 * @brief Destroy the object that created by Response_create
 *
 * @param rep a pointer to the ResponseEntry object.
 * @return  0 if succeeded, otherwise fail.
 *
 * @code
 * ResponseEntry *rep = Response_create();
 * Response_destroy(rep);
 * @endcode
 *
 * @see Response_create
 */
int Response_destroy(ResponseEntry *rep);

/**
 * @brief generate info for handling status
 *
 * @param res a pointer to the ResponseStatus object.
 * @param error_code error code defined by SENAO.
 * @param msg extra message.
 *
 * @return  0 if succeeded, otherwise fail.
 *
 * @code
 * ResponseEntry *rep = Response_create();
 * genErrorMessage(rep->res, API_SUCCESS, NULL); //if don't need extra message
 * genErrorMessage(rep->res, API_INVALID_ARGUMENTS, "demo_extra_message");
 * Response_destroy(rep);
 * @endcode
 *
 * @see printResponseResult
 * @see error_code_t
 *
 */
int genErrorMessage(ResponseStatus *res, int error_code, char* msg);

/**
 * @brief print ResponseStatus and json object
 *
 * @param rep a pointer to the ResponseEntry object.
 * @return  0 if succeeded, otherwise fail.
 *
 * @code
 * ResponseEntry *rep = Response_create();
 * genErrorMessage(rep->res, API_SUCCESS, NULL);
 * genErrorMessage(rep->res, API_INVALID_ARGUMENTS, "demo_extra_message");
 * printResponseResult(rep);
 * Response_destroy(rep);
 * @endcode
 *
 * @see genErrorMessage
 */
char* printResponseResult(ResponseEntry *rep);

/**
 * @brief new json object and store it into stack
 *
 * @param rep ResponseEntry.
 * @return a pointer to create json object if succeeded, NULL if fail.
 *
 *
 * @code
 * ResponseEntry *rep = Response_create();
 * struct json_object* jobj = newObjectFromStack(rep);
 * jobj = getSomeUserDefineJSON();
 * rep->jobj = jobj;
 * Response_destroy(rep);
 * @endcode
 *
 * @see newObjectArrayFromStack
 *
 */
void* newObjectFromStack(ResponseEntry *rep);

bool pushStringToStack(ResponseEntry *rep, char* string);

/**
 * @brief new json object array and store it into stack
 *
 * @param rep ResponseEntry.
 * @return a pointer to new create json object array if succeeded, NULL if fail.
 *
 * @code
 * ResponseEntry *rep = Response_create();
 * struct json_object* jobj = newObjectFromStack(rep);
 * jobj = getSomeUserDefineJSON();
 * rep->jobj = jobj;
 * Response_destroy(rep);
 * @endcode
 *
 * @see newObjectFromStack
 *
 */
void* newObjectArrayFromStack(ResponseEntry *rep);
void* jsonTokenerParseFromStack(ResponseEntry *rep, char* jsonStr);

#endif
