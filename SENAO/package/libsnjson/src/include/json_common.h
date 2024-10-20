#include <stdbool.h>
#include <json_object.h>
#include <json_tokener.h>
#include <api_response.h>

bool senao_json_object_get_boolean(struct json_object *jobj, char *obj_name, bool *res);
bool senao_json_object_get_string(struct json_object *jobj, char *obj_name, char *res);
bool senao_json_object_get_and_create_string(ResponseEntry *rep, struct json_object *jobj, char *obj_name, char **resStr);
bool senao_json_object_get_integer(struct json_object *jobj, char *obj_name, int *res);
int senao_json_iterator_parse(json_object* obj);
