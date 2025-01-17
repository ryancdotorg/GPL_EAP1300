#ifndef _SSNN_H
#define _SSNN_H

// ==================================================================================================
//                                        Structure Definition
// ==================================================================================================

typedef struct
{
    // == Model-dependent settings == //
    int target_height;
    int target_width;

    // == General settings == //
    float prob_thresh;
    float nms_thresh;
    int num_threads;
    int max_detection;
}network_config;

typedef struct
{
    short int x_min, y_min;         // top-left corner
    short int x_max, y_max;         // bottom-right corner
    short int class_idx;
    float prob;
}BBox;

typedef struct
{
    network_config *cfg;
    void *net;
    BBox *boxes;                    // Only for detector
    float *probs;                   // Only for classifier
    int num_detection;
}NetworkHandle;

// ==================================================================================================
//                                           Enumerations
// ==================================================================================================

enum E_NETWORK_TYPE
{
    CLASSIFIER,
    DETECTOR
};

// ==================================================================================================
//                                              API
// ==================================================================================================

int Init_Network(NetworkHandle **phandle, network_config *cfg);
void Release_Network(NetworkHandle **phandle);
int Forward_Network(NetworkHandle *handle, unsigned char *data, int height, int width, int color);
int Get_Detection(NetworkHandle *handle, int img_height, int img_width);   // Only for detector
int Get_Prob(NetworkHandle *handle);                                       // Only for classifier
int Change_Model(NetworkHandle *handle, int network_index);
int Get_Num_Classes(NetworkHandle *handle);
int Get_Network_Type(NetworkHandle *handle);
int Get_Size_Alignment(NetworkHandle *handle);

#endif
