#include <api_json_check.h>

/*-----------------------------------------------------------------------*/
/*                        GLOBAL VARIABLES                               */
/*-----------------------------------------------------------------------*/

// move to some check_json_ssid.h etc
int check_wpa(struct json_object *jobj, ResponseEntry *rep);
int check_radius_server(struct json_object *jobj, ResponseEntry *rep);
int check_accounting_server(struct json_object *jobj, ResponseEntry *rep);
int check_security(struct json_object *jobj, ResponseEntry *rep);



/*-----------------------------------------------------------------------*/
/*                    json object child mapping table                    */
/*-----------------------------------------------------------------------*/
char* security_mapping_table[] = {
    "wpa",
    "radius_server",
    "accounting_server"
};

JSONCheckEntry object_check_mapping_table[] =
{
    {"wpa", check_wpa, NULL, 0},
    {"radius_server", check_radius_server, NULL, 0},
    {"accounting_server", check_accounting_server, NULL, 0},
    {"security", check_security, security_mapping_table, T_NUM_OF_ELEMENTS(security_mapping_table)}
};


/*-----------------------------------------------------------------------*/
/*                            FUNCTION                              */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                    json object check function                    */
/*-----------------------------------------------------------------------*/
int check_wpa(struct json_object *jobj, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    int wpa_group_rekey = 0;
    char *passphrase=NULL;
    ResponseStatus *res = rep->res;

/*
 *  key_interval integer
 *  passphrase string
 *
 * */
    if(jobj)
    {
        senao_json_object_get_integer(jobj, "key_interval", &wpa_group_rekey);
        senao_json_object_get_and_create_string(rep, jobj, "passphrase", &passphrase);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, (char *)__FUNCTION__);
    }
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "KEY INTERVAL");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int check_radius_server(struct json_object *jobj, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    int interval = 0, server1Port = 0;
    char *server1Ip=NULL, *server1Secret=NULL;
    ResponseStatus *res = rep->res;
/*
 *  interval integer
 *  server1Ip string
 *  server1Port integer
 *  server1Secret string
 * */
    if(jobj)
    {
        senao_json_object_get_integer(jobj, "interval",&(interval));
        senao_json_object_get_and_create_string(rep, jobj, "server1Ip", &server1Ip);
        senao_json_object_get_integer(jobj, "server1Port",&(server1Port));
        senao_json_object_get_and_create_string(rep, jobj, "server1Secret", &server1Secret);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, (char *)__FUNCTION__);
    }

//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "INTERVAL");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 IP");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 PORT");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 SECRET");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int check_accounting_server(struct json_object *jobj, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    int server1Port = 0;
    int nasid_enable = 0;
    bool acct_enable = 0;
    char *server1Ip=NULL, *nasIp=NULL, *server1Secret=NULL, *nasid=NULL;
    ResponseStatus *res = rep->res;
/*
 *  enable boolean
 *  nasId integer
 *  nasIpAddr string
 *  server1Ip string
 *  server1Port integer
 *  server1Secret string
 *
 * */
    if(jobj)
    {
        senao_json_object_get_boolean(jobj, "enable",&(acct_enable));
        senao_json_object_get_and_create_string(rep, jobj, "nasId", &nasid);
        senao_json_object_get_and_create_string(rep, jobj, "nasIpAddr", &nasIp);
        senao_json_object_get_and_create_string(rep, jobj, "server1Ip", &server1Ip);
        senao_json_object_get_integer(jobj, "server1Port",&(server1Port));
        senao_json_object_get_and_create_string(rep, jobj, "server1Secret", &server1Secret);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, (char *)__FUNCTION__);
    }

//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCOUNTING SERVER ENABLE");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAS ID");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAS IP");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 IP");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 PORT");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 SECRET");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int check_security(struct json_object *jobj, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    int  encr_type = 0;
    char *auth_type=NULL, *encryption=NULL;
    ResponseStatus *res = rep->res;

/*
 * encryption string
 * auth_type string
 *
 * */
    if(jobj)
    {
        senao_json_object_get_and_create_string(rep, jobj, "encryption", &encryption);
        senao_json_object_get_and_create_string(rep, jobj, "auth_type", &auth_type);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, (char *)__FUNCTION__);
    }

//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENCRYPTION");
//    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH TYPE");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

/*-----------------------------------------------------------------------*/
/*                  api check  utility function                          */
/*-----------------------------------------------------------------------*/

JSONCheckEntry* findJSONCheckEntry(char* name_of_object, JSONCheckEntry* mapping_table, int mapping_table_size)
{
    int i;
    if(name_of_object == NULL)
        return NULL;
    for(i=0;i<mapping_table_size;++i)
    {
        if(strcmp(name_of_object, mapping_table[i].json_object_name) == 0)
        {
            return &mapping_table[i];
        }
    }
    return NULL;
}

int senao_json_object_expand(json_object* obj, const char* obj_name, json_object* expand_object)
{
    struct json_object_iterator it;
    struct json_object_iterator itEnd;
    it = json_object_iter_begin(obj);
    itEnd = json_object_iter_end(obj);

    json_object_object_add(expand_object, obj_name, obj);

    while (!json_object_iter_equal(&it, &itEnd)) {
        //debug_print("%s %s\n",
        //        json_object_iter_peek_name(&it), json_object_iter_peek_value(&it));
        const char *name = json_object_iter_peek_name(&it);
        struct json_object *value = json_object_iter_peek_value(&it);
        switch (json_object_get_type(value)) {
            case json_type_object:
                {
                    debug_print("name: %s object\n", name);
                    senao_json_object_expand(value, name, expand_object);
                    break;
                }
            case json_type_array:
                {
 //                   debug_print("name: %s object array\n", name);
 //                   json_object_object_add(expand_object, name, value);
 //                   json_object* obj_array;
 //                   int arraylen = 0, i = 0;
 //                   obj_array = json_object_object_get(obj, name);
 //                   arraylen = json_object_array_length(obj_array);

 //                   for (i = 0; i < arraylen; i++) {
 //                       senao_json_object_expand(json_object_array_get_idx(obj_array, i), expand_object);
 //                   }
                    break;
                }
            case json_type_null:
                break;
        }
        json_object_iter_next(&it);
    }
    return 0;

}

int json_content_examination(char* name_of_object, struct json_object *jobj, ResponseEntry *rep)
{
    int k;
    int result = 0;
    JSONCheckEntry* jce;
    struct json_object *target_jobj;
    ResponseStatus *res = rep->res;

    jce = findJSONCheckEntry(name_of_object, object_check_mapping_table, T_NUM_OF_ELEMENTS(object_check_mapping_table));

    if(jce)
    {

        debug_print("Jason DEBUG %s[%d] jce->json_object_name = [%s]\n", __FUNCTION__, __LINE__, jce->json_object_name);
        if(jce->json_object_name)
        {
             target_jobj = json_object_object_get(jobj, jce->json_object_name);
             if(NULL == target_jobj)
             {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, name_of_object);
                goto end;
             }
        }

        if(jce->cb_function)
        {

            if(result = jce->cb_function(target_jobj, rep))
                goto end;

        }

        for(k=0;k< jce->child_objects_size;k++)
        {
            if(result = json_content_examination(jce->child_objects[k], jobj, rep))
                goto end;
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, name_of_object);
    }

end:
        return result;

}


int senao_json_validator(ResponseEntry *rep, char *query_str, char* json_object_name)
{
    struct json_object *jobj;
    struct json_object *jobj_expand = json_object_new_object();
    ResponseStatus *res = rep->res;

    if((jobj = jsonTokenerParseFromStack(rep, query_str)))
    {
        senao_json_object_expand(jobj, json_object_name,jobj_expand);
        json_content_examination(json_object_name, jobj_expand, rep);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "INVALID JSON FORMAT");
    }


    return API_SUCCESS;

}
