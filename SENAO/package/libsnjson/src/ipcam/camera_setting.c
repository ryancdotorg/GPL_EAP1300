#include "common.h"

int json_get_camera_camera_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc camera");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_camera_camera_default_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc camera_default");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_camera_camera_settings(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char json_data[MAX_JSON_REPLY_LEN] = {0};
	char cmd[MAX_JSON_REPLY_LEN] = {0};

	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		addBackslashAndNewline(query_str, json_data);
		//debug_print("--- Jason DEBUG %s[%d] addBackslash: #.%s.#\n", __FUNCTION__, __LINE__, json_data);
		// ipcammgr_cli setfunc advanced JSON {\"bind\": \"1\", \"device_name\": \"yoyozz\"}
		snprintf(cmd, sizeof(cmd), "ipcammgr_cli setfunc camera JSON %s", json_data);
		sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_camera_advanced_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc advanced");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_camera_advanced_settings(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char json_data[MAX_JSON_REPLY_LEN] = {0};
	char cmd[MAX_JSON_REPLY_LEN] = {0};

	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		addBackslashAndNewline(query_str, json_data);
		//debug_print("--- Jason DEBUG %s[%d] addBackslash: #.%s.#\n", __FUNCTION__, __LINE__, json_data);
		// ipcammgr_cli setfunc advanced JSON {\"bind\": \"1\", \"device_name\": \"yoyozz\"}
		snprintf(cmd, sizeof(cmd), "ipcammgr_cli setfunc advanced JSON %s", json_data);
		sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);
	}
	// parse json_reply
	// GEN VALUE ERROR MSG
	//RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "APPLE");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_camera_advanced_default_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc advanced_default");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_camera_video_streams_capability(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc video_streams_capability");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	//debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);
	//debug_print("--- JSON DEBUG %s[%d]: stream_idx: %d\n", __FUNCTION__, __LINE__, stream_idx);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_camera_video_stream_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply, int stream_idx)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	debug_print("--- JSON DEBUG %s[%d]: stream_idx: %d\n", __FUNCTION__, __LINE__, stream_idx);

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc video_stream_settings %d", stream_idx);
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	//debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);
	//debug_print("--- JSON DEBUG %s[%d]: stream_idx: %d\n", __FUNCTION__, __LINE__, stream_idx);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_camera_video_stream_settings(ResponseEntry *rep, char *query_str, char *json_reply, int stream_idx)
{
	char json_data[MAX_JSON_REPLY_LEN] = {0};
	char cmd[MAX_JSON_REPLY_LEN] = {0};

	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		addBackslashAndNewline(query_str, json_data);
		//debug_print("--- Jason DEBUG %s[%d] addBackslash: #.%s.#\n", __FUNCTION__, __LINE__, json_data);
		// ipcammgr_cli setfunc advanced JSON {\"bind\": \"1\", \"device_name\": \"yoyozz\"}
		snprintf(cmd, sizeof(cmd), "ipcammgr_cli setfunc video_stream_settings JSON %d %s", stream_idx, json_data);
		sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

#if 0
bool get_audio_stream_capability_json_cb(struct json_object *jobj)
{
	struct json_object *jobj_stream_info;
	struct json_object *jarr_g711_sr, *jobj_g711_sr, *jstr_g771_sr, *jint_g711_sr;
	struct json_object *jarr_g711_br, *jobj_g711_br, *jstr_g771_br, *jint_g711_br;
	struct json_object *jarr_aac_sr, *jobj_aac_sr, *jstr_aac_sr, *jint_aac_sr;
	struct json_object *jarr_aac_br, *jobj_aac_br, *jstr_aac_br, *jint_aac_br;

	int i = 0;

	//json_object_object_add(jobj, "Result", json_object_new_string("OK"));
	jobj_stream_info = json_object_new_object();

	// G711 Sample Rate
	jarr_g711_sr = json_object_new_array();
	for (i = 0; i < AUDIO_SAMPLE_RATE_MAX; i++)
	{
		if(SUPPORT_AUDIO_G711_SAMPLE_RATE & (1 << i))
		{
			jobj_g711_sr = json_object_new_object();

			jint_g711_sr = json_object_new_int(audio_samplerate[i]);
			json_object_object_add(jobj_g711_sr, "Value" ,jint_g711_sr);

			json_object_array_add(jarr_g711_sr, jobj_g711_sr);
		}
	}
	json_object_object_add(jobj_stream_info, "G711SampleRate", jarr_g711_sr);

	// G711 Bit Rate
	jarr_g711_br = json_object_new_array();
	for (i = 0; i < AUDIO_BIT_MAX; i++)
	{
		if(SUPPORT_AUDIO_G711_BIT_RATE & (1 << i))
		{
			jobj_g711_br = json_object_new_object();

			jint_g711_br = json_object_new_int(audio_bitrate[i]);
			json_object_object_add(jobj_g711_br, "Value",jint_g711_br);

			json_object_array_add(jarr_g711_br, jobj_g711_br);
		}
	}
	json_object_object_add(jobj_stream_info, "G711BitRate", jarr_g711_br);

#if IPCAM_AUDIO_CODEC_SUPPORT_AAC
	// AAC Sample Rate
	jarr_aac_sr = json_object_new_array();
	for (i = 0 ; i < AUDIO_SAMPLE_RATE_MAX; i++)
	{
		if(SUPPORT_AUDIO_AAC_SAMPLE_RATE & (1 << i))
		{
			jobj_aac_sr = json_object_new_object();

			jint_aac_sr = json_object_new_int(audio_samplerate[i]);
			json_object_object_add(jobj_aac_sr, "Value", jint_aac_sr);

			json_object_array_add(jarr_aac_sr, jobj_aac_sr);
		}
	}
	json_object_object_add(jobj_stream_info, "AACSampleRate", jarr_aac_sr);

	// AAC Bit Rate
	jarr_aac_br = json_object_new_array();
	for (i = 0; i < AUDIO_BIT_MAX; i++)
	{
		if(SUPPORT_AUDIO_AAC_BIT_RATE & (1 << i))
		{
			jobj_aac_br = json_object_new_object();

			jint_aac_br = json_object_new_int(audio_bitrate[i]);
			json_object_object_add(jobj_aac_br, "Value", jint_aac_br);

			json_object_array_add(jarr_aac_br, jobj_aac_br);
		}
	}
	json_object_object_add(jobj_stream_info, "AACBitRate", jarr_aac_br);
#endif

	json_object_object_add(jobj, "audio_stream_capability_info", jobj_stream_info);

	return 0;
}
#endif
int json_get_camera_audio_stream_capability(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc audio_stream_capability");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	//debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);
	//debug_print("--- JSON DEBUG %s[%d]: stream_idx: %d\n", __FUNCTION__, __LINE__, stream_idx);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#if 0
bool parse_json_ipcam_audio_stream_settings(char *query_str, IPCAM_AUDIO_SETTINGS_T *audio_settings, char *return_str)
{
	bool result, is_jobj;
	struct json_object *jobj, *jobj_stream_settings, *jobj_enable, *jobj_mic, *jobj_speaker;
	struct json_object *jobj_inputGain, *jobj_outputVol;
	struct json_object *jobj_encording, *jobj_bitRate, *jobj_sampleRate, *jobj_alarmLevel;

	ResponseEntry *rep = Response_create();
	char *AudioStreamSettings = NULL;

	result = TRUE;
	is_jobj = FALSE;

	if(jobj = jsonTokenerParseFromStack(rep, query_str))
	{
		//cprintf("%s[%d] query_str: [%s]\n", __FUNCTION__, __LINE__, query_str);

		senao_json_object_get_and_create_string(rep, jobj, "audio_stream", &AudioStreamSettings);

		if(jobj_stream_settings = jsonTokenerParseFromStack(rep, AudioStreamSettings))
		{
			if((jobj_enable = json_object_object_get(jobj_stream_settings, "Enable")))
			{
				audio_settings->enable = json_object_get_boolean(jobj_enable);
				/* Free obj */
				json_object_put(jobj_enable);
			}
			else
			{
				return_str = "ERROR";
				result = FALSE;
				goto out;
			}

			if((jobj_mic = json_object_object_get(jobj_stream_settings, "InputEnable")))
			{
				audio_settings->MIC = json_object_get_boolean(jobj_mic);

				/* Free obj */
				json_object_put(jobj_mic);
			}
			else
			{
				return_str = "ERROR";
				result = FALSE;
				goto out;
			}

			if((jobj_speaker = json_object_object_get(jobj_stream_settings, "OutputEnable")))
			{
				audio_settings->speaker = json_object_get_boolean(jobj_speaker);

				/* Free obj */
				json_object_put(jobj_speaker);
			}
			else
			{
				return_str = "ERROR";
				result = FALSE;
				goto out;
			}

			if((jobj_inputGain = json_object_object_get(jobj_stream_settings, "InputGain")))
			{
				audio_settings->inputGain = json_object_get_int(jobj_inputGain);

				/* Free obj */
				json_object_put(jobj_inputGain);
			}
			else
			{
				return_str = "ERROR";
				result = FALSE;
				goto out;
			}

			if((jobj_outputVol = json_object_object_get(jobj_stream_settings, "OutputVolume")))
			{
				audio_settings->outputVol = json_object_get_int(jobj_outputVol);

				/* Free obj */
				json_object_put(jobj_outputVol);
			}
			else
			{
				return_str = "ERROR";
				result = FALSE;
				goto out;
			}

			if((jobj_encording = json_object_object_get(jobj_stream_settings, "Codec")))
			{
				sprintf(audio_settings->encording, "%s", json_object_get_string(jobj_encording));

				/* Free obj */
				json_object_put(jobj_encording);
			}
			else
			{
				return_str = "ERROR";
				result = FALSE;
				goto out;
			}

			if((jobj_bitRate = json_object_object_get(jobj_stream_settings, "Bitrate")))
			{
				audio_settings->bitRate = json_object_get_int(jobj_bitRate);

				/* Free obj */
				json_object_put(jobj_bitRate);
			}
			else
			{
				return_str = "ERROR";
				result = FALSE;
				goto out;
			}

			if((jobj_sampleRate = json_object_object_get(jobj_stream_settings, "SampleRate")))
			{
				audio_settings->sampleRate = json_object_get_int(jobj_sampleRate);

				/* Free obj */
				json_object_put(jobj_sampleRate);
			}
			else
			{
				return_str = "ERROR";
				result = FALSE;
				goto out;
			}

			if((jobj_alarmLevel = json_object_object_get(jobj_stream_settings, "AlarmLevel")))
			{
				audio_settings->alarmLevel = json_object_get_int(jobj_alarmLevel);

				/* Free obj */
				json_object_put(jobj_alarmLevel);
			}
			else
			{
				return_str = "ERROR";
				result = FALSE;
				goto out;
			}
		}
	}

out:

	if(is_jobj)
	{
		/* Free obj */
		json_object_put(jobj);
		json_object_put(jobj_stream_settings);
	}

	return result;
}

int json_get_camera_audio_stream_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	IPCAM_AUDIO_SETTINGS_T audio_setting;
	T_IPCAM_COMMON_SETTINGS ipcam_settings;

	memset(&audio_setting, 0, sizeof(audio_setting));
	memset(&ipcam_settings, 0, sizeof(ipcam_settings));

	parse_json_ipcam_audio_stream_settings(query_str, &audio_setting, return_str);

#if 1
	cprintf("%s[%d] audio_setting.enable: [%d]\n", __FUNCTION__, __LINE__, audio_setting.enable);
	cprintf("%s[%d] audio_setting.MIC: [%d]\n", __FUNCTION__, __LINE__, audio_setting.MIC);
	cprintf("%s[%d] audio_setting.speaker: [%d]\n", __FUNCTION__, __LINE__, audio_setting.speaker);
	cprintf("%s[%d] audio_setting.inputGain: [%d]\n", __FUNCTION__, __LINE__, audio_setting.inputGain);
	cprintf("%s[%d] audio_setting.outputVol: [%d]\n", __FUNCTION__, __LINE__, audio_setting.outputVol);
	cprintf("%s[%d] audio_setting.bitRate: [%d]\n", __FUNCTION__, __LINE__, audio_setting.bitRate);
	cprintf("%s[%d] audio_setting.sampleRate: [%d]\n", __FUNCTION__, __LINE__, audio_setting.sampleRate);
	cprintf("%s[%d] audio_setting.encording: [%s]\n", __FUNCTION__, __LINE__, audio_setting.encording);
	cprintf("%s[%d] audio_setting.alarmLevel: [%d]\n", __FUNCTION__, __LINE__, audio_setting.alarmLevel);
#endif
	// set uci by api
	if(TRUE != result) goto send_pkt;

	if(TRUE == result)
    {
        return_str = OK_STR;
    }
#if 0
	if (apcfg_ipcam_GetOperationStatus(RECORD_START) == STATUS_START)
	{
		return_str = HNAP_ERROR_RECORDING_NOW_STR;
        goto send_pkt;
	}
#endif
	apcfg_ipcam_GetAudioSettings(&ipcam_settings);

	ipcam_settings.audioEnable  = audio_setting.enable;
	ipcam_settings.inputGain    = audio_setting.inputGain;
	ipcam_settings.outputVolume = audio_setting.outputVol;

	if (audio_setting.MIC==1 && audio_setting.speaker==0)
	{
		ipcam_settings.audioMode = 0;
		ipcam_settings.audioEnable = 1;
	}
	else if (audio_setting.MIC==0 && audio_setting.speaker==1)
	{
		ipcam_settings.audioMode = 1;
		ipcam_settings.audioEnable = 1;
	}
	else if (audio_setting.MIC==1 && audio_setting.speaker==1)
	{
		ipcam_settings.audioMode = 2;
		ipcam_settings.audioEnable = 1;
	}
	else if (audio_setting.MIC==0 && audio_setting.speaker==0)
	{
		ipcam_settings.audioEnable = 0;
	}

	size = sizeof(audio_encording)/sizeof(audio_encording[0]);
	for (i=0;i<size;i++)
	{
		if (strcmp(audio_encording[i],audio_setting.encording)==0)
		{
			ipcam_settings.encoding = i;
			break;
		}
	}

	switch (audio_setting.sampleRate)
	{
		case AUDIO_SAMPLE_RATE_8K:
		case AUDIO_SAMPLE_RATE_16K:
		case AUDIO_SAMPLE_RATE_32K:
		case AUDIO_SAMPLE_RATE_44_1K:
		case AUDIO_SAMPLE_RATE_48K:
		case AUDIO_SAMPLE_RATE_96K:
		case AUDIO_SAMPLE_RATE_128K:
			ipcam_settings.sampleRate = audio_setting.sampleRate;
			break;

		default:
			ipcam_settings.sampleRate = AUDIO_SAMPLE_RATE_8K;
			break;
	}

	switch (audio_setting.bitRate)
	{
		case AUDIO_BIT_RATE_8K:
		case AUDIO_BIT_RATE_16K:
		case AUDIO_BIT_RATE_32K:
		case AUDIO_BIT_RATE_64K:
		case AUDIO_BIT_RATE_128K:
			ipcam_settings.bitRate = audio_setting.bitRate;
			break;

		default:
			ipcam_settings.sampleRate = AUDIO_BIT_RATE_64K;
			break;
	}

#if 0
	size = sizeof(audio_samplerate)/sizeof(audio_samplerate[0]);
	for (i=0;i<size;i++)
	{
		if (audio_samplerate[i]==audio_setting.sampleRate)
		{
			ipcam_settings.sampleRate = i;
			break;
		}
	}

	size = sizeof(audio_bitrate)/sizeof(audio_bitrate[0]);
	for (i=0;i<size;i++)
	{
		if (audio_bitrate[i]==audio_setting.bitRate)
		{
			ipcam_settings.bitRate = i;
			break;
		}
	}
#endif

#if !IPCAM_AUDIO_CODEC_SUPPORT_AAC
	if(ipcam_settings.encoding == AUDIO_ENCODING_AAC_LC)
	{
		ipcam_settings.encoding = AUDIO_ENCODING_G711U;
		ipcam_settings.sampleRate = AUDIO_SAMPLE_RATE_8K;
		ipcam_settings.bitRate = AUDIO_BIT_RATE_64K;
	}
#endif

	if (apcfg_ipcam_ChkAudioSettings(&ipcam_settings) == OK)
	{
		apcfg_ipcam_SetAudioSettings(&ipcam_settings);
	}

	size = sizeof(audio_alarmlevel)/sizeof(audio_alarmlevel[0]);
	for (i=0;i<size;i++)
	{
		if (audio_alarmlevel[i]==audio_setting.alarmLevel)
		{
			ipcam_settings.alarmLevel = i;
			break;
		}
	}

	if (apcfg_ipcam_ChkAudioSettings(&ipcam_settings) == OK)
	{
		apcfg_ipcam_SetAudioDetectSettings(&ipcam_settings);
	}

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc audio_stream_settings");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	//debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);
	//debug_print("--- JSON DEBUG %s[%d]: stream_idx: %d\n", __FUNCTION__, __LINE__, stream_idx);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int get_audio_stream_settings_cb(int defaultVal, IPCAM_AUDIO_SETTINGS_T *audio_settings, T_IPCAM_COMMON_SETTINGS *ipcam_settings)
{
	if(defaultVal)
		apcfg_ipcam_GetAudioDefSettings(ipcam_settings);
	else
		apcfg_ipcam_GetAudioSettings(ipcam_settings);
	//cprintf("%s[%d] %d\n", __FUNCTION__, __LINE__, apCfgGetIntValue(IPCAM_AUDIO_ENABLE_TOK));

	audio_settings->enable = ipcam_settings->audioEnable;
	if (audio_settings->enable)
	{
		if (ipcam_settings->audioMode == 0) // only MIC
		{
			audio_settings->MIC = 1;
			audio_settings->speaker = 0;
		}
		else if (ipcam_settings->audioMode == 1) // only speaker
		{
			audio_settings->MIC = 0;
			audio_settings->speaker = 1;
		}
		else if (ipcam_settings->audioMode == 2) // both
		{
			audio_settings->MIC = 1;
			audio_settings->speaker = 1;
		}
	}
	else
	{
		audio_settings->MIC = 0;
		audio_settings->speaker = 0;
	}

	audio_settings->inputGain = ipcam_settings->inputGain;
	audio_settings->outputVol = ipcam_settings->outputVolume;
#if !IPCAM_AUDIO_CODEC_SUPPORT_AAC
	if(ipcam_settings->encoding == AUDIO_ENCODING_AAC_LC)
	{
		ipcam_settings->encoding = AUDIO_ENCODING_G711U;
		ipcam_settings->bitRate = AUDIO_BIT_RATE_128K;
		ipcam_settings->sampleRate = AUDIO_SAMPLE_RATE_8K;
	}
#endif

	strcpy(audio_settings->encording, audio_encording[ipcam_settings->encoding]);
	//audio_setting.bitRate = audio_bitrate[ipcam_settings.bitRate];
	audio_settings->bitRate = ipcam_settings->bitRate;
	//audio_setting.sampleRate = audio_samplerate[ipcam_settings.sampleRate];
	audio_settings->sampleRate = ipcam_settings->sampleRate;
	audio_settings->alarmLevel = audio_alarmlevel[ipcam_settings->alarmLevel];

	return 0;
}

int get_audio_stream_settings_json_cb(int defaultVal, IPCAM_AUDIO_SETTINGS_T *settings, struct json_object *jobj)
{
	struct json_object *jobj_audio_stream_settings;
	struct json_object *jobj_enable, *jobj_mic, *jobj_speaker, *jobj_inputGain, *jobj_outputVol;
	struct json_object *jobj_encording, *jobj_bitRate, *jobj_sampleRate, *jobj_alarmLevel;

	jobj_audio_stream_settings = json_object_new_object();

	json_object_object_add(jobj_audio_stream_settings, "Enable", json_object_new_boolean(settings->enable));
	json_object_object_add(jobj_audio_stream_settings, "InputEnable", json_object_new_boolean(settings->MIC));
	json_object_object_add(jobj_audio_stream_settings, "OutputEnable", json_object_new_boolean(settings->speaker));
	json_object_object_add(jobj_audio_stream_settings, "InputGain", json_object_new_int(settings->inputGain));
	json_object_object_add(jobj_audio_stream_settings, "OutputVolume", json_object_new_int(settings->outputVol));
	json_object_object_add(jobj_audio_stream_settings, "AlarmLevel", json_object_new_int(settings->alarmLevel));
	json_object_object_add(jobj_audio_stream_settings, "Codec", json_object_new_string(settings->encording));
	json_object_object_add(jobj_audio_stream_settings, "Bitrate", json_object_new_int(settings->bitRate));
	json_object_object_add(jobj_audio_stream_settings, "SampleRate", json_object_new_int(settings->sampleRate));

	if(defaultVal)
		json_object_object_add(jobj, "audio_stream_default", jobj_audio_stream_settings);
	else
		json_object_object_add(jobj, "audio_stream", jobj_audio_stream_settings);

	return 0;
}
#endif

int json_get_camera_audio_stream_default_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc audio_stream_default_settings");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	//debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);
	//debug_print("--- JSON DEBUG %s[%d]: stream_idx: %d\n", __FUNCTION__, __LINE__, stream_idx);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_camera_audio_stream_settings(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char json_data[MAX_JSON_REPLY_LEN] = {0};
	char cmd[MAX_JSON_REPLY_LEN] = {0};

	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		addBackslashAndNewline(query_str, json_data);
		//debug_print("--- Jason DEBUG %s[%d] addBackslash: #.%s.#\n", __FUNCTION__, __LINE__, json_data);
		// ipcammgr_cli setfunc advanced JSON {\"bind\": \"1\", \"device_name\": \"yoyozz\"}
		snprintf(cmd, sizeof(cmd), "ipcammgr_cli setfunc audio JSON %s", json_data);
		//debug_print("cmd: .%s.\n", cmd);
		sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);
		//debug_print("json_reply: .%s.\n", json_reply);
	}
	// parse json_reply
	// GEN VALUE ERROR MSG
	//RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "APPLE");

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_camera_privacy_mask(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc privacymask");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	//debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);
	//debug_print("--- JSON DEBUG %s[%d]: stream_idx: %d\n", __FUNCTION__, __LINE__, stream_idx);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_camera_privacy_mask(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char json_data[MAX_JSON_REPLY_LEN] = {0};
	char cmd[MAX_JSON_REPLY_LEN] = {0};

	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		addBackslashAndNewline(query_str, json_data);
		//debug_print("--- Jason DEBUG %s[%d] addBackslash: #.%s.#\n", __FUNCTION__, __LINE__, json_data);
		// ipcammgr_cli setfunc advanced JSON {\"bind\": \"1\", \"device_name\": \"yoyozz\"}
		snprintf(cmd, sizeof(cmd), "ipcammgr_cli setfunc privacymask JSON %s", json_data);
		//debug_print("cmd: .%s.\n", cmd);
		sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);
		debug_print("json_reply: .%s.\n", json_reply);

		if(strcmp(json_reply, "ok") != 0)
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, json_reply);
		}
	}
	else
	{
		RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "no query_str");
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
