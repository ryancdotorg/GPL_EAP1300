/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                     *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  ezjson                                                                       *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#ifndef _EZJSON_H_
#define _EZJSON_H_

#include "json.h"
#define JS_FAIL -9487
int js_equal(JsonNode *s1, JsonNode *s2);
int js_node_equal(JsonNode *s1, JsonNode *s2);
int js_value_equal(JsonNode *s1, JsonNode *s2);
JsonNode *js_dup(JsonNode *json);
char *jsontag_string(JsonTag t);
int js_array_cnt(JsonNode *json);
int js_member_cnt(JsonNode *json);
JsonNode *js_parse_str(char *s);
JsonNode *js_parse_file(char *filename);
void js_to_fp(JsonNode *json, FILE* fp);
void js_to_file(JsonNode *json, char *fname);
void js_to_file_hr(JsonNode *json, char *fname);
char *js_to_str(JsonNode *json);
char *js_to_str_hr(JsonNode *json);
void js_print(JsonNode *json);
void js_to_fp_hr(JsonNode *json, FILE *fp); // human readable
void js_print_hr(JsonNode *json);
void js_free(JsonNode *json);
JsonNode *js_get_path(JsonNode *json, char *path);
char *js_get_path_str(JsonNode *json, char *path);
char *js_get_path_strz(JsonNode *json, char *path);
bool js_get_path_bool(JsonNode *json, char *path);
int js_get_path_int(JsonNode *json, char *path);
double js_get_path_double(JsonNode *json, char *path);
JsonNode *js_set_path(JsonNode *json, char *path);
int js_set_path_value(JsonNode *json, char *path, void *val, JsonTag t);
int js_set_path_int(JsonNode *json, char *path, int val);
int js_set_path_double(JsonNode *json, char *path, double val);
int js_set_path_str(JsonNode *json, char *path, char *val);
int js_set_path_bool(JsonNode *json, char *path, bool val);
void js_union(JsonNode *dst, JsonNode *src);
void js_join(JsonNode *dst, JsonNode *src);
void js_free_path(JsonNode *json, char *path);
void js_free_js(JsonNode *dst, JsonNode *jref);
void js_idx_union(JsonNode *dst, JsonNode *src);
JsonNode *js_get_idx_js(JsonNode *dst, JsonNode *jref);
JsonNode *js_get_js(JsonNode *dst, JsonNode *jref);
JsonNode *js_idx_set_path(JsonNode *json, char *path);
JsonNode *js_idx_get_path(JsonNode *json, char *path);
char *js_idx_get_path_str(JsonNode *json, char *path);
char *js_idx_get_path_strz(JsonNode *json, char *path);
bool js_idx_get_path_bool(JsonNode *json, char *path);
int js_idx_get_path_int(JsonNode *json, char *path);
double js_idx_get_path_double(JsonNode *json, char *path);
int js_idx_set_path_value(JsonNode *json, char *path, void *val, JsonTag t);
int js_idx_set_path_value(JsonNode *json, char *path, void *val, JsonTag t);
int js_idx_set_path_int(JsonNode *json, char *path, int val);
int js_idx_set_path_double(JsonNode *json, char *path, double val);
int js_idx_set_path_str(JsonNode *json, char *path, char *val);
int js_idx_set_path_bool(JsonNode *json, char *path, bool val);
#endif
