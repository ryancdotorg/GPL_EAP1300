#ifndef _CAMERA_SETTING_H_
#define _CAMERA_SETTING_H_

#if 0
int json_get_camera_camera_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply);
int json_set_camera_camera_settings(ResponseEntry *rep, char *query_str, char *json_reply);

int json_get_camera_camera_default_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply);

int json_get_camera_advanced_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply);
int json_set_camera_advanced_settings(ResponseEntry *rep, char *query_str, char *json_reply);

int json_get_camera_advanced_default_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply);

int json_get_camera_video_streams_capability(ResponseEntry *rep, struct json_object *jobj, char *json_reply);

int json_get_camera_video_stream_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply, int stream_idx);
int json_set_camera_video_stream_settings(ResponseEntry *rep, char *query_str, char *json_reply, int stream_idx);

int json_get_camera_audio_stream_capability(ResponseEntry *rep, struct json_object *jobj, char *json_reply);

int json_get_camera_audio_stream_default_settings(ResponseEntry *rep, struct json_object *jobj, char *json_reply);
#endif
#endif
