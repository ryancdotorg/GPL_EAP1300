#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <json_object.h>
#include <json_tokener.h>
#include <json_common.h>
#include <arraylist.h>
#include "json.h"
#include <assert.h>

bool senao_json_object_get_boolean(struct json_object *jobj, char *obj_name, bool *res)
{
    struct json_object *jobj_bool;
    char boolean_str[16];
    bool result;

    *res = FALSE;
    result = FALSE;

    if((jobj_bool = json_object_object_get(jobj, obj_name)))
    {
        sprintf(boolean_str, "%s", json_object_get_string(jobj_bool));

        /* Get the setting successfully. */
        result = TRUE;

        if(!strcasecmp("true", boolean_str))
        {
            *res = TRUE;
        }
        else if(!strcasecmp("Enable", boolean_str))
        {
            *res = TRUE;
        }
    }

    return result;
}

bool senao_json_object_get_string(struct json_object *jobj, char *obj_name, char *res)
{
    struct json_object *jobj_str;
    bool result;

    result = FALSE;

    if((jobj_str = json_object_object_get(jobj, obj_name)))
    {
        sprintf(res, "%s", json_object_get_string(jobj_str));

        /* Get the setting successfully. */
        result = TRUE;
    }

    return result;
}

bool senao_json_object_get_and_create_string(ResponseEntry *rep, struct json_object *jobj, char *obj_name, char **resStr)
{
    struct json_object *jobj_str;
    bool result;
    char *str=NULL;
    unsigned int length = 0;

    result = FALSE;

    if((jobj_str = json_object_object_get(jobj, obj_name)))
    {
        length = strlen(json_object_get_string(jobj_str));

        debug_print("cl %s:%d key[%s] str[%s] length[%d] ###\n", __FUNCTION__, __LINE__, obj_name, json_object_get_string(jobj_str), length);

        if ( length > 0 )
        {
            /* Get the setting successfully. */
            result = TRUE;
            str = malloc(sizeof(char)*length+1);
            memset(str,0,sizeof(char)*length+1);
            snprintf(str, sizeof(char)*length+1, "%s",json_object_get_string(jobj_str));
            debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        }
        else
        {
            result = false;
            length = 0;
            str = malloc(sizeof(char)*length+1);
            memset(str,0,sizeof(char)*length+1);
            snprintf(str, sizeof(char)*length+1, "");
            debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        }

        /* Free obj */
        //json_object_put(jobj_str);

        *resStr = str;

        pushStringToStack(rep, *resStr);
    }
    else
    {
        result = false;
        length = 0;
        str = malloc(sizeof(char)*length+1);
        memset(str,0,sizeof(char)*length+1);
        snprintf(str, sizeof(char)*length+1, "");
        *resStr = str;
        pushStringToStack(rep, *resStr);
    }

    return result;
}

bool senao_json_object_get_integer(struct json_object *jobj, char *obj_name, int *res)
{
    struct json_object *jobj_int;
    bool result;

    result = FALSE;

    if((jobj_int = json_object_object_get(jobj, obj_name)))
    {
        *res = json_object_get_int(jobj_int);

        /* Get the setting successfully. */
        result = TRUE;
    }

    return result;
}

int senao_json_iterator_parse(json_object* obj)
{
    struct json_object_iterator it;
    struct json_object_iterator itEnd;
    it = json_object_iter_begin(obj);
    itEnd = json_object_iter_end(obj);

    while (!json_object_iter_equal(&it, &itEnd)) {
        //debug_print("%s %s\n",
        //        json_object_iter_peek_name(&it), json_object_iter_peek_value(&it));
        const char *name = json_object_iter_peek_name(&it);
        struct json_object *value = json_object_iter_peek_value(&it);
        switch (json_object_get_type(value)) {
            case json_type_int:
                {
                    int32_t value_as_int = json_object_get_int(value);
                    debug_print("name: %s, value: %d\n", name, value_as_int);
                    break;
                }
            case json_type_boolean:
                {
                    assert(json_object_is_type(value, json_type_boolean));
                    json_bool value_as_boolean = json_object_get_boolean(value);
                    debug_print("name: %s, value: %s\n", name, value_as_boolean ? "true" : "false");
                    break;
                }
            case json_type_double:
                {
                    assert(json_object_is_type(value, json_type_double));
                    double value_as_double = json_object_get_double(value);
                    debug_print("name: %s, value: %lf\n", name, value_as_double);
                    break;
                }
            case json_type_string:
                {
                    assert(json_object_is_type(value, json_type_string));
                    const char *value_as_string = json_object_get_string(value);
                    debug_print("name: %s, value: %s\n", name, value_as_string);
                    break;
                }
            case json_type_object:
                {
                    debug_print("name: %s object\n", name);
                    senao_json_iterator_parse(value);
                    break;
                }
            case json_type_array:
                {
                    debug_print("name: %s object array\n", name);
                    json_object* obj_array;
                    int arraylen = 0, i = 0;
                    obj_array = json_object_object_get(obj, name);
                    arraylen = json_object_array_length(obj_array);

                    for (i = 0; i < arraylen; i++) {
                        senao_json_iterator_parse(json_object_array_get_idx(obj_array, i));
                    }
                    break;
                }
            case json_type_null:
                break;
        }
        json_object_iter_next(&it);
    }
    return 0;

}

