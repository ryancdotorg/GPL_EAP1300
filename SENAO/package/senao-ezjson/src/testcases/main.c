#include <stdio.h>

#include <ezjson.h>
#include <string.h>

char *s1="[ \
        { \
                \"MBV\": 0, \
                \"CRRC\": 0, \
                \"LFrei\": 0 \
        }                \
]";
char *s2="[ \
        { \
                \"MBV\": 0,FAB \
                \"CRRC\": 0, \
                \"LFrei\": 0 \
        }                \
]";

static void validate_file(char *fname, char *fname_expect)
{
	char buf[1024], buf_expect[1024];
	FILE *fp, *fp_expect;
	int len, len_expect;
	if ((fp = fopen(fname,"r"))==NULL){
		printf("FAIL\n");
		return;
	}
	if ((fp_expect = fopen(fname_expect,"r"))==NULL){
		printf("FAIL\n");
		return;
	}
	if ((len = fread(buf, 1, sizeof(buf), fp)) <= 0 ){
		printf("FAIL\n");
		return;
	}
	if ((len_expect = fread(buf_expect, 1, sizeof(buf_expect), fp_expect)) <= 0 ){
		printf("FAIL\n");
		return;
	}
	if (len != len_expect){
		printf("FAIL, len:%d len_expect:%d\n", len, len_expect);
		return;
	}
	if (memcmp(buf, buf_expect, len)){
		printf("FAIL\n");
		return;
	}
	printf("PASS\n");
}

static void validate_string(char *val, char *expect)
{
	if (!strcmp(val, expect))
		printf("PASS, val:%s expect:%s\n", val, expect);
	else
		printf("FAIL, val:%s expect:%s\n", val, expect);

}
static void validate_int(int val, int expect)
{
	if (val == expect)
		printf("PASS, val:%d expect:%d\n", val, expect);
	else
		printf("FAIL, val:%d expect:%d\n", val, expect);
}
static void validate_double(double val, double expect)
{
	if (val == expect)
		printf("PASS, val:%f expect:%f\n", val, expect);
	else
		printf("FAIL, val:%f expect:%f\n", val, expect);
}
static void validate_null(void *val, int expect) // 0: expect null; 1: expect not null
{
	if ((expect == 0 && val == NULL) | (expect == 1 && val != NULL))
		printf("PASS, expect %s\n", expect==0?"null":"not null");
	else
		printf("FAIL, expect %s\n", expect==0?"null":"not null");

}
void test_js_equal_1()
{
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json4.txt");
	jin2 = js_parse_file("json4-1.txt");
    s1 = js_get_path(jin1, "data1");
    s2 = js_get_path(jin2, "data1");
//    js_print(s1);
//    js_print(s2);
	validate_int(js_equal(s1,s2), 57);
}
void test_js_equal_2()
{
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json4.txt");
	jin2 = js_parse_file("json4-1.txt");
    s1 = js_get_path(jin1, "data2");
    s2 = js_get_path(jin2, "data2");
//    js_print(s1);
//    js_print(s2);
	validate_int(js_equal(s1,s2), 4005);
}
void test_js_equal_3()
{
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json4.txt");
	jin2 = js_parse_file("json4-1.txt");
    s1 = js_get_path(jin1, "data3");
    s2 = js_get_path(jin2, "data3");
//    js_print(s1);
//    js_print(s2);
	validate_int(js_equal(s1,s2), -1);
}
void test_js_equal_4()
{
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json4.txt");
	jin2 = js_parse_file("json4-1.txt");
    s1 = js_get_path(jin1, "data4");
    s2 = js_get_path(jin2, "data4");
//    js_print(s1);
//    js_print(s2);
	validate_int(js_equal(s1,s2), -6);
}
void test_js_equal_5()
{
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json4.txt");
	jin2 = js_parse_file("json4-1.txt");
    s1 = js_get_path(jin1, "data5");
    s2 = js_get_path(jin2, "data5");
//    js_print(s1);
//    js_print(s2);
	validate_int(js_equal(s1,s2), -6);
}
void test_js_equal_6()
{
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json4.txt");
	jin2 = js_parse_file("json4.txt");
    s1 = jin1;
    s2 = jin2;
//    js_print(s1);
//    js_print(s2);
	validate_int(js_equal(s1,s2), 0);
}
void test_js_node_equal_1()
{
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json5.txt");
	jin2 = js_parse_file("json5-1.txt");
    s1 = js_get_path(jin1, "data6");
    s2 = js_get_path(jin2, "data6");
//    js_print(s1);
//    js_print(s2);
	validate_int(js_node_equal(s1,s2), 0);
}
void test_js_dup_1()
{
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json5.txt");
    jin2 = js_dup(jin1);
    validate_int(js_equal(jin1,jin2), 0);
}
void test_js_parse_str_1()
{
	JsonNode *jin;
	jin = js_parse_str(s1);
	validate_null(jin,1);
	js_free(jin);
}
void test_js_parse_str_2()
{
	JsonNode *jin;
	jin = js_parse_str(s2);
	validate_null(jin, 0);
	js_free(jin);
}

void test_js_parse_file_1()
{
	JsonNode *jin;
	jin = js_parse_file("./json1.txt");
	validate_null(jin,1);
	js_free(jin);
}
void test_js_parse_file_2()
{
	JsonNode *jin;
	jin = js_parse_file("./json2.txt");
	validate_null(jin,0);
	js_free(jin);
}
void test_js_print_hr()
{
	JsonNode *jin;
	jin = js_parse_file("json1.txt");

	FILE *fp;
	fp = fopen("./json1_output.txt", "w+");
	js_to_fp_hr(jin, fp);	
	fclose(fp);

	validate_file("json1_output.txt", "json1.txt");
	js_free(jin);
}
//test_js_print
void test_js_to_fp()
{
	JsonNode *jin;
	jin = js_parse_file("json1.txt");

	FILE *fp;
	fp = fopen("./json1_strip_output.txt", "w+");
	js_to_fp(jin, fp);	
	fclose(fp);

	validate_file("json1_strip_output.txt", "json1_strip_expect.txt");
	js_free(jin);
}
void test_js_to_str()
{
	JsonNode *jin;
	jin = js_parse_file("json1.txt");

	char *tmp = js_to_str(jin);
    printf("%s\n",tmp);
//	validate_file("json1_strip_output.txt", "json1_strip_expect.txt");
	js_free(jin);
}
//test_js_print
void test_js_get_path_1()
{
	JsonNode *jin;
	jin = js_parse_file("json3.txt");
	JsonNode *jin2;
	jin2 = js_get_path(jin, "data[1]/blocked_categories[2]");
	js_print(jin2);
}
void test_js_get_path_2()
{
	JsonNode *jin;
	jin = js_parse_file("json3.txt");
	JsonNode *jin2;
	jin2 = js_get_path(jin, "data[name=senao]/blocked_categories[0]");
	js_print(jin2);
}
void test_js_get_path_3()
{
	JsonNode *jin;
	jin = js_parse_file("json3.txt");
	JsonNode *jin2;
//    jin2 = js_get_path(jin, "data[name=\"senao\"]");
//	jin2 = js_get_path(jin, "data[name=\"senao\"]/blocked_categories[1]");
//    jin2 = js_get_path(jin, "data[type=1]");
//    jin2 = js_get_path(jin, "data[type=2]");
//    jin2 = js_get_path(jin, "data[name=\"senao\"]/bed_time[1]");
    jin2 = js_get_path(jin, "data[name=\"senao\"]/bed_time[from=1262]");
	js_print(jin2);
}
void test_js_get_path_4()
{
	JsonNode *jin;
	jin = js_parse_file("json9.txt");
	JsonNode *jin2;
//    jin2 = js_get_path(jin, "wireless");
    //jin2 = js_get_path(jin, "wireless/clients");
    //jin2 = js_get_path(jin, "wireless/clients[1]/ip");
    jin2 = js_get_path(jin, "wireless/clients[rssi=3]/ip");
	js_print(jin2);
}
void test_js_idx_get_path_1()
{
	JsonNode *jin;
	jin = js_parse_file("json16.txt");
	JsonNode *jin2;
//    jin2 = js_get_path(jin, "wireless");
    //jin2 = js_get_path(jin, "wireless/clients");
    //jin2 = js_get_path(jin, "wireless/clients[1]/ip");
    jin2 = js_get_path(jin, "Data");
    printf("data:\n");
	js_print(jin2);
    printf("action:\n");
    jin2 = js_idx_get_path(jin2, "CFG_RAM/Action[act_name=\"act_linkcheck\"]");
	js_print(jin2);

}
void test_js_get_path_str_1()
{
	JsonNode *jin;
	jin = js_parse_file("json3.txt");
	char *res = js_get_path_str(jin, "data[name=\"senao\"]/box_ssn");
	if (res == NULL){
		printf("FAIL\n");
		return;
	}
	validate_string(res, "AR9344-8CC7AA01B231");
}
void test_js_get_path_strz_1()
{
	JsonNode *jin;
	jin = js_parse_file("json3.txt");
	char *res = js_get_path_strz(jin, "data[name=\"senao\"]/has_icon");
	validate_string(res, "");
}
void test_js_get_path_strz_2()
{
	JsonNode *jin;
	jin = js_parse_file("json3.txt");
	char *res = js_get_path_strz(jin, "data[name=\"senao\"]/box_ssn");
	validate_string(res, "AR9344-8CC7AA01B231");
}
void test_js_get_path_strz_3()
{
	JsonNode *jin;
	jin = js_parse_file("json3.txt");
	char *res = js_get_path_strz(jin, "data[0]/box_ssn");
	validate_string(res, "AR9344-8CC7AA01B231");
}

void test_js_get_path_strz_4()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json3.txt");
	jin2 = js_get_path(jin, "data[0]");
	char *res = js_get_path_strz(jin2, "name");
	validate_string(res, "Main Profile");
}
void test_js_get_path_int_1()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json3.txt");
	jin2 = js_get_path(jin, "data[0]");
	int res = js_get_path_int(jin2, "type");
	validate_int(res, 1);
}
void test_js_get_path_int_2()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json3.txt");
	jin2 = js_get_path(jin, "data[name=\"senao\"]");
	int res = js_get_path_int(jin2, "blocked_categories[1]");
	validate_int(res, 120);
}
void test_set_path_1()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json3.txt");
	jin2 = js_set_path(jin, "data[name=\"senao\"]/testkey");
}
void test_set_path_int_1() // not exist key
{
	JsonNode *jin;
    int expect = 5, res, ret;
	jin = js_parse_file("json3.txt");
    ret = js_set_path_int(jin, "data[name=\"senao\"]/testkey", expect);
	res = js_get_path_int(jin, "data[name=\"senao\"]/testkey");
	validate_int(res, expect);
}
void test_set_path_int_2() // exist key
{
	JsonNode *jin;
    int expect = 3, res, ret;
	jin = js_parse_file("json3.txt");
    ret = js_set_path_int(jin, "data[name=\"senao\"]/type", expect);
	res = js_get_path_int(jin, "data[name=\"senao\"]/type");
	validate_int(res, expect);
}
void test_set_path_int_3() // type mismatch
{
	JsonNode *jin;
    int expect = 3, res, ret;
	jin = js_parse_file("json3.txt");
    ret = js_set_path_int(jin, "data[name=\"senao\"]/blocked_webs", expect);
	res = js_get_path_int(jin, "data[name=\"senao\"]/blocked_webs");
	validate_int(res, JS_FAIL);
}
void test_set_path_int_4() // array element
{
	JsonNode *jin;
    int expect = 2, res, ret;
	jin = js_parse_file("json3.txt");
  ret = js_set_path_int(jin, "data[name=\"senao\"]/blocked_webs[]", expect);
//    ret = js_set_path_int(jin, "data[name=\"senao\"]/blocked_webs2", expect);
    js_print_hr(js_get_path(jin, "data[name=\"senao\"]"));
//	res = js_get_path_int(jin, "data[name=\"senao\"]/blocked_webs");
//	validate_int(res, JS_FAIL);
}
void test_idx_set_path_int_1()
{
	JsonNode *jin;
    int expect = 5, res, ret;
	jin = js_parse_file("json12-1.txt");
    js_print_hr(jin);
    ret = js_idx_set_path_int(jin, "wireless/clients[mac_addr=\"4\"]/abc", 33);
//    ret = js_idx_set_path_int(jin, "wireless/clients[]/abc", 33);
    js_print_hr(jin);
    printf("%d\n", ret);
//	res = js_get_path_int(jin, "data[name=\"senao\"]/testkey");
//	validate_int(res, expect);
}

void test_set_path_double_1() // not exist key
{
	JsonNode *jin;
    double expect = 5.1234, res, ret;
	jin = js_parse_file("json3.txt");
    ret = js_set_path_double(jin, "data[name=\"senao\"]/testkey", expect);
	res = js_get_path_double(jin, "data[name=\"senao\"]/testkey");

	validate_double(res, expect);
}
void test_set_path_str_1()
{
	JsonNode *jin;
    char expect[] = "testabc", *res;
    int ret;
	jin = js_parse_file("json3.txt");
    ret = js_set_path_str(jin, "data[name=\"senao\"]/testkey", expect);
	res = js_get_path_str(jin, "data[name=\"senao\"]/testkey");
	
	validate_string(res, expect);
}
void test_set_path_str_2()
{
	JsonNode *jin, *jin2;
    char expect[] = "testabc", *res;
    int ret;
	jin = js_parse_file("json3.txt");
    jin2 = js_get_path(jin, "data[name=\"senao\"]");
//    js_print_hr(jin2);
    //char *str = js_set_path_str(jin2, "box_ssn", "1234");
    js_set_path_str(jin2, "box_ssn", "1234");
    js_print_hr(jin2);
//	validate_string(res, expect);
}
void test_set_path_bool_1()
{
	JsonNode *jin;
    bool expect = false, res;
    int ret;
	jin = js_parse_file("json3.txt");
    ret = js_set_path_bool(jin, "data[name=senao]/management_enable", expect);
	res = js_get_path_bool(jin, "data[name=senao]/management_enable");
	
	validate_int(res, expect);
}
void test_set_path_bool_2()
{
	JsonNode *jin;
    bool expect = true, res;
    int ret;
	jin = js_parse_file("json3.txt");
    ret = js_set_path_bool(jin, "data[name=senao]/management_enable", expect);
	res = js_get_path_bool(jin, "data[name=senao]/management_enable");
	
	validate_int(res, expect);
}
void test_set_path_bool_3()
{
	JsonNode *jin;
    bool expect = false, res;
    int ret;
	jin = js_parse_file("json3.txt");
	res = js_get_path_bool(jin, "data[name=senao]/management_enable");
	validate_int(res, expect);
}
void test_js_union_1()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json6.txt");
	jin2 = js_parse_file("json6-1.txt");
//    s1 = js_get_path(jin1, "data1");
//    s2 = js_get_path(jin2, "data1");
    s1 = jin1;
    s2 = jin2;
//    js_print(s1);
//    js_print(s2);
    js_union(s1,s2);
//    js_print_hr(s1);
    validate_string(js_get_path_str(s1, "data1"), "test339");
    validate_int(js_get_path_int(s1, "data2"), 4449);
    validate_int(js_get_path_bool(s1, "data3"), 0);
    validate_string(js_get_path_str(s1, "data4/b1"), "test11");
    validate_int(js_get_path_int(s1, "data4/b2"), 111);
    validate_int(js_get_path_bool(s1, "data4/b3"), 1);
    validate_int(js_get_path_int(s1, "data5[0]"), 1);
    validate_int(js_get_path_int(s1, "data5[1]"), 2);
    validate_int(js_get_path_int(s1, "data5[2]"), 3);
    validate_int(js_get_path_int(s1, "data5[3]"), 4);
    validate_int(js_get_path_int(s1, "data5[4]"), 5);
    validate_int(js_get_path_bool(s1, "data6[3]/a3"), 1);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_union_2()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json7.txt");
	jin2 = js_parse_file("json7-1.txt");
    s1 = js_get_path(jin1, "data6[2]");
    s2 = js_get_path(jin2, "data6[0]");
    js_union(s1,s2);
    js_print_hr(jin1);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_union_3()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json9.txt");
	jin2 = js_parse_str("{}");
    js_print_hr(jin2);
    printf("======\n");
    js_union(jin2,jin1);
    js_print_hr(jin2);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_union_4()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json9.txt");
	jin2 = js_parse_str("{\"rssi\":4}");
    s1 = js_get_path(jin1,"wireless/clients[ip=\"5.6.7.8\"]");
    s2 = jin2;
    //js_print(s1);
    //js_print(s2);
    js_union(s1,s2);
    js_print_hr(s1);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_union_5()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json10.txt");
	jin2 = js_parse_file("json10-1.txt");
    js_print_hr(jin1);
    printf("=== =====\n");
    js_print_hr(jin2);
//    js_set_path(jin2, "wireless");
//    js_set_path(jin2, "wireless/clients");
//    js_idx_set_path(jin2, "wireless/clients[rssi=1]");
//    js_set_path_int(jin2, "wireless/clients[js_idx_rssi=1]/abc", 1);
//    js_set_path_int(jin2, "wireless/clients[js_idx_ip=\"5.6.7.8\"]/abc", 1);
//    js_set_path_int(jin2, "wireless/clients[js_idx_ip=\"1.2.3.4\"]/ssids[js_idx_id=\"55\"]/traffic[js_idx_id=\"0\"]/go", 1);
//    js_set_path_int(jin2, "wireless/clients[js_idx_ip=\"1.2.3.4\"]/untest[]", 4);
//    js_set_path_int(jin2, "wireless/clients[js_idx_ip=\"1.2.3.4\"]/untest[]", 5);
//    js_print_hr(jin2);
//    s1 = js_get_path(jin1,"wireless/clients[ip=\"5.6.7.8\"]");
//    s2 = jin2;
//    js_print(s1);
//    js_print(s2);
    js_union(jin1,jin2);
    js_print_hr(jin1);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_union_6()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json12.txt");
	jin2 = js_parse_file("json12-1.txt");
    printf("=== jin1 =====\n");
    js_print_hr(jin1);
    printf("=== jin2 =====\n");
    js_print_hr(jin2);
    js_union(jin2,jin1);
    printf("=== jin2 =====\n");
    js_print_hr(jin2);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_union_7()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json13.txt");
	jin2 = js_parse_file("json13-1.txt");
    printf("=== jin1 =====\n");
    js_print_hr(jin1);
    printf("=== jin2 =====\n");
    js_print_hr(jin2);
    js_union(jin2,jin1);
    printf("=== jin2 =====\n");
    js_print_hr(jin2);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_union_8()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json14.txt");
    jin2 = js_parse_str("{}");

    js_set_path_str(jin2, "Header/source", "act_sitesurvey");
    js_set_path_int(jin2, "Header/type", 20);
    js_set_path(jin2, "Data");
    printf("=== jin1 =====\n");
    js_print_hr(jin1);
    printf("=== jin2 =====\n");
    js_print_hr(jin2);
    s2 = js_get_path(jin2, "Data");
    printf("=== Data =====\n");
    js_print_hr(s2);
    js_union(js_get_path(jin2, "Data"),jin1);
    printf("=== jin2 =====\n");
    js_print_hr(jin2);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_join_1()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json8.txt");
	jin2 = js_parse_file("json8-1.txt");
    s1 = js_get_path(jin1, "data6[2]");
    s2 = js_get_path(jin2, "data6[0]");
//    s1 = jin1;
//    s2 = jin2;
//    js_print(s1);
//    js_print(s2);
    js_join(s2,s1);
    js_print_hr(s2);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_join_2()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json10.txt");
	jin2 = js_parse_file("json10-1.txt");
    js_print_hr(jin1);
    printf("=== =====\n");
    js_print_hr(jin2);
    js_join(jin1,jin2);
    js_print_hr(jin1);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_join_3()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json15.txt");
	jin2 = js_parse_file("json15-1.txt");
    js_print_hr(jin1);
    printf("=== jin2 =====\n");
    //js_print_hr(jin2);
    js_join(jin1,jin2);
    printf("=== jin 1 =====\n");
    js_print_hr(jin1);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_set_path_2()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json3.txt");
	jin2 = js_set_path(jin, "data[2]/testkey");
}
void test_set_path_3()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json3.txt");
	jin2 = js_set_path(jin, "data[]/testkey");
}
void test_set_path_4()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json3.txt");
	jin2 = js_set_path(jin, "data2[]/testkey");
}
void test_set_path_5()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json3.txt");
	jin2 = js_set_path(jin, "data3/testkey");
}
void test_set_path_6()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json9.txt");
	jin2 = js_set_path(jin, "wireless/clients[rssi=1]/test1");
    js_print_hr(jin);
}
void test_set_path_7()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json3.txt");
	jin2 = js_set_path(jin, "data5[]");
    js_print_hr(jin);
}
void test_set_path_8()
{
	JsonNode *jin = json_mkobject(), *t;
    js_set_path_str(jin, "mac_addr", "11:11:22:22:22:33");
    js_set_path_str(jin, "ifname", "ath10");
//    if (js_get_path(jin,"ssids[ifname=\"ath10\"]") == NULL){
        printf("%s(%d)\n", __func__, __LINE__);
//        t = js_set_path(jin, "ssids[]");
        printf("%s(%d)\n", __func__, __LINE__);
        js_set_path_str(jin, "ssids[]/ifname", "ath10");
        printf("%s(%d)\n", __func__, __LINE__);
//        js_set_path_str(jin, "ssids[]/ifname", "ath10");
//    }
//    js_set_path_str(jin, "ssids[abc=\"1234\"]/abc","1234");
    js_print_hr(jin);
}
void test_free_path_1()
{
	JsonNode *jin;
	jin = js_parse_file("json7.txt");

    js_print_hr(jin);
	js_free_path(jin, "data6[a1=test11]");
    js_print_hr(jin);
}
void test_js_free_js_1()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json7.txt");
    jin2 = js_parse_str("{}");
    js_set_path(jin2, "data1");
//    js_set_path(jin2, "data2");
//    js_set_path(jin2, "data3");
//    js_set_path(jin2, "data4");
//    js_set_path(jin2, "data4/b1");
//    js_set_path(jin2, "data5"); // can not remove single array element
//    js_set_path(jin2, "data6[]/a3");  // TODO: delete data6[2]/a3, how to present 2?
    js_print_hr(jin);
    printf("----\n");
    js_print_hr(jin2);
    printf("============\n");
	js_free_js(jin, jin2);
    js_print_hr(jin);
}
void test_js_get_js_1()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json7.txt");
    jin2 = js_parse_str("{}");
//    js_set_path(jin2, "data1");
//    js_set_path(jin2, "data2");
//    js_set_path(jin2, "data3");
//    js_set_path(jin2, "data4");
//    js_set_path(jin2, "data4/b1");
//    js_set_path(jin2, "data5"); // can not remove single array element
    js_set_path(jin2, "data6[]/a3");  // TODO: get data6[2]/a3, how to present 2?
    js_print_hr(jin);
    printf("----\n");
    js_print_hr(jin2);
    printf("============\n");
    js_print_hr(js_get_js(jin,jin2));
}
void test_js_get_js_2()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json9.txt");
    jin2 = js_parse_str("{}");
//    js_set_path(jin2, "wireless");
//    js_set_path(jin2, "wireless/clients");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:44:55:66]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=55]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=56]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=55]/traffic[id=0]");
    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=55]/traffic[id=1]");
	js_print_hr(jin2);
    printf("----\n");
    js_print_hr(js_get_js(jin,jin2));
}
void test_js_get_js_3()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json9.txt");
    jin2 = js_parse_str("{}");
//    js_set_path(jin2, "wireless");
//    js_set_path(jin2, "wireless/clients");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:44:55:66]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=55]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=56]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=55]/traffic[id=0]");
    js_set_path_int(jin2, "wireless/clients[rssi=1]/rate", 6);
    js_union(jin, jin2);
	js_print_hr(jin2);
    printf("----++\n");
	js_print_hr(jin);
    //js_print_hr(js_get_js(jin,jin2));
}
void test_js_get_idx_js_1()
{
	JsonNode *jin, *jin2;
	jin = js_parse_file("json9.txt");
    jin2 = js_parse_str("{}");
//    js_set_path(jin2, "wireless");
//    js_set_path(jin2, "wireless/clients");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:44:55:66]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=55]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=56]");
//    js_set_path(jin2, "wireless/clients[mac_addr=aa:bb:cc:11:22:33]/ssids[id=55]/traffic[id=0]");
//    js_set_path_int(jin2, "wireless/clients[rssi=0]/rate", 1);
    js_set_path_int(jin2, "wireless/clients[js_idx_rssi=1]/rate", 1);
    js_print_hr(jin2);
    js_print_hr(js_get_idx_js(jin, jin2));
    printf("----++\n");
//	js_print_hr(jin);
    //js_print_hr(js_get_js(jin,jin2));
}
void test_js_idx_join_1()
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2;
	jin1 = js_parse_file("json9.txt");
	jin2 = js_parse_str("{}");
//    js_set_path(jin2, "wireless/clients[js_idx_rssi=4]/ssids[js_idx_id=\"56\"]");
//    js_set_path(jin2, "wireless/clients[js_idx_rssi=1]/ssids[js_idx_id=\"55\"]");
//    js_set_path(jin2, "wireless/clients[js_idx_rssi=1]/ssids[js_idx_id=\"55\"]/traffic[js_idx_id=\"0\"]");
    js_set_path(jin2, "wireless/clients[js_idx_rssi=3]/ip");
    js_print_hr(jin2);
    printf("----++\n");
    js_join(jin2, jin1);
    js_print_hr(jin2);
    printf("=== [%s] testing end ===\n", __func__);
}
void test_js_idx_join_2() // idx_join
{
    printf("=== [%s] testing ===\n", __func__);
	JsonNode *jin1, *jin2, *s1, *s2;
	jin1 = js_parse_file("json11.txt");
	jin2 = js_parse_file("json11-1.txt");
    js_print_hr(jin1);
    printf("=== =====\n");
    js_print_hr(jin2);
    js_join(jin1,jin2);
    js_print_hr(jin1);
    printf("=== [%s] testing end ===\n", __func__);
}

static char ezmformat[] = 
"{"
"	\"EZMCloud\": {"
"		\"wireless\": {"
"			\"ssid_map\": ["
"				{"
"					\"id\": 1,"
"					\"ifname\": \"ath0\""
"				}"
"			]"
"		}"
"	}"
"}";

int main(void)
{
//    test_idx_set_path_int_1();
//    test_set_path_double_1();
//    test_set_path_8();

//    test_js_join_1();
//    test_js_join_2();
//    test_js_join_3();
//      test_js_idx_join_2();
//    test_js_idx_join_1();
//
//    test_set_path_str_2();

//    test_js_union_1();
//    test_js_union_2();
//    test_js_union_3();
//    test_js_union_4();
//    test_js_union_5();
//    test_js_union_6();
//    test_js_union_7();
//    test_js_union_8();
//    test_set_path_bool_1();
//    test_set_path_bool_2();
//    test_set_path_bool_3();

//    test_js_get_path_4();
	test_js_idx_get_path_1();
//    test_js_get_js_3();
//    test_js_get_idx_js_1();
/*
	test_set_path_1();
	test_set_path_2();
	test_set_path_3();
	test_set_path_4();
	test_set_path_5();
	test_set_path_int_1();
    test_set_path_int_2();
    test_set_path_int_3();
    test_set_path_int_4();
    test_set_path_double_1();
    test_set_path_str_1();
    test_set_path_bool_1();
    test_set_path_bool_2();
    test_js_to_str();
    test_js_equal_1();
    test_js_equal_2();
    test_js_equal_3();
    test_js_equal_4();
    test_js_equal_5();
    test_js_equal_6();
    test_js_node_equal_1();
    test_js_dup_1();
    test_js_parse_str_1();
	test_js_parse_str_2();
	test_js_parse_file_1();
	test_js_parse_file_2();
	test_js_to_fp();
	test_js_get_path_str_1();
	test_js_get_path_strz_1();
	test_js_get_path_strz_2();
	test_js_get_path_strz_3();
	test_js_get_path_strz_4();
	test_js_get_path_int_1();
	test_js_get_path_int_2();
    test_free_path_1();
    test_js_free_js_1();
    test_js_get_js_1();
*/

//    test_js_get_js_2();
//    test_js_get_path_3();
/*
	//test_js_print_hr();

*/
	return 0;
}

