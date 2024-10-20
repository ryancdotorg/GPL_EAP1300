/**
 * @file api_json_check.h
 * @author LEO, Areal
 * @date 8 Feb 2018
 * @brief define json validate process for API request
 *
 * write more here
 *
 */
#ifndef _API_JSON_CHECK_H_
#define _API_JSON_CHECK_H_
#include <stdlib.h>
#include <fcgi_stdio.h>
#include <stdlib.h>
#include <api_tokens.h>
#include <wireless_tokens.h>
#include <variable/api_wireless.h>
#include <stack.h>
#include <api_response.h>
#include <json_object.h>
#include <json_tokener.h>
#include <assert.h>
#include <json_ssid.h>
#include <json_wireless.h>
#include <json_common.h>
#include <arraylist.h>
#include "json.h"

/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/

/**
 * @brief check function and sub objects of json object
 *
 * write more here
 *
 */
typedef struct _JSONCheckEntry
{
    const char *json_object_name;                               /**< json object name JSONCheckEntry#json_object_name */
    int (*cb_function)(struct json_object*, ResponseEntry*);   /**< check function of json object JSONCheckEntry#cb_function */
    char** child_objects;                                       /**< child object name JSONCheckEntry#child_objects */
    int child_objects_size;                                     /**< child object size JSONCheckEntry#child_objects_size */
} JSONCheckEntry;

/**
 * @brief expand inner objects recursively to just one level mapping
 *
 * @param obj a json object which wnat to be expand.
 * @param obj_name josn object name so that we can mapping the content by our define table. 
 * @param expand a json object that get the expand result
 * @return  0 if succeeded, otherwise is fail.
 *
 * After this process, we can get the inner object by key value mapping.
 * Just do it once, and save the extra effort after that.
 * But it still have many issues.
 * 1. json object array not support yet.
 * 2. sub object name can't have the same name.
 *
 * @code
 * @endcode
 *
 */
int senao_json_object_expand(json_object* obj, const char* obj_name, json_object* expand);


/**
 * @brief validate any json object that we define.
 *
 * @param res a a pointer to ResponseStatus object, use for fill in validate result.
 * @param query_str the string with valid json format.
 * @param json_object_name the name of json obejct which we defined.
 * @return  0 if succeeded, otherwise is fail.
 *
 * Plays the controller role in validate process.
 * It includes these part:
 *
 * 1.parser #senao_json_object_expand
 *  convert the json string to the json object which is expand recursively.
 *
 * 2.validate #json_content_examination
 *  excute the json validate process, get the json object relative check function and excute recursively. 
 *
 * @code
 * @endcode
 *
 * @see senao_json_object_expand
 * @see json_content_examination
 *
 */
int senao_json_validator(ResponseEntry *rep, char *query_str, char* josn_object_name);

/**
 * @brief a simple mapping for name and JSONCheckEntry.
 *
 * @param name_of_object the name of json object that we defined.
 * @param mapping_table the array of JSONCheckEntry for mapping its check and child objects.
 * @param mapping_table_size the array size of JSONCheckEntry.
 * @return  a pointer to JSONCheckEntry object if succeeded, NULL if fail.
 *
 *
 * @code
 * @endcode
 *
 * @see json_content_examination
 *
 */
JSONCheckEntry* findJSONCheckEntry(char* name_of_object, JSONCheckEntry* mapping_table, int mapping_table_size);

/**
 * @brief get json object check function and excute recursively.
 *
 * @param name_of_object the name of json object that we defined.
 * @param jobj a pointer to json ojbect which expand already.
 * @param res a pointer to ResponseStatus object which use for fill info during validation.
 * @return  a pointer to JSONCheckEntry object if succeeded, NULL if fail.
 *
 *
 * @code
 * @endcode
 *
 * @see json_content_examination
 *
 */
int json_content_examination(char* name_of_object, struct json_object *jobj, ResponseEntry *rep);
#endif
