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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "ezjson.h"

/**
 * @brief chomp
 *
 * handle preprocess text data for json library
 *
 * @param *s input data
 * @return result string
 */
static char *chomp(char *s) //function taken from CCAN JSON example
{
    char *e;
    if (s == NULL || *s == 0)
        return s;

    e = strchr(s, 0);
    if (e[-1] == '\n')
        *--e = 0;
    return s;
}

/**
 * @brief js_equal, compare 2 JsonNode recursively
 *
 * Compare 2 JsonNode recursively, check if two node identically 
 * Base on s1, check if s2 has the same node
 *
 * @param *s1 node to be compared
 * @param *s2 check if there is same node is s2
 * @return 0 for equal, others if different
 *
 */
int js_equal(JsonNode *s1, JsonNode *s2)
{
    if (s1 == NULL || s2 == NULL){
        printf("null compare\n");
        return -1;
    }
    if (s1->tag != s2->tag)
        return -2;

    if ((s1->key != NULL && s2->key != NULL) && // first object, key would be null
        (strcmp(s1->key, s2->key))){ // not equal return
        //printf("[%s] s1->key %s s2->key:%s\n",jsontag_string(s1->tag), s1->key, s2->key);
        return -5;
    }

    JsonNode *t1, *t2;
    int found = 0;
//    printf("s1 tag:%s s2 tag:%s\n", jsontag_string(s1->tag), jsontag_string(s2->tag));
	switch(s1->tag)
	{
		case ezJSON_BOOL:
            return s1->bool_ - s2->bool_;
		case ezJSON_STRING:
            if (s1->string_ == NULL && s2->string_ == NULL) // null equal
                return 0;
            if (s1->string_ == NULL || s2->string_ == NULL) // one null, not equal
                return -3;
            return strcmp(s1->string_, s2->string_);
		case ezJSON_NUMBER:
            return s1->number_ - s2->number_;
		case ezJSON_OBJECT:
		case ezJSON_ARRAY:  // array and object
            // equal, check members
            json_foreach(t1, s1){
                found = 0;
                json_foreach(t2, s2){
                    if (js_equal(t1, t2)==0){ 
                        found = 1;
                        break;
                    }
                }
                if(found == 0) return -6; // no child or no the same child
            }
            return 0;
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            return -9;
	}
}

/**
 * @brief js_node_equal, compare 2 single JsonNode
 *
 * Compare 2 JsonNode without check their children 
 * Base on s1, check if s2 has the same node
 *
 * @param *s1 node to be compared
 * @param *s2 check if there is same node is s2
 * @return 0 for equal, others if different
 *
 */
int js_node_equal(JsonNode *s1, JsonNode *s2)
{
    if (s1 == NULL || s2 == NULL){
        printf("null compare\n");
        return -1;
    }
    if (s1->tag != s2->tag)
        return -2;

    if ((s1->key != NULL && s2->key != NULL) && // first object, key would be null
        (strcmp(s1->key, s2->key))) {// not equal return
        //printf("[%s] s1->key %s s2->key:%s\n",jsontag_string(s1->tag), s1->key, s2->key);
        return -5;
    }

    JsonNode *t1, *t2;
	switch(s1->tag)
	{
		case ezJSON_BOOL:
            return s1->bool_ - s2->bool_;
		case ezJSON_STRING:
            if (s1->string_ == NULL && s2->string_ == NULL) // null equal
                return 0;
            if (s1->string_ == NULL || s2->string_ == NULL) // one null, not equal
                return -3;
            return strcmp(s1->string_, s2->string_);
		case ezJSON_NUMBER:
            return s1->number_ - s2->number_;
		case ezJSON_OBJECT:
		case ezJSON_ARRAY:  // array and object
            return 0; // node_equal don't care children
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            return -9;
	}
}

/**
 * @brief js_value_equal, check if tag/values in 2 JsonNode equal or not
 *
 * Check if tag/values in 2 JsonNode equal or not
 *
 * @param *s1 node to be checked
 * @param *s2 node to be checked
 * @return 0 for equal, others if different
 *
 */
int js_value_equal(JsonNode *s1, JsonNode *s2)
{
    if (s1 == NULL || s2 == NULL){
        printf("null compare\n");
        return -1;
    }
    if (s1->tag != s2->tag)
        return -2;

    JsonNode *t1, *t2;
	switch(s1->tag)
	{
		case ezJSON_BOOL:
            return s1->bool_ - s2->bool_;
		case ezJSON_STRING:
            if (s1->string_ == NULL && s2->string_ == NULL) // null equal
                return 0;
            if (s1->string_ == NULL || s2->string_ == NULL) // one null, not equal
                return -3;
            return strcmp(s1->string_, s2->string_);
		case ezJSON_NUMBER:
            return s1->number_ - s2->number_;
		case ezJSON_OBJECT:
		case ezJSON_ARRAY:  // array and object
            return 0; // node_equal don't care children
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            return -9;
	}
}

/**
 * @brief js_idx_match, check if JsonNode exist in another Nodes' children
 *
 * Check if idx with key exist in js's child
 *
 * @param *js node's child to be checked
 * @param *key key to check
 * @param *idx key node to check
 * @return 0 if match, others if not
 */
int js_idx_match(JsonNode *js, char *key, JsonNode *idx)
{
    JsonNode *j, *k;
    int found;
    if ((js->tag != ezJSON_ARRAY && js->tag != ezJSON_OBJECT) || key == NULL)
        return -1;
    k = js_dup(idx);
    free(k->key);
    k->key = strdup(key);
    json_foreach(j, js){
        if (!js_node_equal(j, k)){
            free(k->key);
            js_free(k);
            return 0; // match
        }
    }
    free(k->key);
    js_free(k);
    return 1; // not match
}

/**
 * @brief js_value_cmp, check if value in json equal to non-type value
 *
 * Check if value json equal to value, check type autumatically
 *
 * @param *json node to be checked
 * @param *value value be compare
 * @return 0 for equal, others if different
 *
 */
int js_value_cmp(JsonNode *json, char *value)
{
	if (json == NULL || value == NULL)
		return -1;
	if (json->tag == ezJSON_STRING){
		if (json->string_ == NULL) 
			return -1;
		return strcmp(json->string_, value);
	}
	else if (json->tag == ezJSON_NUMBER){
		return json->number_ - atoi(value);
	}
	else if (json->tag == ezJSON_BOOL){
		if (json->bool_ == atoi(value))
			return 0;
		else 
			return -1;
	}
	else
		return 1;
}

/**
 * @brief jsontag_string, print tag of a json node
 *
 * Print node tag of a json node
 *
 * @param t json tag to convert and print
 * @return output tag string
 */
char *jsontag_string(JsonTag t)
{
	switch(t)
	{
		case ezJSON_NULL:
			return "null";
		case ezJSON_BOOL:
			return "bool";
		case ezJSON_STRING:
			return "string";
		case ezJSON_NUMBER:
			return "integer";
		case ezJSON_ARRAY:
			return "array";
		case ezJSON_OBJECT:
			return "object";
		default:
			return "unknown";
	}
}

/**
 * @brief js_array_cnt return json array member number
 *
 * traverse Json Node with array tag, return the number of members
 * 
 * @param *json node to check
 * @return -1 if fail, others for the total count
 */
int js_array_cnt(JsonNode *json)
{
	if (json == NULL)
		return -1;
	if (json->tag != ezJSON_ARRAY)
		return -1;

	int count = 0;
	JsonNode *tmp;
	json_foreach(tmp, json)
		++count;;
	return count;
}

/**
 * @brief js_member_cnt return json array/object member number
 *
 * traverse Json Node with array/object tag, return the number of members
 *
 * @param *json node to check
 * @return -1 if fail, others for the total count
 */
int js_member_cnt(JsonNode *json)
{
	if (json == NULL)
		return -1;
	if (json->tag != ezJSON_ARRAY && json->tag != ezJSON_OBJECT)
		return -1;

	int count = 0;
	JsonNode *tmp;
	json_foreach(tmp, json)
		++count;;
	return count;
}
	
/**
 * @brief js_parse_str convert string to JsonNode format
 *
 * convert string to JsonNode
 *  
 * @param *s text to convert
 * @return result JsonNode, need to be freed.
 */
JsonNode *js_parse_str(char *s)
{
	if (s==NULL) 
		return NULL;
	const char *s1 = chomp(s);
	return json_decode(s1);
}

/**
 * @brief js_parse_file convert content in file to JsonNode format
 *
 * convert content in file to JsonNode
 *  
 * @param *filename filename of the file need to be converted
 * @return result JsonNode, need to be freed.
 */
#define MAX_BUF_SIZE 40960
JsonNode *js_parse_file(char *filename)
{
	int fd, len;
	char *p = NULL, *p2;
	JsonNode *j;
	if (filename == NULL) 
		return NULL;
	if ((fd = open(filename, O_RDONLY)) == -1)
		return NULL;

	len = lseek(fd, 0, SEEK_END);
	if (len == 0){
		close(fd);
		return NULL;
	}
	p = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
	if ((p2 = calloc(1, len+1)) == NULL){
		munmap(0, len);
		close(fd);
		return NULL;
	}

	memcpy(p2, p, len);
	j = js_parse_str(p2);
	free(p2);
	munmap(0, len);
	close(fd);
	return j;
}

/**
 * @brief js_to_fp_hr convert JsonNode format, saved to file 
 *
 * Convert JsonNode to plain text(human readable) and save to file
 *  
 * @param *json JsonNode to be converted
 * @param *fp 
 */
void js_to_fp_hr(JsonNode *json, FILE* fp)
{
	if (json == NULL || fp == NULL) 
		return;
	char *tmp = json_stringify(json, "\t");
	fprintf(fp, "%s\n", tmp);
	free(tmp);
}

/**
 * @brief js_print_hr print JsonNode to stdout
 *
 * Convert JsonNode to plain text(human readable) and put in stdout
 *  
 * @param *json JsonNode to be printed
 *
 */
void js_print_hr(JsonNode *json)
{
	if (json == NULL) 
		return;
	js_to_fp_hr(json, stdout);
}

/**
 * @brief js_to_fp print JsonNode to file point
 *
 * Convert JsonNode to plain text and print to file point
 *  
 * @param *json JsonNode to be printed
 * @param *fp file point for output
 *
 */
void js_to_fp(JsonNode *json, FILE *fp)
{
	if (json == NULL || fp == NULL) 
		return;

	char *tmp = json_encode(json);
	fprintf(fp, "%s\n", tmp);
	free(tmp);
}

/**
 * @brief js_to_file print JsonNode to file
 *
 * Convert JsonNode to plain text and print to file
 *  
 * @param *json JsonNode to be printed
 * @param *fname file as output
 *
 */
void js_to_file(JsonNode *json, char *fname)
{
	if (json == NULL || fname == NULL) 
		return;

    FILE *fp = fopen(fname, "w");
    if (fp == NULL)
        return;
    js_to_fp(json, fp);
    fclose(fp);
}

/**
 * @brief js_to_file_hr print JsonNode to file
 *
 * Convert JsonNode to plain text(human readable) and print to file
 *
 * @param *json JsonNode to be printed
 * @param *fname file as output
 *
 */
void js_to_file_hr(JsonNode *json, char *fname)
{
	if (json == NULL || fname == NULL)
		return;

    FILE *fp = fopen(fname, "w");
    if (fp == NULL)
        return;
    js_to_fp_hr(json, fp);
    fclose(fp);
}

/**
 * @brief js_to_str convert JsonNode to string
 *
 * Convert JsonNode to plain text and print to file
 *  
 * @param *json JsonNode to be printed
 * @return output string
 *
 */
char *js_to_str(JsonNode *json)
{
	if (json == NULL)
		return NULL;
	return json_encode(json);
}

char *js_to_str_hr(JsonNode *json)
{
	if (json == NULL)
		return NULL;
	return json_stringify(json, "\t");
}

/**
 * @brief js_print print JsonNode to stdout
 *
 * Convert JsonNode to plain text and put in stdout
 *  
 * @param *json JsonNode to be printed
 *
 */
void js_print(JsonNode *json)
{
	if (json == NULL) 
		return;
	js_to_fp(json, stdout);
}

/**
 * @brief js_free free memory of JsonNode 
 *
 * free a JsonNode
 *  
 * @param *json JsonNode to be freeed
 */
void js_free(JsonNode *json)
{
	if (json == NULL) 
		return;
	json_delete(json);
}

/**
 * @brief js_get_path get member node from JsonNode with path
 *
 * this is a search function, root is json, path example:
 * Data                : json's child Data
 * Data/Req[key=1]     : return n'th Req which member has key=1(integer), Req is array
 * Data/Req[Key="wep"] : return n'th Req which member has key=wep(string), Req is array
 * Data/Req[5] : return 5'th Req, Req is array
 *
 * @param *json root json node
 * @param *path path start from json
 * @return JsonNode we need.
 *
 */
JsonNode *js_get_path(JsonNode *json, char *path)
{
	if (json == NULL || path == NULL)
		return NULL;

	char buffer[128];
	strcpy(buffer, path);

	char *key, *subpath = NULL, *start, *end;
	key = buffer;
	end = strchr(buffer, '/');
	if (end != NULL){
		*end = '\0';
		subpath = end + 1;
	}
    int vtype = ezJSON_NUMBER; // 0: int, 1: str
	char *s, *e, *idx = NULL;
	char *s2 = NULL, *k = NULL, *v = NULL, *vs = NULL, *ve=NULL;
	if ( (s = strchr(key, '[')) && (e = strchr(key,']'))){ // array
		*s = '\0';
		if ((s2 = strchr(s+1, '='))){ // key value member 
			*s2 = '\0';
			k = s+1;
			v = s2+1;
            if (v && (vs = strchr(v, '\"')) && (ve = strrchr(v, '\"'))){ // value is str
                vtype = ezJSON_STRING;
                v = vs + 1;
                *ve = '\0';
            }
			*e = '\0';
		}else{ // index
			idx = s+1;
			*e = '\0';
		}
	}
//	printf("%s(%d) json->tag:[%s] key:%s k:%s v:%s vtype:%d i:%s subpath:%s\n", __func__, __LINE__, jsontag_string(json->tag),key, k, v, vtype, idx, subpath);
    //js_print(json);
	JsonNode *subnode = NULL;
	int idx_num;
	JsonNode *j;
	json_foreach(j, json)
		if (!strcmp(j->key, key)){
			if (idx != NULL){ // has index
				if (!strcmp(idx,""))
					subnode = NULL;
				else
					subnode = json_find_element(j, atoi(idx));
			}else if (k!=NULL && v!=NULL) { // has sub-element key-value
				// check what the index is in sub elements, k(ey), v(alue)
				JsonNode *j2, *j3;
				json_foreach(j2, j) // array
					json_foreach(j3, j2) // object
                        if (vtype == ezJSON_NUMBER){
    						if (!strcmp(j3->key, k) && 
                                j3->tag == ezJSON_NUMBER && 
                                j3->number_ == atol(v)){
                                subnode = j2;
                                break;
                            }
                        }
                        else{ // vtype = ezJSON_STRING
    						if (!strcmp(j3->key, k) && !js_value_cmp(j3,v)){
	    						subnode = j2;
		    					break;
			    			}
                        }
			}
			else{ // no index
				subnode = j;
				break;
			}
		}
    
	if (subnode == NULL)
		return NULL; // not found
	if (subpath != NULL)
		return js_get_path(subnode, subpath);
	else 
		return subnode;
}

/**
 * @brief js_get_path_str get value of JsonNode object, who's tag is string
 *
 * check js_get_path function, get the end node, return its value in string type
 *
 * @param *json root json node
 * @param *path path start from json
 * @return string value of request JsonNode, NULL if not found or type mismatch
 *
 */
char *js_get_path_str(JsonNode *json, char *path)
{
	JsonNode *j = js_get_path(json, path);
	if (j == NULL)
		return NULL;
	if (j->tag != ezJSON_STRING){
		//printf("%s(%d) type mismatch, request:string, yours:[%s]\n", __func__, __LINE__, jsontag_string(j->tag));
		return NULL;
	}
	return j->string_;	
}

/**
 * @brief js_get_path_strz get value of JsonNode object, who's tag is string
 *
 * check js_get_path function, get the end node, return its value in string type
 *
 * @param *json root json node
 * @param *path path start from json
 * @return string value of request JsonNode, "" if not found or type mismatch
 *
 */
char *js_get_path_strz(JsonNode *json, char *path){

	char *ret = js_get_path_str(json, path);
	return (ret==NULL)?"":ret;
}

/**
 * @brief js_get_path_int get value of JsonNode object, who's tag is number (force 
 * to integer)
 *
 * check js_get_path function, get the end node, return its value in integer type
 *
 * @param *json root json node
 * @param *path path start from json
 * @return integer value of request JsonNode, JS_FAIL if not found or type mismatch
 *
 */
int js_get_path_int(JsonNode *json, char *path)
{
	JsonNode *j = js_get_path(json, path);
	if (j == NULL)
		return JS_FAIL;
	if (j->tag != ezJSON_NUMBER){
		//printf("%s(%d) type mismatch, request:string, yours:[%s]\n", __func__, __LINE__, jsontag_string(j->tag));
		return JS_FAIL;
	}
	return (int)j->number_;
}

/**
 * @brief js_get_path_double get value of JsonNode object, who's tag is number (double)
 *
 * check js_get_path function, get the end node, return its value in double type
 *
 * @param *json root json node
 * @param *path path start from json
 * @return integer value of request JsonNode, JS_FAIL if not found or type mismatch
 *
 */
double js_get_path_double(JsonNode *json, char *path)
{
	JsonNode *j = js_get_path(json, path);
	if (j == NULL)
		return JS_FAIL;
	if (j->tag != ezJSON_NUMBER){
		//printf("%s(%d) type mismatch, request:string, yours:[%s]\n", __func__, __LINE__, jsontag_string(j->tag));
		return JS_FAIL;
	}
	return j->number_;
}

/**
 * @brief js_get_path_bool get value of JsonNode object, who's tag is bool
 *
 * check js_get_path function, get the end node, return its value in bool type
 *
 * @param *json root json node
 * @param *path path start from json
 * @return bool value of request JsonNode, 0(false) if not found or type mismatch
 *
 */
bool js_get_path_bool(JsonNode *json, char *path)
{
	JsonNode *j = js_get_path(json, path);
	if (j == NULL)
		return 0; // fail case, what to return?
	if (j->tag != ezJSON_BOOL){
		//printf("%s(%d) type mismatch, request:string, yours:[%s]\n", __func__, __LINE__, jsontag_string(j->tag));
        return 0; // fail case, what to return?
	}
    return j->bool_;
}

/**
 * @brief js_free_path get JsonNode by path, free the Node
 *
 * locate the member Node start from json by path, free the Node
 *
 * @param *json root json node
 * @param *path path start from json
 *
 */
void js_free_path(JsonNode *json, char *path)
{
    JsonNode *j;
    j = js_get_path(json, path);

    if (j)
        js_free(j);

    return;
}

/**
 * @brief js_get_js use js to get json values
 *
 * Use jkey as reference, check dst's value, fill the value to jkey
 *
 * @param *dst json storage for fetching values
 * @param *jkey json as reference, check which values to get
 * @return result json, jkey structure with dst's value
 *
 */
JsonNode *js_get_js(JsonNode *dst, JsonNode *jkey)
{
    if (!jkey || !dst)
        return NULL;

    JsonNode *d, *j, *m=NULL, *t = NULL;
    int found;
	switch(jkey->tag)
	{
		case ezJSON_BOOL:
		case ezJSON_STRING:
		case ezJSON_NUMBER:
            return jkey;
		case ezJSON_OBJECT:
            json_foreach(j, jkey){
                found = 0;
                json_foreach(d, dst){
                    if ((d->key == NULL || j->key == NULL) ||
                        (strcmp(d->key, j->key)))
                        continue;
//                    printf("[object]check %s\n", d->key);
                    if (d->tag == j->tag){
                        if (d->tag != ezJSON_OBJECT && d->tag != ezJSON_ARRAY &&
                            !js_node_equal(d,j)){
                            found = 1;
                            m = dst;
                            break;
                        }
                        if ((d->tag == ezJSON_OBJECT || d->tag == ezJSON_ARRAY) &&
                            (json_first_child(j) == NULL)){
                            return d;
                        }
                        if ((d->tag == ezJSON_OBJECT || d->tag == ezJSON_ARRAY) &&
                            (t = js_get_js(d,j)) != NULL){
                            return t;
                        }
                    }
                    if (j->tag == ezJSON_OBJECT && json_first_child(j) == NULL){
                        return d;
                    }
                }
                if (found == 0) return NULL;
            }
            return (t==NULL)?m:t;
        case ezJSON_ARRAY:  // array and object
            json_foreach(j, jkey){
                found = 0;
                json_foreach(d, dst){
                    if (d->tag == j->tag){
//                        printf("[array]check %s\n", d->key);
                        if (d->tag != ezJSON_OBJECT && d->tag != ezJSON_ARRAY &&
                                !js_equal(d,j)){
                            found = 1;
                            m = dst;
                            break;
                        }
                        if ((d->tag == ezJSON_OBJECT || d->tag == ezJSON_ARRAY) &&
                            (t = js_get_js(d, j)) != NULL){
                            return t;
                        }
                    }
                }
                if (found == 0) return NULL;
            }
//            return (found == 1)?m:NULL;
            return (t == NULL)?m:t;
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            return NULL;
	}
}

/**
 * @brief js_set_path base on root JsonNode, recursively set the child element
 *
 * Base on json, check path, add the element recursively, path example:
 * Data                  : add child Data(if not exist) in json 
 * Data/Req[key=1]       : add child Data(if not exist), add Req array(if not exist) 
 *                         in Data, Add one array element(object) and key=1(integer) 
 *                         in this array element if not exist
 * Data/Req[key="wep"]   : add child Data(if not exist), add Req array(if not exist) 
 *                         in Data, Add one array element(object) and key=wep(string) 
 *                         in this array element if not exist
 * Data/Req[]            : add child Data(if not exist), add Req array(if not exist) 
 *                         in Data, Add new array element(object) in this array element
 * Data/Req[key=1]/info : add child Data(if not exist), add Req array(if not exist) in 
 *                        Data, Add one array element(object), add a key = 1(integer) 
 *                        in this array element(if no element with key=1 child), add 
 *                        info in this array element
 * @param *json root json node
 * @param *path path start from json
 * @return JsonNode of the last added Node
 *
 */
JsonNode *js_set_path(JsonNode *json, char *path)
{
	JsonNode *json_child;
	if (json == NULL)
		return NULL;
	if (path == NULL)
		return json;

	char buffer[128];
	strcpy(buffer, path);

	char *key, *subpath = NULL, *start, *end;
	key = buffer;
	end = strchr(buffer, '/');
	if (end != NULL){
		*end = '\0';
		subpath = end + 1;
	}
	if ((json_child = js_get_path(json, key)) != NULL){
		return js_set_path(json_child, subpath);	
	}
	// json_child is null
    int vtype = ezJSON_NUMBER; // 0: int, 1: str
	char *s, *e, *idx = NULL;
	char *s2 = NULL, *k = NULL, *v = NULL, *vs = NULL, *ve=NULL;
	if ( (s = strchr(key, '[')) && (e = strchr(key,']'))){ // array
		*s = '\0';
		if ((s2 = strchr(s+1, '='))){ // key value member 
			*s2 = '\0';
			k = s+1;
			v = s2+1;
            if (v && (vs = strchr(v, '\"')) && (ve = strrchr(v, '\"'))){ // value is str
                vtype = ezJSON_STRING;
                v = vs + 1;
                *ve = '\0';
            }
		}else{ // index
			idx = s+1;
		}
		*e = '\0';

		if ((json_child = js_get_path(json, key)) == NULL){ // member not exist, add it
			json_child = json_mkarray();
			json_append_member(json, key, json_child);
		}

		if (json_child->tag != ezJSON_ARRAY)
			return NULL;

		if (k != NULL && v != NULL){
//	printf("%s(%d) key:%s k:%s v:%s vtype:%d i:%s subpath:%s\n", __func__, __LINE__, key, k, v, vtype, idx, subpath);
			JsonNode *child_son = json_mkobject();

            if (vtype == ezJSON_NUMBER)
    			json_append_member(child_son, k, json_mknumber(atof(v)));
            else // vtype = ezJSON_STRING
    			json_append_member(child_son, k, json_mkstring(v));
			json_append_element(json_child, child_son);
			//js_print(json_child);
			return js_set_path(child_son, subpath);
		}else if (idx!=NULL){ // 2. idx exist
			if (!strcmp(idx,"")){ // idx empty, append to tail
                if (end != NULL){ // subpath exist
				    JsonNode *child_son = json_mkobject();
				    json_append_element(json_child, child_son);
				    return js_set_path(child_son, subpath);
                }
                else // path[] without subpath, return path node
                    return json_child;
			}else{
				//printf("idx:[%s]\n", idx);
				JsonNode *child_son = json_find_element(json_child, atoi(idx));
				return js_set_path(child_son, subpath);
			}
		}
	}else{ // object
		//printf("%s(%d) key:%s k:%s v:%s i:%s subpath:%s\n", __func__, __LINE__, key, k, v, idx, subpath);
		JsonNode *json_child = json_mkobject();
		json_append_member(json, key, json_child);
		return js_set_path(json_child, subpath);
	}
}

/**
 * @brief js_set_path_value set value in a Node located by json + path, Node type
 * is decided by t
 *
 * check js_set_path function, set the last node, with value, value type is set by t
 *
 * @param *json root json node
 * @param *path path start from json
 * @param *val value to set
 * @param t type (ezJSON_STRING/ezJSON_NUMBER/ezJSON_BOOL)
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_set_path_value(JsonNode *json, char *path, void *val, JsonTag t)
{
    JsonNode *j, *o;
	if (json == NULL || path == NULL)
		return JS_FAIL;
	
	char *base_path, *nodename, *start, *end;
	char *s, *e, *idx = NULL;
	char buffer[128];

	strcpy(buffer, path);
	base_path = buffer;
	end = strrchr(buffer, '/');

	if (end != NULL){
		*end = '\0';
		nodename = end + 1;
        
    	j = js_set_path(json, base_path);
        if (j == NULL)
            return JS_FAIL;
        if (j->tag == ezJSON_ARRAY){ // base path is empty array
            // TODO: array do something
            o = json_mkobject();
			json_append_element(j, o);
            j = o;
        }
	}
    else { // no base_path, just nodename
        base_path = NULL;
        nodename = buffer;
        j = json;        
    }
//    printf("base_path:%s nodename:[%s]\n", base_path, nodename);	
	if ( (s = strchr(nodename, '[')) && (e = strchr(nodename,']'))){ // array
		*s = '\0';
		idx = s+1;
		*e = '\0';
    }

	// nodename exist?
    JsonNode *k = js_get_path(j, nodename);
    JsonNode *v;
    if (k == NULL ||  // node not exist
            (k != NULL && k->tag == ezJSON_ARRAY)){ // node is array
        switch(t){
            case ezJSON_BOOL:
                v = json_mkbool(*((bool *)val));
                break;
            case ezJSON_STRING:
                v = json_mkstring((char *)val);
                break;
            case ezJSON_NUMBER:
                v = json_mknumber(*((double *)val));
                break;
            case ezJSON_ARRAY:
            case ezJSON_OBJECT:
            default:
                return JS_FAIL;
        }
    }

    if (k == NULL){
        if (idx!=NULL){ // k is array
            JsonNode *tmp = json_mkarray();
		    json_append_element(tmp, v);
    		json_append_member(j, nodename, tmp);
        }
        else
    		json_append_member(j, nodename, v);
        return 1;
    }
    if (k->tag == ezJSON_ARRAY){
		json_append_element(k, v);
    }

    // nodename exist, not array
    if (k->tag != t){
//        printf("%s(%d): type mismatch\n", __func__,__LINE__);
        return JS_FAIL;
    }
    switch(t){
        case ezJSON_BOOL:
            k->bool_ = *((bool *)val);
            break;
        case ezJSON_STRING:
            if (k->string_ != NULL)
                free(k->string_);
            k->string_ = strdup((char *)val);
            break;
        case ezJSON_NUMBER:
            k->number_ = *((double *)val);
            break;
        case ezJSON_ARRAY:
        case ezJSON_OBJECT:
            printf("fail\n");
        default:
            return JS_FAIL;
    }
    return 1; 
}

/**
 * @brief js_set_path_int set integer in a Node located by json + path
 *
 * check js_set_path_value function, node type is number (put int in double type)
 *
 * @param *json root json node
 * @param *path path start from json
 * @param val value to set, integer type
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_set_path_int(JsonNode *json, char *path, int val)
{
    double v = (double)val;
    return js_set_path_value(json, path, (void *)&v, ezJSON_NUMBER);
}

/**
 * @brief js_set_path_double set double in a Node located by json + path
 *
 * check js_set_path_value function, node type is number (double)
 *
 * @param *json root json node
 * @param *path path start from json
 * @param val value to set, double type
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_set_path_double(JsonNode *json, char *path, double val)
{
    return js_set_path_value(json, path, (void *)&val, ezJSON_NUMBER);
}

/**
 * @brief js_set_path_str set string in a Node located by json + path
 *
 * check js_set_path_value function, node type is string
 *
 * @param *json root json node
 * @param *path path start from json
 * @param *val value to set, string type
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_set_path_str(JsonNode *json, char *path, char *val)
{
    return js_set_path_value(json, path, (void *)val, ezJSON_STRING);
}

/**
 * @brief js_set_path_bool set bool in a Node located by json + path
 *
 * check js_set_path_value function, node type is bool
 *
 * @param *json root json node
 * @param *path path start from json
 * @param val value to set, bool type
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_set_path_bool(JsonNode *json, char *path, bool val)
{
    bool v = (bool)val;
    return js_set_path_value(json, path, (void *)&v, ezJSON_BOOL);
}

/**
 * @brief js_dup duplicate a json tree from one to another
 *
 * duplicate a json 's tree (include memory allocation)
 *
 * @param *json json node to copy
 * @return Json node duplicated, need to be freed
 *
 */
JsonNode *js_dup(JsonNode *json)
{
    if (json == NULL) 
        return NULL;
    JsonNode *t, *j, *t2;
	switch(json->tag)
	{
		case ezJSON_BOOL:
            t = json_mkbool(json->bool_);
            break;
		case ezJSON_STRING:
            t = json_mkstring(json->string_);
            break;
		case ezJSON_NUMBER:
            t = json_mknumber(json->number_);
            break;
		case ezJSON_OBJECT:
            t = json_mkobject();
            json_foreach(j, json){
                t2 = js_dup(j);
                json_append_member(t, j->key, t2);
            }
            break;
		case ezJSON_ARRAY:  // array and object
            t = json_mkarray();
            json_foreach(j, json){
                t2 = js_dup(j);
                json_append_element(t, t2);
            }
            break;
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            return NULL;
	}
    return t;
}
/**
 * @brief js_node_dup duplicate a json structure from one to another
 *
 * duplicate a json's structure (include memory allocation)
 *
 * @param *json json node to copy
 * @return Json node duplicated, need to be freed
 *
 */
JsonNode *js_node_dup(JsonNode *json)
{
    if (json == NULL)
        return NULL;
    JsonNode *t, *j, *t2;
	switch(json->tag)
	{
		case ezJSON_BOOL:
            t = json_mkbool(json->bool_);
            break;
		case ezJSON_STRING:
            t = json_mkstring(json->string_);
            break;
		case ezJSON_NUMBER:
            t = json_mknumber(json->number_);
            break;
		case ezJSON_OBJECT:
            t = json_mkobject();
            break;
		case ezJSON_ARRAY:  // array and object
            t = json_mkarray();
            break;
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            return NULL;
	}
    return t;
}

/**
 * @brief js_idx_union copy JsonNode from src to dst, fill the lack of src, filter js_idx_ info
 *
 * copy src to dst, add node exist in src but not in dst, overwrite the same tag with 
 * different value, match element with js_idx_ infomation, remove the tag after locate element
 *
 * @param *dst json node copy to 
 * @param *src json node copy from
 *
 */
void js_idx_union(JsonNode *dst, JsonNode *src)
{
    if (!src || !dst)
        return;

//    printf("src->key:[%s] dst->key[%s] src->tag:[%s] dst->tag[%s]\n", src->key, dst->key, jsontag_string(src->tag), jsontag_string(dst->tag));
    JsonNode *d, *s, *c, *jkey;
    char *idx = NULL, *key = NULL;
    int found, key_dup;
	switch(src->tag)
	{
		case ezJSON_BOOL:
            if (src->bool_ != dst->bool_)
                dst->bool_ = src->bool_;
            break;
		case ezJSON_STRING:
            if (src->string_ != NULL && dst->string_ != NULL && strcmp(src->string_, dst->string_)){
                free(dst->string_);
                dst->string_ = strdup(src->string_);
            }
            break;
		case ezJSON_NUMBER:
            if (src->number_ != dst->number_)
                dst->number_ = src->number_;
            break;
		case ezJSON_OBJECT:
            json_foreach(s, src){
                found = 0;
                json_foreach(d, dst){
                    if ((s->key == NULL || d->key == NULL) ||
                        (strcmp(d->key, s->key)))
                        continue;
                    found = 1;
                    if (d->tag == s->tag){
                        if (s->tag != ezJSON_OBJECT && s->tag != ezJSON_ARRAY){
                            js_idx_union(d,s);
                            break;
                        }
                        // ezJSON_OBJECT or ezJSON_ARRAY
                        js_idx_union(d, s);
                        break;
                    }
                    // tag not equal
                }
                if (found == 0){
                    c = js_node_dup(s);
                    json_append_member(dst, s->key, c);
                    js_idx_union(c, s);
                }
            }
            break;
        case ezJSON_ARRAY:  // array and object
            json_foreach(s, src){
                if (s->tag == ezJSON_OBJECT){
                    // find index: idx and jkey
                    idx = NULL;
                    jkey= NULL;
                    json_foreach(c, s){
                        if ((idx = strstr(c->key,"js_idx_")) != NULL){
                            idx = idx + strlen("js_idx_");
                            jkey = c;
                            break;
                        }
                    }
                    // check duplicated key with idx
                    key_dup = 0;
                    if (idx != NULL && jkey != NULL){
                        json_foreach(c, s){
                            if (!strcmp(c->key, idx)){
                                key_dup = 1;
                                break;
                            }
                        }
                    }
                    // locate elements in dst
                    found = 0;
                    if (idx != NULL && jkey != NULL){
                        json_foreach(d, dst){
                            if (d->tag == ezJSON_OBJECT){
                                if (!js_idx_match(d, idx, jkey)){
                                    js_free(jkey); // located, remove idx in s after found
                                    js_idx_union(d,s);
                                    found = 1;
                                    break;
                                }
                            }
                        }
                    }
                    if (found == 0){ // create new
                        if(key_dup == 1){
                            js_free(jkey);
                        }else if (jkey!=NULL && idx!=NULL){
                            key = jkey->key;
                            jkey->key = strdup(idx);
                            free(key);
                        }
                        c = js_node_dup(s);
                        json_append_element(dst, c);
                        js_idx_union(c, s);
                    }
                }
                if (s->tag == ezJSON_ARRAY){
                    printf("dont put array in array, not support\n");
                    break;
                }
                if (s->tag != ezJSON_ARRAY && s->tag != ezJSON_OBJECT){ // int or string array
                    found = 0;
                    json_foreach(d, dst){
                        if(!js_node_equal(s,d))
                            found = 1;
                    }
                    if (found == 0)
                        json_append_element(dst, js_dup(s));
                }
            }
            break;
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            printf("no such tag\n");
            break;
	}
    return;
}

/**
 * @brief js_union copy JsonNode from src to dst, fill the lack of src
 *
 * copy src to dst, add node exist in src but not in dst, overwrite the same tag with
 * different value
 *
 * @param *dst json node copy to
 * @param *src json node copy from
 *
 */
void js_union(JsonNode *dst, JsonNode *src)
{
    if (!src || !dst)
        return;

//    printf("src->key:[%s] dst->key[%s] src->tag:[%s] dst->tag[%s]\n", src->key, dst->key, jsontag_string(src->tag), jsontag_string(dst->tag));
    JsonNode *d, *s, *c, *jkey;
    char *idx = NULL, *key = NULL;
    int found, key_dup;
	switch(src->tag)
	{
		case ezJSON_BOOL:
            if (src->bool_ != dst->bool_)
                dst->bool_ = src->bool_;
            break;
		case ezJSON_STRING:
            if (src->string_ != NULL && dst->string_ != NULL && strcmp(src->string_, dst->string_)){
                free(dst->string_);
                dst->string_ = strdup(src->string_);
            }
            break;
		case ezJSON_NUMBER:
            if (src->number_ != dst->number_)
                dst->number_ = src->number_;
            break;
		case ezJSON_OBJECT:
            json_foreach(s, src){
                found = 0;
                json_foreach(d, dst){
                    if ((s->key == NULL || d->key == NULL) ||
                        (strcmp(d->key, s->key)))
                        continue;
                    found = 1;
                    if (d->tag == s->tag){
                        if (s->tag != ezJSON_OBJECT && s->tag != ezJSON_ARRAY){
                            js_union(d,s);
                            break;
                        }
                        // ezJSON_OBJECT or ezJSON_ARRAY
                        js_union(d, s);
                        break;
                    }
                    // tag not equal
                }
                if (found == 0){
                    c = js_node_dup(s);
                    json_append_member(dst, s->key, c);
                    js_union(c, s);
                }
            }
            break;
        case ezJSON_ARRAY:  // array and object
            json_foreach(s, src){
                if (s->tag == ezJSON_OBJECT){
                    // locate elements in dst
                    found = 0;
                    json_foreach(d, dst){
                        if (d->tag == ezJSON_OBJECT){
                            if (!js_equal(s, d)){
                                js_union(d,s);
                                found = 1;
                                break;
                            }
                        }
                    }
                    if (found == 0){ // create new
                        c = js_node_dup(s);
                        json_append_element(dst, c);
                        js_union(c, s);
                    }
                }                    
                if (s->tag == ezJSON_ARRAY){
                    printf("dont put array in array, not support\n");
                    break;
                }
                if (s->tag != ezJSON_ARRAY && s->tag != ezJSON_OBJECT){ // int or string array
                    found = 0;
                    json_foreach(d, dst){
                        if(!js_node_equal(s,d))
                            found = 1;
                    }
                    if (found == 0)
                        json_append_element(dst, js_dup(s));
                }
            }
            break;
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            printf("no such tag\n");
            break;
	}
    return;
}

/**
 * @brief js_join Copy leave node from one Json to another

 * base on dst, copy the leave Node located in same path of src
 *
 * @param *dst json node copy to 
 * @param *src json node copy from
 *
 */
void js_join(JsonNode *dst, JsonNode *src)
{
    if (!src || !dst)
        return;

//    printf("src->key:[%s] dst->key[%s] src->tag:[%s] dst->tag[%s]\n", src->key, dst->key, jsontag_string(src->tag), jsontag_string(dst->tag));

    JsonNode *d, *s, *c;
    char *idx = NULL;
    int found;
	switch(dst->tag)
	{
		case ezJSON_BOOL:
            if (src->bool_ != dst->bool_)
                dst->bool_ = src->bool_;
            break;
		case ezJSON_STRING:
            if (src->string_ != NULL && dst->string_ != NULL && strcmp(src->string_, dst->string_)){
                free(dst->string_);
                dst->string_ = strdup(src->string_);
            }
            break;
		case ezJSON_NUMBER:
            if (src->number_ != dst->number_)
                dst->number_ = src->number_;
            break;
		case ezJSON_OBJECT:
            if (json_first_child(dst) == NULL){
                if (json_first_child(src) != NULL){
                    js_union(dst, src);
                }
                return;
            }
            json_foreach(d, dst){
                json_foreach(s, src){
                    if ((d->key == NULL || s->key == NULL) ||
                        (strcmp(d->key, s->key)))
                        continue;
                    if (d->tag == s->tag){
                        if (s->tag != ezJSON_OBJECT && s->tag != ezJSON_ARRAY){
                            js_join(d,s);
                            break;
                        }
                        // ezJSON_OBJECT or ezJSON_ARRAY
                        if (json_first_child(d) == NULL)
                            js_union(d, s);
                        else
                            js_join(d, s);
                        break;
                    }
                    // tag not equal
                    if (d->tag == ezJSON_OBJECT && json_first_child(d) == NULL){
                        c = js_node_dup(s);
                        json_append_member(dst, s->key, c);
                        js_union(c, s);
                        json_delete(d);
                    }
                }
            }
            break;
        case ezJSON_ARRAY:  // array and object
            if ((c = json_first_child(dst))==NULL){
                if (json_first_child(src) != NULL){
                    js_union(dst, src);
                }
                return;
            }
            if (c->tag != ezJSON_ARRAY && c->tag != ezJSON_OBJECT){
                js_union(dst, src);
                return;
            }
            // OBJECT and ARRAY
            json_foreach(d, dst){
                if (d->tag == ezJSON_OBJECT){
                    idx = NULL;
                    // find index
                    json_foreach(c, d){
                        if ((idx = strstr(c->key,"js_idx_")) != NULL){
                            idx = idx + strlen("js_idx_");
                            //printf("idx:%s\n", idx);
                            // locate elements
                            json_foreach(s, src){
                                if (s->tag == ezJSON_OBJECT){
                                    if (!js_idx_match(s, idx, c)){
                                        // keep the key for idx_function to search
                                        //js_free(c);
                                        if (js_member_cnt(d) == 1){ // only idx in member, no child, duplicate src to dst
                                            js_union(d,s);
                                        }
                                        else // keep join children
                                            js_join(d,s);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }                    
                if (d->tag == ezJSON_ARRAY){
                    printf("dont put array in array, not support\n");
                }
            }
            break;
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            return;
	}
}

/**
 * @brief js_free_js free JsonNode base on another json node

 * get js_node by js_get_js(by JsonNode jref, search dst), free this node in dst
 * 
 * @param *dst json node copy to 
 * @param *jref json node show the node to free in dst
 *
 */
void js_free_js(JsonNode *dst, JsonNode *jref)
{
    JsonNode *j;
    j = js_get_js(dst, jref);
    if (j)
        js_free(j);
}

/*
    Index function
    Since Json Array is no index concept, I use js_idx_ as prefix to overcome
    this shortage.
*/

/**
 * @brief js_get_idx_js use js to get json values
 *
 * Use jkey as reference, check dst's value, fill the value to jkey
 * if dst contains array, jkey can put js_idx_[key] in the specific member
 * this function will depend on the [key] to locate this array member
 *
 * @param *dst json storage for fetching values
 * @param *jkey json as reference, check which values to get
 * @return result json, jkey structure with dst's value
 *
 */
JsonNode *js_get_idx_js(JsonNode *dst, JsonNode *jkey)
{
    if (!jkey || !dst)
        return NULL;

    JsonNode *d, *j, *m=NULL, *t = NULL;
    char *idx = NULL;
    int found, match;
	switch(jkey->tag)
	{
		case ezJSON_BOOL:
		case ezJSON_STRING:
		case ezJSON_NUMBER:
            return jkey;
		case ezJSON_OBJECT:
            match = 0;
            json_foreach(j, jkey){
                if (j->tag != ezJSON_OBJECT && j->tag != ezJSON_ARRAY &&
                    (idx = strstr(j->key,"js_idx_")) != NULL){
                    idx = idx + strlen("js_idx_");
                    //printf("idx:%s\n", idx);
                }
                found = 0;
                json_foreach(d, dst){
                    //printf("[object]check %s\n", d->key);
                    if (idx != NULL && 
                        !strcmp(d->key, idx) &&
                        !js_value_equal(d, j)){
                            match = 1;
                            m = dst;
                            continue;
                    }
                    if ((d->key == NULL || j->key == NULL) ||
                        (strcmp(d->key, j->key)))
                        continue;
                    if (d->tag == j->tag){
                        if (d->tag != ezJSON_OBJECT && d->tag != ezJSON_ARRAY &&
                            !js_node_equal(d,j)){
                            found = 1;
                            m = dst;
                            break;
                        }
                        if ((d->tag == ezJSON_OBJECT || d->tag == ezJSON_ARRAY) &&
                            (json_first_child(j) == NULL)){
                            return d;
                        }
                        if (d->tag == ezJSON_OBJECT || d->tag == ezJSON_ARRAY){
                            if ((t = js_get_idx_js(d,j)) != NULL){
                                return t;
                            }
                        }
                    }
                    if (j->tag == ezJSON_OBJECT && json_first_child(j) == NULL){
                        return d;
                    }
                }
                if (found == 0 && match != 1) return NULL;
            }
            return (t==NULL)?m:t;
        case ezJSON_ARRAY:  // array and object
            json_foreach(j, jkey){
                found = 0;
                json_foreach(d, dst){
                    if (d->tag == j->tag){
//                        printf("[array]check %s\n", d->key);
                        if (d->tag != ezJSON_OBJECT && d->tag != ezJSON_ARRAY &&
                                !js_equal(d,j)){
                            found = 1;
                            m = dst;
                            break;
                        }
                        if ((d->tag == ezJSON_OBJECT || d->tag == ezJSON_ARRAY)){
                            if ((t = js_get_idx_js(d, j)) != NULL){
                                return t;
                            }
                        }
                    }
                }
                if (found == 0) return NULL;
            }
//            return (found == 1)?m:NULL;
            return (t == NULL)?m:t;
		case ezJSON_NULL:
            printf("tag is ezJSON_NULL!?\n");
		default:
            return NULL;
	}
}

/**
 * @brief js_idx_get_path get member node from JsonNode with path
 *
 * same as js_get_path, but, the for the array key in path [key=1]
 * this function will search if array member's has js_idx_key as child
 * i.e., json contains js_idx_key already
 *
 * @param *json root json node
 * @param *path path start from json
 * @return JsonNode we need.
 *
 */
JsonNode *js_idx_get_path(JsonNode *json, char *path)
{
	if (json == NULL || path == NULL)
		return NULL;

	char buffer[128];
	strcpy(buffer, path);

	char *key, *subpath = NULL, *start, *end;
	key = buffer;
	end = strchr(buffer, '/');
	if (end != NULL){
		*end = '\0';
		subpath = end + 1;
	}
    int vtype = ezJSON_NUMBER; // 0: int, 1: str
	char *s, *e, *idx = NULL;
	char *s2 = NULL, *k = NULL, *v = NULL, *vs = NULL, *ve=NULL;
	if ( (s = strchr(key, '[')) && (e = strchr(key,']'))){ // array
		*s = '\0';
		if ((s2 = strchr(s+1, '='))){ // key value member 
			*s2 = '\0';
			k = s+1;
			v = s2+1;
            if (v && (vs = strchr(v, '\"')) && (ve = strrchr(v, '\"'))){ // value is str
                vtype = ezJSON_STRING;
                v = vs + 1;
                *ve = '\0';
            }
			*e = '\0';
		}else{ // index
			idx = s+1;
			*e = '\0';
		}
	}
//	printf("%s(%d) json->tag:[%s] key:%s k:%s v:%s vtype:%d i:%s subpath:%s\n", __func__, __LINE__, jsontag_string(json->tag),key, k, v, vtype, idx, subpath);
    //js_print(json);
	JsonNode *subnode = NULL;
	int idx_num;
	JsonNode *j;
	json_foreach(j, json)
		if (!strcmp(j->key, key)){
			if (idx != NULL){ // has index
				if (!strcmp(idx,""))
					subnode = NULL;
				else
					subnode = json_find_element(j, atoi(idx));
			}else if (k!=NULL && v!=NULL) { // has sub-element key-value
                char prefix_key[32];
                sprintf(prefix_key, "js_idx_%s", k);
				// check what the index is in sub elements, k(ey), v(alue)
				JsonNode *j2, *j3;
				json_foreach(j2, j) // array
					json_foreach(j3, j2) // object
                        if (vtype == ezJSON_NUMBER){
    						if (!strcmp(j3->key, prefix_key) && 
                                j3->tag == ezJSON_NUMBER && 
                                j3->number_ == atol(v)){
                                subnode = j2;
                                break;
                            }
                        }
                        else{ // vtype = ezJSON_STRING
    						if (!strcmp(j3->key, prefix_key) && !js_value_cmp(j3,v)){
	    						subnode = j2;
		    					break;
			    			}
                        }
			}
			else{ // no index
				subnode = j;
				break;
			}
		}
    
	if (subnode == NULL)
		return NULL; // not found
	if (subpath != NULL)
		return js_idx_get_path(subnode, subpath);
	else 
		return subnode;
}

/**
 * @brief js_idx_get_path_str get value of JsonNode object, who's tag is string
 *
 * js_get_path_str index version, use index machanism to locate the array
 *
 * @param *json root json node
 * @param *path path start from json
 * @return string value of request JsonNode, NULL if not found or type mismatch
 *
 */
char *js_idx_get_path_str(JsonNode *json, char *path)
{
	JsonNode *j = js_idx_get_path(json, path);
	if (j == NULL)
		return NULL;
	if (j->tag != ezJSON_STRING){
		//printf("%s(%d) type mismatch, request:string, yours:[%s]\n", __func__, __LINE__, jsontag_string(j->tag));
		return NULL;
	}
	return j->string_;
}

/**
 * @brief js_idx_get_path_strz get value of JsonNode object, who's tag is string
 *
 * js_get_path_strz index version, use index machanism to locate the array
 *
 * @param *json root json node
 * @param *path path start from json
 * @return string value of request JsonNode, "" if not found or type mismatch
 *
 */
char *js_idx_get_path_strz(JsonNode *json, char *path){

	char *ret = js_idx_get_path_str(json, path);
	return (ret==NULL)?"":ret;
}

/**
 * @brief js_idx_get_path_int get value of JsonNode object, who's tag is number (force 
 * to integer)
 *
 * js_get_path_int index version, use index machanism to locate the array
 *
 * @param *json root json node
 * @param *path path start from json
 * @return integer value of request JsonNode, JS_FAIL if not found or type mismatch
 *
 */
int js_idx_get_path_int(JsonNode *json, char *path)
{
	JsonNode *j = js_idx_get_path(json, path);
	if (j == NULL)
		return JS_FAIL;
	if (j->tag != ezJSON_NUMBER){
		//printf("%s(%d) type mismatch, request:string, yours:[%s]\n", __func__, __LINE__, jsontag_string(j->tag));
		return JS_FAIL;
	}
	return (int)j->number_;
}

/**
 * @brief js_idx_get_path_double get value of JsonNode object, who's tag is number (double)
 *
 * js_get_path_double index version, use index machanism to locate the array
 *
 * @param *json root json node
 * @param *path path start from json
 * @return integer value of request JsonNode, JS_FAIL if not found or type mismatch
 *
 */
double js_idx_get_path_double(JsonNode *json, char *path)
{
	JsonNode *j = js_idx_get_path(json, path);
	if (j == NULL)
		return JS_FAIL;
	if (j->tag != ezJSON_NUMBER){
		//printf("%s(%d) type mismatch, request:string, yours:[%s]\n", __func__, __LINE__, jsontag_string(j->tag));
		return JS_FAIL;
	}
	return j->number_;
}

/**
 * @brief js_idx_get_path_bool get value of JsonNode object, who's tag is bool
 *
 * js_get_path_bool index version, use index machanism to locate the array
 *
 * @param *json root json node
 * @param *path path start from json
 * @return bool value of request JsonNode, JS_FAIL if not found or type mismatch
 *
 */
bool js_idx_get_path_bool(JsonNode *json, char *path)
{
	JsonNode *j = js_idx_get_path(json, path);
	if (j == NULL)
		return JS_FAIL;
	if (j->tag != ezJSON_BOOL){
		//printf("%s(%d) type mismatch, request:string, yours:[%s]\n", __func__, __LINE__, jsontag_string(j->tag));
		return JS_FAIL;
	}
	return j->bool_;
}

/**
 * @brief js_idx_set_path get member node from JsonNode with path
 *
 * same as js_get_path, but, the for the array key in path [key=1]
 * this function will append prefix js_idx_ to it, the key will be set 
 * in the same path, but rename as js_idx_key
 *
 * @param *json root json node
 * @param *path path start from json
 * @return JsonNode of the last added Node
 *
 */
JsonNode *js_idx_set_path(JsonNode *json, char *path)
{
	JsonNode *json_child;
	if (json == NULL)
		return NULL;
	if (path == NULL)
		return json;

	char buffer[128];
	strcpy(buffer, path);

	char *key, *subpath = NULL, *start, *end;
	key = buffer;
	end = strchr(buffer, '/');
	if (end != NULL){
		*end = '\0';
		subpath = end + 1;
	}
	if ((json_child = js_get_path(json, key)) != NULL){
		return js_idx_set_path(json_child, subpath);	
	}
	// json_child is null
    int vtype = ezJSON_NUMBER; // 0: int, 1: str
	char *s, *e, *idx = NULL;
	char *s2 = NULL, *k = NULL, *v = NULL, *vs = NULL, *ve=NULL;
	if ( (s = strchr(key, '[')) && (e = strchr(key,']'))){ // array
		*s = '\0';
		if ((s2 = strchr(s+1, '='))){ // key value member 
			*s2 = '\0';
			k = s+1;
			v = s2+1;
            if (v && (vs = strchr(v, '\"')) && (ve = strrchr(v, '\"'))){ // value is str
                vtype = ezJSON_STRING;
                v = vs + 1;
                *ve = '\0';
            }
		}else{ // index
			idx = s+1;
		}
		*e = '\0';

		if ((json_child = js_get_path(json, key)) == NULL){ // member not exist, add it
			json_child = json_mkarray();
			json_append_member(json, key, json_child);
		}

		if (json_child->tag != ezJSON_ARRAY)
			return NULL;

		if (k != NULL && v != NULL){
            char prefix_key[32];
//	printf("%s(%d) key:%s k:%s v:%s vtype:%d i:%s subpath:%s\n", __func__, __LINE__, key, k, v, vtype, idx, subpath);
			JsonNode *child_son = json_mkobject();
            sprintf(prefix_key, "js_idx_%s", k);        
            if (vtype == ezJSON_NUMBER)
    			json_append_member(child_son, prefix_key, json_mknumber(atof(v)));
            else // vtype = ezJSON_STRING
    			json_append_member(child_son, prefix_key, json_mkstring(v));
			json_append_element(json_child, child_son);
			//js_print(json_child);
			return js_idx_set_path(child_son, subpath);
		}else if (idx!=NULL){ // 2. idx exist
			if (!strcmp(idx,"")){ // idx empty, append to tail
                if (end != NULL){ // subpath exist
                    JsonNode *child_son = json_mkobject();
                    json_append_element(json_child, child_son);
                    return js_idx_set_path(child_son, subpath);
                }
                else // path[] without subpath, return path node
                    return json_child;
			}else{
				//printf("idx:[%s]\n", idx);
				JsonNode *child_son = json_find_element(json_child, atoi(idx));
				return js_idx_set_path(child_son, subpath);
			}
		}
	}else{ // object
//		printf("%s(%d) key:%s k:%s v:%s i:%s subpath:%s\n", __func__, __LINE__, key, k, v, idx, subpath);
		JsonNode *json_child = json_mkobject();
		json_append_member(json, key, json_child);
		return js_idx_set_path(json_child, subpath);
	}
}

/**
 * @brief js_idx_set_path_value js_set_path_value index version
 *
 * Same as js_set_path_value function, use index in the path location to get 
 * correct array member
 *
 * @param *json root json node
 * @param *path path start from json
 * @param *val value to set
 * @param *t type (ezJSON_STRING/ezJSON_NUMBER/ezJSON_BOOL)
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_idx_set_path_value(JsonNode *json, char *path, void *val, JsonTag t)
{
    JsonNode *j, *o;
	if (json == NULL || path == NULL)
		return JS_FAIL;
	
	char *base_path, *nodename, *start, *end;
	char *s, *e, *idx = NULL;
	char buffer[128];

	strcpy(buffer, path);
	base_path = buffer;
	end = strrchr(buffer, '/');

	if (end != NULL){
		*end = '\0';
		nodename = end + 1;
        
    	j = js_idx_set_path(json, base_path);
        if (j == NULL)
            return JS_FAIL;
        if (j->tag == ezJSON_ARRAY){ // base path is empty array
            // TODO: array do something
            o = json_mkobject();
			json_append_element(j, o);
            j = o;
        }
	}
    else { // no base_path, just nodename
        base_path = NULL;
        nodename = buffer;
        j = json;        
    }
//    printf("base_path:%s nodename:[%s]\n", base_path, nodename);	
	if ( (s = strchr(nodename, '[')) && (e = strchr(nodename,']'))){ // array
		*s = '\0';
		idx = s+1;
		*e = '\0';
    }

	// nodename exist?
    JsonNode *k = js_idx_get_path(j, nodename);
    JsonNode *v;
    if (k == NULL ||  // node not exist
            (k != NULL && k->tag == ezJSON_ARRAY)){ // node is array
        switch(t){
            case ezJSON_BOOL:
                v = json_mkbool(*((bool *)val));
                break;
            case ezJSON_STRING:
                v = json_mkstring((char *)val);
                break;
            case ezJSON_NUMBER:
                v = json_mknumber(*((double *)val));
                break;
            case ezJSON_ARRAY:
            case ezJSON_OBJECT:
            default:
                return JS_FAIL;
        }
    }

    if (k == NULL){
        if (idx!=NULL){ // k is array
            JsonNode *tmp = json_mkarray();
		    json_append_element(tmp, v);
    		json_append_member(j, nodename, tmp);
        }
        else
    		json_append_member(j, nodename, v);
        return 1;
    }
    if (k->tag == ezJSON_ARRAY){
		json_append_element(k, v);
    }

    // nodename exist, not array
    if (k->tag != t){
//        printf("%s(%d): type mismatch\n", __func__,__LINE__);
        return JS_FAIL;
    }
    switch(t){
        case ezJSON_BOOL:
            k->bool_ = *((bool *)val);
            break;
        case ezJSON_STRING:
            if (k->string_ != NULL)
                free(k->string_);
            k->string_ = strdup((char *)val);
            break;
        case ezJSON_NUMBER:
            k->number_ = *((double *)val);
            break;
        case ezJSON_ARRAY:
        case ezJSON_OBJECT:
            printf("fail\n");
        default:
            return JS_FAIL;
    }
    return 1; 
}

/**
 * @brief js_idx_set_path_int js_set_path_int index version
 *
 * js_set_path_int index version, use index mechanism to locate the array
 *
 * @param *json root json node
 * @param *path path start from json
 * @param val value to set, integer type
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_idx_set_path_int(JsonNode *json, char *path, int val)
{
    double v = (double)val;
    return js_idx_set_path_value(json, path, (void *)&v, ezJSON_NUMBER);
}

/**
 * @brief js_idx_set_path_double js_set_path_double index version
 *
 * js_set_path_double index version, use index mechanism to locate the array
 *
 * @param *json root json node
 * @param *path path start from json
 * @param val value to set, number(double) type
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_idx_set_path_double(JsonNode *json, char *path, double val)
{
    return js_idx_set_path_value(json, path, (void *)&val, ezJSON_NUMBER);
}

/**
 * @brief js_idx_set_path_str js_set_path_str index version
 *
 * js_set_path_int index version, use index mechanism to locate the array 
 *
 * @param *json root json node
 * @param *path path start from json
 * @param *val value to set, string type
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_idx_set_path_str(JsonNode *json, char *path, char *val)
{
    return js_idx_set_path_value(json, path, (void *)val, ezJSON_STRING);
}

/**
 * @brief js_idx_set_path_bool js_set_path_bool index version
 *
 * js_set_path_bool index version, use index mechanism to locate the array
 *
 * @param *json root json node
 * @param *path path start from json
 * @param val value to set, bool type
 * @return integer 1 for success, JS_FAIL if failure
 *
 */
int js_idx_set_path_bool(JsonNode *json, char *path, bool val)
{
    bool v = (bool)val;
    return js_idx_set_path_value(json, path, (void *)&v, ezJSON_BOOL);
}

