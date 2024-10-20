#include <ext_api.h>

ApiEntry extensionTable[] = 
{
    //  1       2         3          4                    5         6     7      METHOD  callback_function
    {"test", "get",    "",       "",                 "",       "",   "",   "GET",   0x3, ext_getcallback},
    {"test", "set",    "",       "",                 "",       "",   "",   "POST",  0x3, ext_setcallback},
};

int extTableLen = (sizeof(extensionTable)/sizeof(extensionTable[0]));

//http://192.168.1.1:8000/api/test/test
//{ "status_code": 200, "reason_phrase": "OK", "error_code": 0, "error_message": "", "data": { "TEST": "SUCCESS" } }
int ext_getcallback(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;
    jobj = json_object_new_object();
    json_object_object_add(jobj, "TEST", json_object_new_string("SUCCESS"));
    rep->jobj = jobj;
    RET_GEN_ERRORMSG(rep->res, API_SUCCESS, NULL);
}

//curl -v -k -X POST "http://192.168.1.1:8000/api/test/set" -H "accept: */*" -H "Content-Type: application/json" -H "Authorization: Basic YWRtaW46YWRtaW4=" -d "{\"TEST\":100}"
int ext_setcallback(HTTPEntry packet, ResponseEntry *rep)
{
    int test = 0;
    struct json_object *jobj;
    if(NULL != packet.body)
    {
        if((jobj = json_tokener_parse(packet.body)))
        {
            senao_json_object_get_integer(jobj, "TEST", &test);
            json_object_put(jobj);
        }
    }
    return 0;
}
