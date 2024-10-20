/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include <drv_sensor_common.h>
#include <sensor_i2c_api.h>
#include <drv_sensor.h>

#ifdef __cplusplus
}
#endif

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX335);

#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE_LINEAR CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (4)
//#define SENSOR_MIPI_HDR_MODE (1) //0: Non-HDR mode. 1:Sony DOL mode
//MIPI config end.
//============================================

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3


//#undef SENSOR_DBG
#define SENSOR_DBG 0

///////////////////////////////////////////////////////////////
//          @@@                                                                                       //
//       @   @@      ==  S t a r t * H e r e ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                           //
//         @@@@                                                                                  //
//                                                                                                     //
//      Start Step 1 --  show preview on LCM                                         //
//                                                                                                    �@//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////

#define SENSOR_ISP_TYPE         ISP_EXT                   //ISP_EXT, ISP_SOC
#define F_number  22                                  // CFG, demo module
//#define SENSOR_DATAFMT         CUS_DATAFMT_BAYER        //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE       CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_MIPI_HSYNC_MODE  PACKET_HEADER_EDGE1
#define SENSOR_DATAPREC         CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAMODE     CUS_SEN_10TO12_9098     //CFG
#define SENSOR_BAYERID          CUS_BAYER_RG            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_RGBIRID          CUS_RGBIR_NONE
#define SENSOR_ORIT             CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
//#define SENSOR_YCORDER         CUS_SEN_YCODR_YC       //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
#define lane_number 2
#define vc0_hs_mode 3   //0: packet header edge  1: line end edge 2: line start edge 3: packet footer edge
#define long_packet_type_enable 0x00 //UD1~UD8 (user define)

#define Preview_MCLK_SPEED       CUS_CMU_CLK_37P125MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_line_period      7407//9697//30580                  //(36M/37.125M)*30fps=29.091fps(34.375msec), hts=34.375/1125=30556,
//#define Line_per_second         32727
#define vts_30fps                4500//4125                              //for 29.091fps @ MCLK=36MHz
#define Prv_Max_line_number      1944                    //maximum exposure line munber of sensor when preview
#define Preview_WIDTH            2592                    //resolution Width when preview
#define Preview_HEIGHT           1944                    //resolution Height when preview
#define Preview_MAX_FPS          30                     //fastest preview FPS
#define Preview_MIN_FPS          3                      //slowest preview FPS
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR          0x34                   //I2C slave address
#define SENSOR_I2C_SPEED         200000 //300000// 240000                  //I2C speed, 60000~320000

#define SENSOR_I2C_LEGACY        I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT           I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_POS        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_POS        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
//static int  drv_Fnumber = 22;


static int pCus_SetAEGain(ms_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ms_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ms_cus_sensor *handle, u32 fps);
static int pCus_SetOrien(ms_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);

CUS_CAMSENSOR_CAP sensor_cap = {
    .length = sizeof(CUS_CAMSENSOR_CAP),
    .version = 0x0001,
};

typedef struct {
    struct {
        u16 pre_div0;
        u16 div124;
        u16 div_cnt7b;
        u16 sdiv0;
        u16 mipi_div0;
        u16 r_divp;
        u16 sdiv1;
        u16 r_seld5;
        u16 r_sclk_dac;
        u16 sys_sel;
        u16 pdac_sel;
        u16 adac_sel;
        u16 pre_div_sp;
        u16 r_div_sp;
        u16 div_cnt5b;
        u16 sdiv_sp;
        u16 div12_sp;
        u16 mipi_lane_sel;
        u16 div_dac;
    } clk_tree;
    struct {
        bool bVideoMode;
        u16 res_idx;
        //        bool binning;
        //        bool scaling;
        CUS_CAMSENSOR_ORIT  orit;
    } res;
    struct {
        float sclk;
        u32 hts;
        u32 vts;
        u32 ho;
        u32 xinc;
        u32 line_freq;
        u32 us_per_line;
        u32 final_us;
        u32 final_gain;
        u32 back_pv_us;
        u32 fps;
        u32 expo_us;
        u32 expo_lines;
    } expo;

    int sen_init;
    int still_min_fps;
    int video_min_fps;
    bool dirty;
    bool change;
    I2C_ARRAY tVts_reg[3];
    I2C_ARRAY tGain_reg[2];
    I2C_ARRAY tExpo_reg[3];
} imx335_params;
// set sensor ID address and data,

const static I2C_ARRAY Sensor_id_table[] =
{
    {0x3003, 0x00},      // {address of ID, ID },
    {0x3033, 0x00}
};

const static I2C_ARRAY Sensor_init_table[] =
{
#if 0
    {0x3002,0x00},   //Master mode stop
    {0xffff,0x14},   //delay
    {0x3000,0x01},   // standby
    {0xffff,0x14},   //delay

  //{0x3000,    0x01},
  //{0x3001,    0x00},
  //{0x3002,    0x01},
  //{0x3003,    0x00},
    {0x300C,0x42},  //BCWAIT_TIME
    {0x300D,0x2E},  //CPWAIT_TIME
    {0x3018,0x00},  //WINMODE
    {0x302C,0x30},  //HTRIMMING_START
    {0x302D,0x00},  //HTRIMMING_START
    {0x302E,0x38},  //HNUM
    {0x302F,0x0A},  //HNUM
    {0x3030,0x1D},  //VMAX
    {0x3031,0x10},  //VMAX
    {0x3032,0x00},  //VMAX
    {0x3034,0xD0},  //HMAX
    {0x3035,0x02},  //HMAX
    {0x304C,0x14},  //OPB_SIZE_V
    {0x304E,0x00},  //HREVERSE
    {0x304F,0x00},  //VREVERSE
    {0x3050,0x00},  //ADBIT
    {0x3056,0xAC},  //Y_OUT_SIZE
    {0x3057,0x07},  //Y_OUT_SIZE
    {0x3058,0x09},  //SHR0
    {0x3059,0x00},  //SHR0
    {0x305A,0x00},  //SHR0
    {0x3072,0x28},  //AREA2_WIDTH_1
    {0x3073,0x00},  //AREA2_WIDTH_1
    {0x3074,0xB0},  //AREA3_ST_ADR_1
    {0x3075,0x00},  //AREA3_ST_ADR_1
    {0x3076,0x58},  //AREA3_WIDTH_1
    {0x3077,0x0F},  //AREA3_WIDTH_1
    {0x3078,0x01},  //Reverse
    {0x3079,0x02},  //Reverse
    {0x307A,0xFF},  //Reverse
    {0x307B,0x02},  //Reverse
    {0x307C,0x00},  //Reverse
    {0x307D,0x00},  //Reverse
    {0x307E,0x00},  //Reverse
    {0x307F,0x00},  //Reverse
    {0x3080,0x01},  //Reverse
    {0x3081,0x02},  //0xFE:Reverse  0x02:Normal
    {0x3082,0xFF},  //Reverse
    {0x3083,0x02},  //0xFE:Reverse  0x02:Normal
    {0x3084,0x00},  //Reverse
    {0x3085,0x00},  //Reverse
    {0x3086,0x00},  //Reverse
    {0x3087,0x00},  //Reverse
    {0x30A4,0x33},  //Reverse
    {0x30A8,0x10},  //Reverse
    {0x30A9,0x04},  //Reverse
    {0x30AC,0x00},  //Reverse
    {0x30AD,0x00},  //Reverse
    {0x30B0,0x10},  //Reverse
    {0x30B1,0x08},  //Reverse
    {0x30B4,0x00},  //Reverse
    {0x30B5,0x00},  //Reverse
    {0x30B6,0x00},  //0xFA:Reverse  0x00:Normal
    {0x30B7,0x00},  //0x01:Reverse  0x00:Normal
    {0x3112,0x08},  //Reverse
    {0x3113,0x00},  //Reverse
    {0x3116,0x08},  //0x02:Reverse  0x08:Normal
    {0x3117,0x00},  //Reverse
    {0x314C,0x08},  //INCKSEL1
    {0x314D,0x01},  //INCKSEL1
    {0x315A,0x06},  //INCKSEL2
    {0x3168,0x8F},  //INCKSEL3
    {0x316A,0x7E},  //INCKSEL4
    {0x3199,0x00},  //HADD/VADD
    {0x319D,0x00},  //MDBIT 0:10Bit  1:12Bit
    {0x319E,0x02},  //SYS_MODE
    {0x31A0,0x2A},  //XH&VSOUTSEL
    {0x3300,0x00},  //TCYCLE
    {0x3302,0x32},  //BLKLEVEL
    {0x341C,0xFF},  //ADBIT1 10Bit:0x01FF  12Bit:0x0047
    {0x341D,0x01},  //ADBIT1 10Bit:0x01FF  12Bit:0x0047
    {0x3A00,0x01},  //for CUE 0x00:HS 0x01: LP-11
    {0x3A01,0x01},  //LANEMODE 0x03:4Lane  0x01:2Lane
    {0x3A18,0x7F},  //TCLKPOST
    {0x3A19,0x00},  //TCLKPOST
    {0x3A1A,0x37},  //TCLKPREPARE
    {0x3A1B,0x00},  //TCLKPREPARE
    {0x3A1C,0x37},  //TCLKTRAIL
    {0x3A1D,0x00},  //TCLKTRAIL
    {0x3A1E,0xF7},  //TCLKZERO
    {0x3A1F,0x00},  //TCLKZERO
    {0x3A20,0x3F},  //THSPREPARE
    {0x3A21,0x00},  //THSPREPARE
    {0x3A22,0x6F},  //THSZERO
    {0x3A23,0x00},  //THSZERO
    {0x3A24,0x3F},  //THSTRAIL
    {0x3A25,0x00},  //THSTRAIL
    {0x3A26,0x5F},  //THSEXIT
    {0x3A27,0x00},  //THSEXIT
    {0x3A28,0x2F},  //TLPX
    {0x3A29,0x00},  //TLPX

    {0x3288,0x21},
    {0x328A,0x02},
    {0x3414,0x05},
    {0x3416,0x18},
    {0x3648,0x01},
    {0x364A,0x04},
    {0x364C,0x04},
    {0x3678,0x01},
    {0x367C,0x31},
    {0x367E,0x31},
    {0x3706,0x10},
    {0x3708,0x03},
    {0x3714,0x02},
    {0x3715,0x02},
    {0x3716,0x01},
    {0x3717,0x03},
    {0x371C,0x3D},
    {0x371D,0x3F},
    {0x372C,0x00},
    {0x372D,0x00},
    {0x372E,0x46},
    {0x372F,0x00},
    {0x3730,0x89},
    {0x3731,0x00},
    {0x3732,0x08},
    {0x3733,0x01},
    {0x3734,0xFE},
    {0x3735,0x05},
    {0x3740,0x02},
    {0x375D,0x00},
    {0x375E,0x00},
    {0x375F,0x11},
    {0x3760,0x01},
    {0x3768,0x1B},
    {0x3769,0x1B},
    {0x376A,0x1B},
    {0x376B,0x1B},
    {0x376C,0x1A},
    {0x376D,0x17},
    {0x376E,0x0F},
    {0x3776,0x00},
    {0x3777,0x00},
    {0x3778,0x46},
    {0x3779,0x00},
    {0x377A,0x89},
    {0x377B,0x00},
    {0x377C,0x08},
    {0x377D,0x01},
    {0x377E,0x23},
    {0x377F,0x02},
    {0x3780,0xD9},
    {0x3781,0x03},
    {0x3782,0xF5},
    {0x3783,0x06},
    {0x3784,0xA5},
    {0x3788,0x0F},
    {0x378A,0xD9},
    {0x378B,0x03},
    {0x378C,0xEB},
    {0x378D,0x05},
    {0x378E,0x87},
    {0x378F,0x06},
    {0x3790,0xF5},
    {0x3792,0x43},
    {0x3794,0x7A},
    {0x3796,0xA1},

    {0x3000,0x00},   // operating
    {0xffff,0x14},

#endif

    {0x3000,0x01},
    {0x3001,0x00},
    {0x3002,0x01},
    {0x3003,0x00},

    {0x300c,0x5b},
    {0x300d,0x40},
    {0x3018,0x00},
    {0x302c,0x30},
    {0x302d,0x00},
    {0x302e,0x38},
    {0x302f,0x0a},
    {0x3030,0x94},
    {0x3031,0x11},
    {0x3032,0x00},
    {0x3033,0x00},
    {0x3034,0x26},
    {0x3035,0x02},
    {0x304c,0x14},
    {0x304e,0x01},
    {0x304f,0x01},
    {0x3050,0x00},
    {0x3056,0xac},
    {0x3057,0x07},
    {0x3058,0x09},
    {0x3059,0x00},
    {0x305a,0x00},
    {0x305c,0x12},
    {0x305d,0x00},
    {0x305e,0x00},
    {0x3060,0xe8},
    {0x3061,0x00},
    {0x3062,0x00},
    {0x3068,0xce},
    {0x3069,0x00},
    {0x306a,0x00},
    {0x306c,0x68},
    {0x306d,0x06},
    {0x306e,0x00},
    {0x3072,0x28},
    {0x3073,0x00},
    {0x3074,0xb0},
    {0x3075,0x00},
    {0x3076,0x58},
    {0x3077,0x0f},
    {0x30c6,0x00},
    {0x30c7,0x00},
    {0x30ce,0x00},
    {0x30cf,0x00},
    {0x30d8,0x4c},
    {0x30d9,0x10},
    {0x30e8,0x00},
    {0x30e9,0x00},
    {0x30ea,0x00},
    {0x30eb,0x00},
    {0x30ec,0x00},
    {0x30ed,0x00},
    {0x314c,0x80},
    {0x314d,0x00},
    {0x315a,0x06},
    {0x3168,0x68},
    {0x316a,0x7e},
    {0x3199,0x00},
    {0x319d,0x00},
    {0x319e,0x03},
    {0x319f,0x01},
    {0x31a0,0x2a},
    {0x31a1,0x0f},
    {0x31d4,0x00},
    {0x31d5,0x00},
    {0x31d7,0x00},
    {0x3200,0x01},
    {0x3288,0x21},
    {0x328a,0x02},
    {0x3300,0x00},
    {0x3302,0x32},
    {0x3303,0x00},
    {0x3414,0x05},
    {0x3416,0x18},
    {0x341c,0xff},//0x47--12-bit   0xff--10bit
    {0x341d,0x01},//
    {0x3648,0x01},
    {0x364a,0x04},
    {0x364c,0x04},
    {0x3678,0x01},
    {0x367c,0x31},
    {0x367e,0x31},
    {0x3706,0x10},
    {0x3708,0x03},
    {0x3714,0x02},
    {0x3715,0x02},
    {0x3716,0x01},
    {0x3717,0x03},
    {0x371c,0x3d},
    {0x371d,0x3f},
    {0x372c,0x00},
    {0x372d,0x00},
    {0x372e,0x46},
    {0x372f,0x00},
    {0x3730,0x89},
    {0x3731,0x00},
    {0x3732,0x08},
    {0x3733,0x01},
    {0x3734,0xfe},
    {0x3735,0x05},
    {0x3740,0x02},
    {0x375d,0x00},
    {0x375e,0x00},
    {0x375f,0x11},
    {0x3760,0x01},
    {0x3768,0x1b},
    {0x3769,0x1b},
    {0x376a,0x1b},
    {0x376b,0x1b},
    {0x376c,0x1a},
    {0x376d,0x17},
    {0x376e,0x0f},
    {0x3776,0x00},
    {0x3777,0x00},
    {0x3778,0x46},
    {0x3779,0x00},
    {0x377a,0x89},
    {0x377b,0x00},
    {0x377c,0x08},
    {0x377d,0x01},
    {0x377e,0x23},
    {0x377f,0x02},
    {0x3780,0xd9},
    {0x3781,0x03},
    {0x3782,0xf5},
    {0x3783,0x06},
    {0x3784,0xa5},
    {0x3788,0x0f},
    {0x378a,0xd9},
    {0x378b,0x03},
    {0x378c,0xeb},
    {0x378d,0x05},
    {0x378e,0x87},
    {0x378f,0x06},
    {0x3790,0xf5},
    {0x3792,0x43},
    {0x3794,0x7a},
    {0x3796,0xa1},
    {0x37b0,0x36},
    {0x3a01,0x03},
    {0x3a04,0x48},
    {0x3a05,0x09},
    {0x3a18,0x67},
    {0x3a19,0x00},
    {0x3a1a,0x27},
    {0x3a1b,0x00},
    {0x3a1c,0x27},
    {0x3a1d,0x00},
    {0x3a1e,0xb7},
    {0x3a1f,0x00},
    {0x3a20,0x2f},
    {0x3a21,0x00},
    {0x3a22,0x4f},
    {0x3a23,0x00},
    {0x3a24,0x2f},
    {0x3a25,0x00},
    {0x3a26,0x47},
    {0x3a27,0x00},
    {0x3a28,0x27},
    {0x3a29,0x00},

    {0x3000,0x00},
    {0x3002,0x00},
};

static I2C_ARRAY TriggerStartTbl[] = {
{0x3002,0x00},//Master mode start
};

static I2C_ARRAY PatternTbl[] = {
{0x308c,0x20}, //colorbar pattern , bit 0 to enable
};

I2C_ARRAY Current_Mirror_Flip_Tbl[] =
{
  {0x304e, 0x00},     //M0F0
  {0x304f, 0x00},
  {0x3081, 0x02},
  {0x3083, 0x02},
  {0x30b6, 0x00},
  {0x30b7, 0x00},
  {0x3016, 0x08},
};

/////////////////////////////////////////////////////////////////
//       @@@@@@                                                                                    //
//                 @@                                                                                    //
//             @@@                                                                                      //
//       @       @@                                                                                    //
//         @@@@                                                                                        //
//                                                                                                          //
//      Step 3 --  complete camera features                                              //
//                                                                                                         //
//                                                                                                         //
//  camera set EV, MWB, orientation, contrast, sharpness                          //
//   , saturation, and Denoise can work correctly.                                     //
//                                                                                                          //
/////////////////////////////////////////////////////////////////


const static I2C_ARRAY mirr_flip_table[] =
{
    {0x304e, 0x00},     //M0F0
    {0x304f, 0x00},     //M0F0
    {0x3081, 0x02},
    {0x3083, 0x02},
    {0x30b6, 0x00},
    {0x30b7, 0x00},
    {0x3016, 0x08},

    {0x304e, 0x01},     //M1F0
    {0x304f, 0x00},     //M1F0
    {0x3081, 0x02},
    {0x3083, 0x02},
    {0x30b6, 0x00},
    {0x30b7, 0x00},
    {0x3016, 0x08},

    {0x304e, 0x00},     //M0F1
    {0x304f, 0x01},     //M0F1
    {0x3081, 0xfe},
    {0x3083, 0xfe},
    {0x30b6, 0xfa},
    {0x30b7, 0x01},
    {0x3016, 0x02},

    {0x304e, 0x01},     //M1F1
    {0x304f, 0x01},     //M1F1
    {0x3081, 0xfe},
    {0x3083, 0xfe},
    {0x30b6, 0xfa},
    {0x30b7, 0x01},
    {0x3016, 0x02},
};

typedef struct {
    short reg;
    char startbit;
    char stopbit;
} COLLECT_REG_SET;



const static I2C_ARRAY gain_reg[] = {
    {0x30E8, 0x00},// bit0-7 low
    {0x30E9, 0x00},// bit0-2(8-10)

};

static int g_sensor_ae_min_gain = 1024;
static CUS_GAIN_GAP_ARRAY gain_gap_compensate[16] = {  //compensate  gain gap
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0}
};

const static I2C_ARRAY expo_reg[] = {
   {0x305a, 0x00},  // bit0-3(16-18)
   {0x3059, 0x00},  // bit0-7(8-15)
   {0x3058, 0x09},  // bit0-7
};

const static I2C_ARRAY vts_reg[] = {
   {0x3032, 0x00},  // bit0-3(16-18)
   {0x3031, 0x11},  // bit0-7(8-15)
   {0x3030, 0x94},  // bit0-7
};

#if 0
static CUS_INT_TASK_ORDER def_order = {
        .RunLength = 9,
        .Orders = {
                CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AE|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_AWB|CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
                CUS_INT_TASK_VDOS|CUS_INT_TASK_AF,
        },
};
#endif

/////////// function definition ///////////////////
#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) LOGD(args)
//#define SENSOR_DMSG(args...) LOGE(args)
#define SENSOR_DMSG(args...) printf(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME imx335


#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,handle->i2c_cfg,_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,handle->i2c_cfg,_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, handle->i2c_cfg,(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, handle->i2c_cfg,(_reg),(_len)))

static int cus_camsensor_release_handle(ms_cus_sensor *handle);

/////////////////// sensor hardware dependent //////////////
#if 0
static int ISP_config_io(ms_cus_sensor *handle) {
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;

    SENSOR_DMSG("[%s]", __FUNCTION__);

    sensor_if->HsyncPol(handle, handle->HSYNC_POLARITY);
    sensor_if->VsyncPol(handle, handle->VSYNC_POLARITY);
    sensor_if->ClkPol(handle, handle->PCLK_POLARITY);
    sensor_if->BayerFmt(handle, handle->bayer_id);
    sensor_if->DataBus(handle, handle->sif_bus);

    sensor_if->DataPrecision(handle, handle->data_prec);
    sensor_if->FmtConv(handle,  handle->data_mode);
    return SUCCESS;
}
#endif
static int pCus_poweron(ms_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->MCLK(idx, 1, handle->mclk);

    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, handle->reset_POLARITY);
    SENSOR_USLEEP(500);
    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, handle->pwdn_POLARITY);
    SENSOR_USLEEP(500);

    // power -> high, reset -> high
    SENSOR_DMSG("[%s] power high\n", __FUNCTION__);
    sensor_if->PowerOff(idx, !handle->pwdn_POLARITY);
    SENSOR_USLEEP(500);
    SENSOR_DMSG("[%s] reset high\n", __FUNCTION__);
    sensor_if->Reset(idx, !handle->reset_POLARITY);
    SENSOR_USLEEP(1000);

    //sensor_if->Set3ATaskOrder(handle, def_order);
    // pure power on
    //ISP_config_io(handle);
    //sensor_if->PowerOff(idx, !handle->pwdn_POLARITY);
    //SENSOR_MSLEEP(5);
    //handle->i2c_bus->i2c_open(handle->i2c_bus,&handle->i2c_cfg);

    return SUCCESS;
}

static int pCus_poweroff(ms_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, handle->reset_POLARITY);
    SENSOR_USLEEP(500);

    SENSOR_DMSG("[%s] power low\n", __FUNCTION__);
    sensor_if->PowerOff(idx, handle->pwdn_POLARITY);
    SENSOR_USLEEP(500);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    sensor_if->MCLK(idx, 0, handle->mclk);

    handle->orient = SENSOR_ORIT;

    return SUCCESS;
}

/////////////////// image function /////////////////////////
/* static int pCus_CheckSonyProductID(ms_cus_sensor *handle)
{
    u16 sen_data;

    // Read Product ID
    if (SensorReg_Read(0x31DC, &sen_data)) {
        return FAIL;
    }

    if ((sen_data & 0x0007) != 0x1) {
        SENSOR_EMSG("[***ERROR***]Check Product ID Fail: 0x%x\n", sen_data);
        return FAIL;
    }

    return SUCCESS;
} */

//Get and check sensor ID
//if i2c error or sensor id does not match then return FAIL
static int pCus_GetSensorID(ms_cus_sensor *handle, u32 *id)
{
    int i,n;
    int table_length= ARRAY_SIZE(Sensor_id_table);
    I2C_ARRAY id_from_sensor[ARRAY_SIZE(Sensor_id_table)];

    for(n=0;n<table_length;++n)
    {
      id_from_sensor[n].reg = Sensor_id_table[n].reg;
      id_from_sensor[n].data = 0;
    }

    *id =0;
    if(table_length>8) table_length=8;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    for(n=0;n<4;++n) //retry , until I2C success
    {
      if(n>2) return FAIL;

      if( SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == SUCCESS) //read sensor ID from I2C
        break;
      else
        continue;
    }

    //convert sensor id to u32 format
    for(i=0;i<table_length;++i)
    {
      if( id_from_sensor[i].data != Sensor_id_table[i].data )
        return FAIL;
      *id = id_from_sensor[i].data;
    }

    SENSOR_DMSG("[%s]Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}

static int imx335_SetPatternMode(ms_cus_sensor *handle,u32 mode)
{
  /*if(mode > 10) mode = 0;
  if(mode)
  {
    SensorReg_Write(0x3148,0x10);
    SensorReg_Write(0x3280,0x00);
    SensorReg_Write(0x329c,0x01);
    SensorReg_Write(0x329e,(mode-1));
  }
  else
  {
    SensorReg_Write(0x3148,0x00);
    SensorReg_Write(0x3280,0x01);
    SensorReg_Write(0x329c,0x00);
    SensorReg_Write(0x329e,0x00);
  }*/
  return SUCCESS;
}
//int g_fps = 0;
//module_param(g_fps, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

static int pCus_init(ms_cus_sensor *handle)
{
    int i,cnt=0;
    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

/*     if (pCus_CheckSonyProductID(handle)) {
        return FAIL;
    } */

    for(i=0;i< ARRAY_SIZE(Sensor_init_table);i++)
    {
        if(Sensor_init_table[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table[i].reg,Sensor_init_table[i].data) != SUCCESS)
            {
                cnt++;
                //printf("Sensor_init_table -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //printf("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                SENSOR_MSLEEP(10);
            }
        }
    }

    for(i=0;i< ARRAY_SIZE(PatternTbl);i++)
    {
        if(SensorReg_Write(PatternTbl[i].reg,PatternTbl[i].data) != SUCCESS)
        {
            //MSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }

    for(i=0;i< ARRAY_SIZE(TriggerStartTbl);i++)
    {
        if(SensorReg_Write(TriggerStartTbl[i].reg,TriggerStartTbl[i].data) != SUCCESS)
        {
            //MSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
            return FAIL;
        }
    }

    pCus_SetOrien(handle, handle->orient);

    pCus_SetAEGain(handle,2048);
    pCus_SetAEUSecs(handle, 25000);
    //if(g_fps)
    //{
    //    pCus_SetFPS(handle, g_fps*1000);
    //}
    //MSG("[%s:%d]Sensor init success!!\n", __FUNCTION__, __LINE__);
    return SUCCESS;
}
/*
int pCus_release(ms_cus_sensor *handle)
{
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    sensor_if->PCLK(NULL,CUS_PCLK_OFF);
    return SUCCESS;
}
*/

static int pCus_GetVideoResNum( ms_cus_sensor *handle, u32 *ulres_num)
{
    *ulres_num = handle->video_res_supported.num_res;
    return SUCCESS;
}

static int pCus_GetVideoRes(ms_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[res_idx];

    return SUCCESS;
}

static int pCus_GetCurVideoRes(ms_cus_sensor *handle, u32 *cur_idx, cus_camsensor_res **res)
{
    u32 num_res = handle->video_res_supported.num_res;

    *cur_idx = handle->video_res_supported.ulcur_res;

    if (*cur_idx >= num_res) {
        return FAIL;
    }

    *res = &handle->video_res_supported.res[*cur_idx];

    return SUCCESS;
}

static int pCus_SetVideoRes(ms_cus_sensor *handle, u32 res_idx)
{
    u32 num_res = handle->video_res_supported.num_res;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            handle->pCus_sensor_init = pCus_init;
            break;

        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ms_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit) {
    short Horiz_Inv = 0;
    short Verti_Inv = 0;
    short Orien_Mode = 0;
    SensorReg_Read(0x304e, &Horiz_Inv);
    SensorReg_Read(0x304f, &Verti_Inv);
    Horiz_Inv &= 0x01;
    Verti_Inv &= 0X01;
    Orien_Mode = Horiz_Inv |(Verti_Inv << 2);
    switch(Orien_Mode)
    {
        case 0x00:
            *orit = CUS_ORIT_M0F0;
        break;
        case 0x01:
            *orit = CUS_ORIT_M1F0;
        break;
        case 0x02:
            *orit = CUS_ORIT_M0F1;
        break;
        case 0x03:
            *orit = CUS_ORIT_M1F1;
        break;
    }
    return SUCCESS;
}

static int pCus_SetOrien(ms_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    int table_length = ARRAY_SIZE(mirr_flip_table);
    int seg_length=table_length/4;
    int i,j;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    switch(orit)
    {
        case CUS_ORIT_M0F0:
            for(i=0,j=0;i<seg_length;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                handle->orient = CUS_ORIT_M0F0;
            }
            break;

        case CUS_ORIT_M1F0:
            for(i=seg_length,j=0;i<seg_length*2;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                handle->orient = CUS_ORIT_M1F0;
            }
            break;

        case CUS_ORIT_M0F1:
            for(i=seg_length*2,j=0;i<seg_length*3;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                handle->orient = CUS_ORIT_M0F1;
            }
            break;

        case CUS_ORIT_M1F1:
            for(i=seg_length*3,j=0;i<seg_length*4;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                handle->orient = CUS_ORIT_M1F1;
            }
            break;

        default :
            for(i=0,j=0;i<seg_length;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                handle->orient = CUS_ORIT_M0F0;
            }
            break;
    }
    return SUCCESS;
}

static int pCus_GetFPS(ms_cus_sensor *handle)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    //SENSOR_DMSG("[%s]", __FUNCTION__);

    return  params->expo.fps;
}
static int pCus_SetFPS(ms_cus_sensor *handle, u32 fps)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.vts=  (vts_30fps*30000 + fps * 500 )/ (fps * 1000);
        params->tVts_reg[0].data = (params->expo.vts>> 16) & 0x0003;
        params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
        params->dirty = true;
        return SUCCESS;
    }else if((fps>=(min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.vts=  (vts_30fps*30000 + (fps>>1))/fps;
        params->tVts_reg[0].data = (params->expo.vts>> 16) & 0x0003;
        params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
        params->dirty = true;
        return SUCCESS;
    }else{
        //params->expo.vts=vts_30fps;
        //params->expo.fps=25;
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }
}
#if 0
static int pCus_GetSensorCap(ms_cus_sensor *handle, CUS_CAMSENSOR_CAP *cap) {
    if (cap)
        memcpy(cap, &sensor_cap, sizeof(CUS_CAMSENSOR_CAP));
    else     return FAIL;
    return SUCCESS;
}
#endif

///////////////////////////////////////////////////////////////////////
// auto exposure
///////////////////////////////////////////////////////////////////////
// unit: micro seconds
//AE status notification
static int pCus_AEStatusNotify(ms_cus_sensor *handle, CUS_CAMSENSOR_AE_STATUS_NOTIFY status){
    imx335_params *params = (imx335_params *)handle->private_data;
    //ISensorIfAPI2 *sensor_if1 = handle->sensor_if_api2;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             //SensorReg_Write(0x3001,0);
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty)
            {
/*
                if(params->change){

                    // sensor_if1->SetSkipFrame(handle,3);
                     params->change = false;

                    }
*/
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
               // printf("0x3009=0x%x,0x3014=0x%x,0x3016=0x%x,0x3020=0x%x,0x3021=0x%x\n", params->tGain_reg[1].data,params->tGain_reg[0].data,params->tGain_reg[2].data,expo_reg[2].data,expo_reg[1].data);
                params->dirty = false;

            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ms_cus_sensor *handle, u32 *us) {
    //int rc;
    u32 lines = 0;
    imx335_params *params = (imx335_params *)handle->private_data;
    //int rc = SensorRegArrayR((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));

    lines |= (u32)(params->tExpo_reg[0].data&0xff)<<16;
    lines |= (u32)(params->tExpo_reg[1].data&0xff)<<8;
    lines |= (u32)(params->tExpo_reg[2].data&0xff)<<0;

    *us = (lines*Preview_line_period)/1000;
    SENSOR_DMSG("[%s] sensor expo lines/us %u,%u us\n", __FUNCTION__, lines, *us);
    //return rc;
    return SUCCESS;
}

static int pCus_SetAEUSecs(ms_cus_sensor *handle, u32 us) {
    u32 lines = 0, vts = 0,activeline = 0;
    imx335_params *params = (imx335_params *)handle->private_data;
    lines = (1000 * us) / Preview_line_period;
    if(lines < 9) lines = 9;
    params->expo.expo_lines = lines;

    if (lines >params->expo.vts-1) {
        vts = lines +1;
    }
    else
      vts=params->expo.vts;

   // lines=us/Preview_line_period;
    SENSOR_DMSG("[%s] us %u, lines %u, vts %u\n", __FUNCTION__,
                us,
                lines,
                params->expo.vts
                );
    activeline=vts-lines;
    if(activeline < 9) activeline = 9;
    params->tExpo_reg[0].data = (activeline>>16) & 0x000f;
    params->tExpo_reg[1].data = (activeline>>8) & 0x00ff;
    params->tExpo_reg[2].data = (activeline>>0) & 0x00ff;


    params->tVts_reg[0].data = (vts >> 16) & 0x000f;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;
    //SensorReg_Write(0x3001,1);
    //SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
    //SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
    //SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
    params->dirty = true;
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain(ms_cus_sensor *handle, u32* gain) {
    //int rc = SensorRegArrayR((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
    unsigned short temp_gain;
    imx335_params *params = (imx335_params *)handle->private_data;
  //  *gain=params->expo.final_gain;
    temp_gain=params->tGain_reg[0].data;

    *gain=(u32)(10^((temp_gain*3)/200))*1024;
    if (params->tGain_reg[1].data & 0x10)
       *gain = (*gain) * 2;

    SENSOR_DMSG("[%s] get gain/reg (1024=1X)= %u/0x%x\n", __FUNCTION__, *gain,params->tGain_reg[0].data);
    //return rc;
    return SUCCESS;
}

static int pCus_SetAEGain_cal(ms_cus_sensor *handle, u32 gain) {
    imx335_params *params = (imx335_params *)handle->private_data;
    //double gain_double;
    u64 gain_double;
    params->expo.final_gain = gain;

    if(gain<1024)
       gain=1024;
    else if(gain>=3980*1024)
        gain=3980*1024;

    gain_double = 20*(intlog10(gain)-intlog10(1024));
    params->tGain_reg[0].data=(u16)(((gain_double*10)>> 24)/3);
    //gain_double = 20*log10((double)gain/1024);
    //params->tGain_reg[0].data=(u16)((gain_double*10)/3);

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data);

    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEGain(ms_cus_sensor *handle, u32 gain) {
    //extern DBG_ITEM Dbg_Items[DBG_TAG_MAX];
    imx335_params *params = (imx335_params *)handle->private_data;
    u32 i;//, gain_before=0;
    CUS_GAIN_GAP_ARRAY* Sensor_Gain_Linearity;
    //double gain_double;
    u64 gain_double;
   // u32 times = log2((double)gain/1024.0f)/log(2);
    params->expo.final_gain = gain;
    if(gain<1024)
        gain=1024;
    else if(gain>=3981*1024)
        gain=3981*1024;
    Sensor_Gain_Linearity = gain_gap_compensate;

    for(i = 0; i < sizeof(gain_gap_compensate)/sizeof(CUS_GAIN_GAP_ARRAY); i++){
        //LOGD("GAP:%x %x\r\n",Sensor_Gain_Linearity[i].gain, Sensor_Gain_Linearity[i].offset);

        if (Sensor_Gain_Linearity[i].gain == 0)
            break;
        if((gain>Sensor_Gain_Linearity[i].gain) && (gain < (Sensor_Gain_Linearity[i].gain + Sensor_Gain_Linearity[i].offset))){
              gain=Sensor_Gain_Linearity[i].gain;
              break;
        }
    }

    gain_double = 20*(intlog10(gain)-intlog10(1024));
    params->tGain_reg[0].data=(u16)(((gain_double*10)>> 24)/3) & 0x00ff;
    params->tGain_reg[1].data=(u16)((((gain_double*10)>> 24)/3) >> 8) & 0x0007;

    //SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));

//LOGD("s:%x %x\r\n", params->tGain_reg[0].data, params->tGain_reg[1].data);
    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
    //return CusHW_i2c_array_tx(handle, handle->i2c_cfg, params->tGain_reg, sizeof(gain_reg)/sizeof(CUS_I2C_ARRAY));
   // return SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, sizeof(gain_reg)/sizeof(I2C_ARRAY));
}

static int pCus_GetAEMinMaxUSecs(ms_cus_sensor *handle, u32 *min, u32 *max) {
    *min = 1;
    *max = 1000000/Preview_MIN_FPS;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ms_cus_sensor *handle, u32 *min, u32 *max) {
    *min = handle->sat_mingain;
    *max = 3981*1024;
    return SUCCESS;
}

static int IMX335_GetShutterInfo(struct __ms_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/Preview_MIN_FPS;
    info->min = (Preview_line_period * 9);
    info->step = Preview_line_period;
    return SUCCESS;
}

static int pCus_setCaliData_gain_linearity(ms_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num) {
    u32 i, j;

    for(i=0,j=0;i< num ;i++,j+=2){
        gain_gap_compensate[i].gain=pArray[i].gain;
        gain_gap_compensate[i].offset=pArray[i].offset;
    }
    //LOGD("[%s]%d, %d, %d, %d\n", __FUNCTION__, num, pArray[0].gain, pArray[1].gain, pArray[num-1].offset);

    return SUCCESS;
}

static int cus_camsensor_init_handle(ms_cus_sensor* drv_handle) {
    ms_cus_sensor *handle = drv_handle;
    imx335_params *params;
    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    //private data allocation & init
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
    params = (imx335_params *)handle->private_data;
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));


    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->model_id,"IMX335_MIPI");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC;  //CUS_DATAPRECISION_8;
    handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->orient      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 0; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0;
    handle->video_res_supported.res[0].width = 2592;
    handle->video_res_supported.res[0].height = 1944;
    handle->video_res_supported.res[0].max_fps= 30;
    handle->video_res_supported.res[0].min_fps= 3;
    handle->video_res_supported.res[0].crop_start_x= 0;
    handle->video_res_supported.res[0].crop_start_y= 0;
    handle->video_res_supported.res[0].nOutputWidth= 2592;
    handle->video_res_supported.res[0].nOutputHeight= 1944;
    sprintf(handle->video_res_supported.res[0].strResDesc, "2592x1944@30fps");

    // i2c

    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED;

    //polarity
    /////////////////////////////////////////////////////
    handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->ae_gain_delay       = 1;//0;//1;
    handle->ae_shutter_delay    = 1;//1;//2;

    handle->ae_gain_ctrl_num = 1;
    handle->ae_shutter_ctrl_num = 1;

    ///calibration
    handle->sat_mingain=g_sensor_ae_min_gain;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init    ;
    //handle->pCus_sensor_powerupseq  = pCus_powerupseq   ;
    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    // Normal
    handle->pCus_sensor_GetSensorID       = pCus_GetSensorID   ;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes   ;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes   ;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien      ;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS      ;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS      ;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap;
    handle->pCus_sensor_SetPatternMode = imx335_SetPatternMode;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    //handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;

    handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain;
    handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs;

     //sensor calibration
    handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal;
    handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity;
    handle->pCus_sensor_GetShutterInfo = IMX335_GetShutterInfo;
    params->expo.vts=vts_30fps;
    params->expo.fps = 30;
    params->dirty = false;
    return SUCCESS;
}

int cus_camsensor_release_handle(ms_cus_sensor *handle)
{
    return SUCCESS;
}

SENSOR_DRV_ENTRY_IMPL_END_EX(  IMX335,
                            cus_camsensor_init_handle,
                            NULL,
                            NULL,
                            imx335_params
                         );


