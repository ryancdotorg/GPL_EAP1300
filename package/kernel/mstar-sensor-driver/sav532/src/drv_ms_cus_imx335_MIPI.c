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

SENSOR_DRV_ENTRY_IMPL_BEGIN_EX(IMX335_HDR);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE CAM_OS_ARRAY_SIZE
#endif

#define SENSOR_PAD_GROUP_SET CUS_SENSOR_PAD_GROUP_A
#define SENSOR_CHANNEL_NUM (0)
#define SENSOR_CHANNEL_MODE CUS_SENSOR_CHANNEL_MODE_REALTIME_NORMAL
#define SENSOR_CHANNEL_MODE_SONY_DOL CUS_SENSOR_CHANNEL_MODE_RAW_STORE_HDR

#define SENSOR_CHANNEL_imx335_ISP_CALIBRATION_ENABLE (1)

//============================================
//MIPI config begin.
#define SENSOR_MIPI_LANE_NUM (4)
#define SENSOR_MIPI_LANE_NUM_DOL (4)
//#define SENSOR_MIPI_HDR_MODE (0) //0: Non-HDR mode. 1:Sony DOL mode
//MIPI config end.
//============================================

//#define LOGD //(...)

//c11 extern int usleep(u32 usec);
//int usleep(u32 usec);

#define R_GAIN_REG 1
#define G_GAIN_REG 2
#define B_GAIN_REG 3

#undef SENSOR_DBG
#define SENSOR_DBG 0

///////////////////////////////////////////////////////////////
//          @@@                                                                                       //
//       @   @@      ==  S t a r t * H e r e ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                            //
//            @@      ==  S t a r t * H e r e  ==                                           //
//         @@@@                                                                                  //
//                                                                                                     //
//      Start Step 1 --  show preview on LCM                                         //
//                                                                                                    ���?//
//  Fill these #define value and table with correct settings                        //
//      camera can work and show preview on LCM                                 //
//                                                                                                       //
///////////////////////////////////////////////////////////////
#define SENSOR_ISP_TYPE     ISP_EXT                   //ISP_EXT, ISP_SOC
//#define F_number  22                                  // CFG, demo module
//#define SENSOR_DATAFMT      CUS_DATAFMT_BAYER        //CUS_DATAFMT_YUV, CUS_DATAFMT_BAYER
#define SENSOR_IFBUS_TYPE   CUS_SENIF_BUS_MIPI      //CFG //CUS_SENIF_BUS_PARL, CUS_SENIF_BUS_MIPI
#define SENSOR_MIPI_HSYNC_MODE PACKET_HEADER_EDGE1
#define SENSOR_MIPI_HSYNC_MODE_HDR_DOL PACKET_FOOTER_EDGE
#define SENSOR_DATAPREC     CUS_DATAPRECISION_10    //CFG //CUS_DATAPRECISION_8, CUS_DATAPRECISION_10
#define SENSOR_DATAPREC_DOL     CUS_DATAPRECISION_10
#define SENSOR_DATAMODE     CUS_SEN_10TO12_9098     //CFG
#define SENSOR_BAYERID      CUS_BAYER_RG //CUS_BAYER_GR            //CFG //CUS_BAYER_GB, CUS_BAYER_GR, CUS_BAYER_BG, CUS_BAYER_RG
#define SENSOR_BAYERID_HDR_DOL      CUS_BAYER_RG//CUS_BAYER_GR
#define SENSOR_RGBIRID      CUS_RGBIR_NONE
#ifndef SENSOR_ORIT
#define SENSOR_ORIT         CUS_ORIT_M0F0           //CUS_ORIT_M0F0, CUS_ORIT_M1F0, CUS_ORIT_M0F1, CUS_ORIT_M1F1,
#endif
//#define SENSOR_YCORDER      CUS_SEN_YCODR_YC       //CUS_SEN_YCODR_YC, CUS_SEN_YCODR_CY
//#define vc0_hs_mode 3   //0: packet header edge  1: line end edge 2: line start edge 3: packet footer edge
//#define long_packet_type_enable 0x00 //UD1~UD8 (user define)
#define SENSOR_MAX_GAIN     (1412 * 1024)
#define SENSOR_MIN_GAIN      (1 * 1024)
#define SENSOR_GAIN_DELAY_FRAME_COUNT      (1)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT      (2)
#define SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL    (1)

#define Preview_MCLK_SPEED_2lane       CUS_CMU_CLK_27MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_4lane       CUS_CMU_CLK_37P125MHZ        //CFG //CUS_CMU_CLK_12M, CUS_CMU_CLK_16M, CUS_CMU_CLK_24M, CUS_CMU_CLK_27M
#define Preview_MCLK_SPEED_HDR_DOL  CUS_CMU_CLK_24MHZ
u32 Preview_line_period;
u32 vts_30fps;
u32 Preview_MAX_FPS;
u32 Preview_line_period_HDR_DOL;
u32 vts_30fps_HDR_DOL;
//#define Preview_line_period 7407//30580                  //(36M/37.125M)*30fps=29.091fps(34.375msec), hts=34.375/1125=30556,
//#define Preview_line_period_HDR_DOL 7407//14842//30535//30580   10^9 / (1158 * 2) / 29.091   //(36M/37.125M)*30.02fps=29.11fps(34.352msec), hts=34.352/1125=30556,
//#define vts_30fps  4500//1090                              //for 29.091fps @ MCLK=36MHz
//#define vts_30fps_HDR_DOL  5400/1125//1090                              //for 29.091fps @ MCLK=36MHz //VMAX
#define Prv_Max_line_number 1944                    //maximum exposure line munber of sensor when preview
#define Preview_WIDTH            2592                    //resolution Width when preview
#define Preview_HEIGHT           1944                    //resolution Height when preview
#define Preview_HEIGHT_HDR_DOL     1944                    //resolution Height when preview
//#define Preview_MAX_FPS     25                     //fastest preview FPS
#define Preview_MIN_FPS     3                      //slowest preview FPS
#define Preview_CROP_START_X     0                      //CROP_START_X
#define Preview_CROP_START_HDR_X     8                      //CROP_START_X
#define Preview_CROP_START_Y     0                      //CROP_START_Y

#define SENSOR_I2C_ADDR    0x34                   //I2C slave address
#define SENSOR_I2C_SPEED   20000//200000 //300000// 240000                  //I2C speed, 60000~320000
#define SENSOR_I2C_CHANNEL  1                           //I2C Channel
#define SENSOR_I2C_PAD_MODE 2                           //Pad/Mode Number

#define SENSOR_I2C_LEGACY  I2C_NORMAL_MODE     //usally set CUS_I2C_NORMAL_MODE,  if use old OVT I2C protocol=> set CUS_I2C_LEGACY_MODE
#define SENSOR_I2C_FMT     I2C_FMT_A16D8        //CUS_I2C_FMT_A8D8, CUS_I2C_FMT_A8D16, CUS_I2C_FMT_A16D8, CUS_I2C_FMT_A16D16

#define SENSOR_PWDN_POL     CUS_CLK_POL_NEG        // if PWDN pin High can makes sensor in power down, set CUS_CLK_POL_POS
#define SENSOR_RST_POL      CUS_CLK_POL_NEG        // if RESET pin High can makes sensor in reset state, set CUS_CLK_POL_NEG

// VSYNC/HSYNC POL can be found in data sheet timing diagram,
// Notice: the initial setting may contain VSYNC/HSYNC POL inverse settings so that condition is different.

#define SENSOR_VSYNC_POL    CUS_CLK_POL_NEG        // if VSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_HSYNC_POL    CUS_CLK_POL_NEG        // if HSYNC pin High and data bus have data, set CUS_CLK_POL_POS
#define SENSOR_PCLK_POL     CUS_CLK_POL_NEG        // depend on sensor setting, sometimes need to try CUS_CLK_POL_POS or CUS_CLK_POL_NEG
//static int  drv_Fnumber = 22;

#if SENSOR_DBG == 1
//#define SENSOR_DMSG(args...) LOGD(args)
//#define SENSOR_DMSG(args...) LOGE(args)
//#define SENSOR_DMSG(args...) printf(args)
#elif SENSOR_DBG == 0
//#define SENSOR_DMSG(args...)
#endif
#undef SENSOR_NAME
#define SENSOR_NAME IMX335

#if defined (SENSOR_MODULE_VERSION)
#define TO_STR_NATIVE(e) #e
#define TO_STR_PROXY(m, e) m(e)
#define MACRO_TO_STRING(e) TO_STR_PROXY(TO_STR_NATIVE, e)
static char *sensor_module_version = MACRO_TO_STRING(SENSOR_MODULE_VERSION);
module_param(sensor_module_version, charp, S_IRUGO);
#endif

//#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,handle->i2c_cfg,_reg,_data))
//#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,handle->i2c_cfg,_reg,_data))
//#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, handle->i2c_cfg,(_reg),(_len)))
//#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, handle->i2c_cfg,(_reg),(_len)))

static int cus_camsensor_release_handle(ms_cus_sensor *handle);
static int pCus_SetAEGain(ms_cus_sensor *handle, u32 gain);
static int pCus_SetAEUSecs(ms_cus_sensor *handle, u32 us);
static int pCus_SetFPS(ms_cus_sensor *handle, u32 fps);
static int pCus_SetOrien(ms_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit);
//static CUS_MCLK_FREQ UseParaMclk(const char *mclk);

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
    I2C_ARRAY tExpo_shr_dol1_reg[3];
    I2C_ARRAY tExpo_rhs1_reg[3];
    I2C_ARRAY tGain_hdr_dol_lef_reg[2];
    I2C_ARRAY tGain_hdr_dol_sef_reg[2];
} imx335_params;
// set sensor ID address and data,

const static I2C_ARRAY Sensor_id_table[] =
{
    {0x3003, 0x00},      // {address of ID, ID },
    {0x3033, 0x00}
};

const static I2C_ARRAY Sensor_init_table_2lane[] =
{
    {0x3002,0x00},   //Master mode stop
    {0xffff,0x14},   //delay
    {0x3000,0x01},   // standby
    {0xffff,0x14},   //delay

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
};

const static I2C_ARRAY Sensor_init_table_4lane[] =
{
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

const static I2C_ARRAY Sensor_init_table_HDR_DOL_4lane25fps[] =
{
    {0x3000,0x01},
    {0x3001,0x00},
    {0x3002,0x01},
    {0x3003,0x00},

    {0x300c,0x3b},
    {0x300d,0x2a},
    {0x3018,0x00},
    {0x302c,0x30},
    {0x302d,0x00},
    {0x302e,0x38},
    {0x302f,0x0a},
    {0x3030,0x18},
    {0x3031,0x15},
    {0x3032,0x00},
    {0x3033,0x00},
    {0x3034,0x13},
    {0x3035,0x01},
    {0x3048,0x01},
    {0x3049,0x01},
    {0x304a,0x04},
    {0x304b,0x03},
    {0x304c,0x00},
    {0x304e,0x00},
    {0x304f,0x00},
    {0x3050,0x00},
    {0x3056,0x07},
    {0x3057,0x08},
    {0x3058,0x88},
    {0x3059,0x1d},
    {0x305a,0x00},
    {0x305c,0x12},
    {0x305d,0x00},
    {0x305e,0x00},
    {0x3060,0xe8},
    {0x3061,0x00},
    {0x3062,0x00},
    {0x3064,0x09},
    {0x3065,0x00},
    {0x3066,0x00},
    {0x3068,0x22},
    {0x3069,0x01},
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

    {0x3078,0x01},
    {0x3079,0x02},
    {0x307a,0xff},
    {0x307b,0x02},
    {0x307c,0x00},
    {0x307d,0x00},
    {0x307e,0x00},
    {0x307f,0x00},
    {0x3080,0x01},
    {0x3081,0x02},
    {0x3082,0xff},
    {0x3083,0x02},
    {0x3084,0x00},
    {0x3085,0x00},
    {0x3086,0x00},
    {0x3087,0x00},

    {0x30a4,0x33},
    {0x30a8,0x10},
    {0x30a9,0x04},
    {0x30ac,0x00},
    {0x30ad,0x00},
    {0x30b0,0x10},
    {0x30b1,0x08},
    {0x30b4,0x00},
    {0x30b5,0x00},
    {0x30b6,0x00},
    {0x30b7,0x00},

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
    {0x30ee,0x00},
    {0x30ef,0x00},
    {0x3112,0x08},
    {0x3113,0x00},
    {0x3116,0x08},
    {0x3117,0x00},
    {0x314c,0xc6},
    {0x314d,0x00},
    {0x315a,0x02},
    {0x3167,0x01},
    {0x3168,0xa0},
    {0x316a,0x7e},
    {0x3199,0x00},
    {0x319d,0x00},
    {0x319e,0x01},
    {0x319f,0x00},
    {0x31a0,0x2a},
    {0x31a1,0x00},
    {0x31a4,0x00},
    {0x31a5,0x00},
    {0x31a6,0x00},
    {0x31a8,0x00},
    {0x31ac,0x00},
    {0x31ad,0x00},
    {0x31ae,0x00},
    {0x31d4,0x00},
    {0x31d5,0x00},
    {0x31d7,0x01},
    {0x31e4,0x01},
    {0x31e8,0x00},
    {0x31f3,0x01},
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
    {0x3a04,0x90},
    {0x3a05,0x12},
    {0x3a18,0x8f},
    {0x3a19,0x00},
    {0x3a1a,0x4f},
    {0x3a1b,0x00},
    {0x3a1c,0x47},
    {0x3a1d,0x00},
    {0x3a1e,0x37},
    {0x3a1f,0x01},
    {0x3a20,0x4f},
    {0x3a21,0x00},
    {0x3a22,0x87},
    {0x3a23,0x00},
    {0x3a24,0x4f},
    {0x3a25,0x00},
    {0x3a26,0x7f},
    {0x3a27,0x00},
    {0x3a28,0x3f},
    {0x3a29,0x00},

    {0x3000,0x00},
    {0x3002,0x00},
};

const static I2C_ARRAY Sensor_init_table_HDR_DOL_4lane20fps[] =
{
    {0x3000,0x01},
    {0x3001,0x00},
    {0x3002,0x01},
    {0x3003,0x00},

    {0x300c,0x3b},
    {0x300d,0x2a},
    {0x3018,0x00},
    {0x302c,0x30},
    {0x302d,0x00},
    {0x302e,0x38},
    {0x302f,0x0a},
    {0x3030,0x5e},
    {0x3031,0x1a},
    {0x3032,0x00},
    {0x3033,0x00},
    {0x3034,0x13},
    {0x3035,0x01},
    {0x3048,0x01},
    {0x3049,0x01},
    {0x304a,0x04},
    {0x304b,0x03},
    {0x304c,0x00},
    {0x304e,0x00},
    {0x304f,0x00},
    {0x3050,0x00},
    {0x3056,0x07},
    {0x3057,0x08},
    {0x3058,0x14},
    {0x3059,0x28},
    {0x305a,0x00},
    {0x305c,0x12},
    {0x305d,0x00},
    {0x305e,0x00},
    {0x3060,0xe8},
    {0x3061,0x00},
    {0x3062,0x00},
    {0x3064,0x09},
    {0x3065,0x00},
    {0x3066,0x00},
    {0x3068,0x22},
    {0x3069,0x01},
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

    {0x3078,0x01},
    {0x3079,0x02},
    {0x307a,0xff},
    {0x307b,0x02},
    {0x307c,0x00},
    {0x307d,0x00},
    {0x307e,0x00},
    {0x307f,0x00},
    {0x3080,0x01},
    {0x3081,0x02},
    {0x3082,0xff},
    {0x3083,0x02},
    {0x3084,0x00},
    {0x3085,0x00},
    {0x3086,0x00},
    {0x3087,0x00},

    {0x30a4,0x33},
    {0x30a8,0x10},
    {0x30a9,0x04},
    {0x30ac,0x00},
    {0x30ad,0x00},
    {0x30b0,0x10},
    {0x30b1,0x08},
    {0x30b4,0x00},
    {0x30b5,0x00},
    {0x30b6,0x00},
    {0x30b7,0x00},

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
    {0x30ee,0x00},
    {0x30ef,0x00},
    {0x3112,0x08},
    {0x3113,0x00},
    {0x3116,0x08},
    {0x3117,0x00},
    {0x314c,0xc6},
    {0x314d,0x00},
    {0x315a,0x02},
    {0x3167,0x01},
    {0x3168,0xa0},
    {0x316a,0x7e},
    {0x3199,0x00},
    {0x319d,0x00},
    {0x319e,0x01},
    {0x319f,0x00},
    {0x31a0,0x2a},
    {0x31a1,0x00},
    {0x31a4,0x00},
    {0x31a5,0x00},
    {0x31a6,0x00},
    {0x31a8,0x00},
    {0x31ac,0x00},
    {0x31ad,0x00},
    {0x31ae,0x00},
    {0x31d4,0x00},
    {0x31d5,0x00},
    {0x31d7,0x01},
    {0x31e4,0x01},
    {0x31e8,0x00},
    {0x31f3,0x01},
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
    {0x3a04,0x90},
    {0x3a05,0x12},
    {0x3a18,0x8f},
    {0x3a19,0x00},
    {0x3a1a,0x4f},
    {0x3a1b,0x00},
    {0x3a1c,0x47},
    {0x3a1d,0x00},
    {0x3a1e,0x37},
    {0x3a1f,0x01},
    {0x3a20,0x4f},
    {0x3a21,0x00},
    {0x3a22,0x87},
    {0x3a23,0x00},
    {0x3a24,0x4f},
    {0x3a25,0x00},
    {0x3a26,0x7f},
    {0x3a27,0x00},
    {0x3a28,0x3f},
    {0x3a29,0x00},

    {0x3000,0x00},
    {0x3002,0x00},
};

const static I2C_ARRAY TriggerStartTbl[] = {
    {0x3002,0x00},//Master mode start
};

static I2C_ARRAY PatternTbl[] = {
    {0x308c,0x20}, //colorbar pattern , bit 0 to enable
};

const static I2C_ARRAY gain_HDR_DOL_LEF_reg[] =
{
    {0x30E8, 0x00},// bit0-7 low
    {0x30E9, 0x00},// bit0-2(8-10)
};

const static I2C_ARRAY gain_HDR_DOL_SEF1_reg[] =
{
    {0x30EA, 0x00},// bit0-7 low
    {0x30EB, 0x00},// bit0-2(8-10)
};

const static I2C_ARRAY expo_shr_dol1_reg[] =
{ //SEL
   {0x305e, 0x00},  // bit0-3(16-18)
   {0x305d, 0x00},  // bit0-7(8-15)
   {0x305c, 0x12},  // bit0-7
};

const I2C_ARRAY expo_rhs1_reg[] =
{ //SEL
   {0x306a, 0x00},  // bit0-3(16-18)
   {0x3069, 0x00},  // bit0-7(8-15)
   {0x3068, 0xce},  // bit0-7
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

//static int g_sensor_ae_min_gain = 1024;
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

#define SensorReg_Read(_reg,_data)     (handle->i2c_bus->i2c_rx(handle->i2c_bus,handle->i2c_cfg,_reg,_data))
#define SensorReg_Write(_reg,_data)    (handle->i2c_bus->i2c_tx(handle->i2c_bus,handle->i2c_cfg,_reg,_data))
#define SensorRegArrayW(_reg,_len)  (handle->i2c_bus->i2c_array_tx(handle->i2c_bus, handle->i2c_cfg,(_reg),(_len)))
#define SensorRegArrayR(_reg,_len)  (handle->i2c_bus->i2c_array_rx(handle->i2c_bus, handle->i2c_cfg,(_reg),(_len)))

/////////////////// sensor hardware dependent //////////////
#if 0
static int ISP_config_io(ms_cus_sensor *handle) {
#if 0
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;

    SENSOR_DMSG("[%s]", __FUNCTION__);

    sensor_if->HsyncPol(handle, handle->HSYNC_POLARITY);
    sensor_if->VsyncPol(handle, handle->VSYNC_POLARITY);
    sensor_if->ClkPol(handle, handle->PCLK_POLARITY);
    sensor_if->BayerFmt(handle, handle->bayer_id);
    sensor_if->DataBus(handle, handle->sif_bus);

    sensor_if->DataPrecision(handle, handle->data_prec);
    sensor_if->FmtConv(handle,  handle->data_mode);
#endif
    return SUCCESS;
}
#endif

static int pCus_poweron(ms_cus_sensor *handle, u32 idx)
{
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    SENSOR_DMSG("[%s] ", __FUNCTION__);

    //Sensor power on sequence
    sensor_if->PowerOff(idx, !handle->pwdn_POLARITY);
    sensor_if->Reset(idx, !handle->reset_POLARITY);
    sensor_if->SetIOPad(idx, handle->sif_bus, handle->interface_attr.attr_mipi.mipi_lane_num);
    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_216M);
    sensor_if->SetCSI_Lane(idx, handle->interface_attr.attr_mipi.mipi_lane_num, 1);
    sensor_if->SetCSI_LongPacketType(idx, 0, 0x1C00, 0);

    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 1);
    }

    sensor_if->Reset(idx, handle->reset_POLARITY );
    sensor_if->MCLK(idx, 1, handle->mclk);
    SENSOR_UDELAY(20); //TLOW

    sensor_if->PowerOff(idx, handle->pwdn_POLARITY);
    SENSOR_UDELAY(20);
    sensor_if->PowerOff(idx, !handle->pwdn_POLARITY);
    SENSOR_UDELAY(20);

    sensor_if->Reset(idx, !handle->reset_POLARITY );
    SENSOR_UDELAY(20); //TXCE

    return SUCCESS;
}

static int pCus_poweroff(ms_cus_sensor *handle, u32 idx)
{
    // power/reset low
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;

    SENSOR_DMSG("[%s] reset low\n", __FUNCTION__);
    sensor_if->Reset(idx, handle->reset_POLARITY);
    sensor_if->MCLK(idx, 0, handle->mclk);

    sensor_if->SetCSI_Clk(idx, CUS_CSI_CLK_DISABLE);
    if (handle->interface_attr.attr_mipi.mipi_hdr_mode == CUS_HDR_MODE_SONY_DOL) {
        sensor_if->SetCSI_hdr_mode(idx, handle->interface_attr.attr_mipi.mipi_hdr_mode, 0);
    }
    handle->orient = SENSOR_ORIT;

    return SUCCESS;
}

/////////////////// image function /////////////////////////

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

      if(/* SensorRegArrayR((I2C_ARRAY*)id_from_sensor,table_length) == */SUCCESS) //read sensor ID from I2C
          break;
      else
          SENSOR_MSLEEP(1);
    }

    //convert sensor id to u32 format
    for(i=0;i<table_length;++i)
    {
      if( id_from_sensor[i].data != Sensor_id_table[i].data )
        return FAIL;
      *id = id_from_sensor[i].data;
    }


    SENSOR_DMSG("[%s]IMX335 sensor ,Read sensor id, get 0x%x Success\n", __FUNCTION__, (int)*id);
    return SUCCESS;
}

static int imx335_SetPatternMode(ms_cus_sensor *handle,u32 mode)
{
    int i;
  switch(mode)
  {
  case 1:
    PatternTbl[0].data = 0x21; //enable
  break;
  case 0:
    PatternTbl[0].data &= 0xFE; //disable
  break;
  default:
    PatternTbl[0].data &= 0xFE; //disable
  break;
  }

  for(i=0;i< ARRAY_SIZE(PatternTbl);i++)
  {
    if(SensorReg_Write(PatternTbl[i].reg,PatternTbl[i].data) != SUCCESS)
    {
      //MSG("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
      return FAIL;
    }
  }

  return SUCCESS;
}

static int pCus_init_mipi4lane_linear(ms_cus_sensor *handle)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    int i,cnt=0;

    //UartSendTrace("IMX335 Sensor_init_table_4lane\n");
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_4lane);i++)
    {
        if(Sensor_init_table_4lane[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_4lane[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_4lane[i].reg,Sensor_init_table_4lane[i].data) != SUCCESS)
            {
                cnt++;
                //printf("Sensor_init_table_4lane -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //printf("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //usleep(10*1000);
            }
            //SensorReg_Read( Sensor_init_table_4lane[i].reg, &sen_data );
            //UartSendTrace("IMX335 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_4lane[i].reg, Sensor_init_table_4lane[i].data, sen_data);
        }
    }

    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
    pCus_SetOrien(handle, handle->orient);
    return SUCCESS;
}

static int pCus_init_mipi2lane_linear(ms_cus_sensor *handle)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    int i,cnt=0;

    //UartSendTrace("IMX335 Sensor_init_table_2lane\n");
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_2lane);i++)
    {
        if(Sensor_init_table_4lane[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_2lane[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_2lane[i].reg,Sensor_init_table_2lane[i].data) != SUCCESS)
            {
                cnt++;
                //printf("Sensor_init_table_2lane -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //printf("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //usleep(10*1000);
            }
            //SensorReg_Read( Sensor_init_table_2lane[i].reg, &sen_data );
            //UartSendTrace("IMX335 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_2lane[i].reg, Sensor_init_table_2lane[i].data, sen_data);
        }
    }

    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
    pCus_SetOrien(handle, handle->orient);
    return SUCCESS;
}

static int pCus_init_mipi4lane25fps_HDR_DOL(ms_cus_sensor *handle)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt=0;
    //ISensorIfAPI *sensor_if = &(handle->sensor_if_api);
    //    short sen_data;
    //sensor_if->PCLK( CUS_SNR_PCLK_MIPI_TOP );//sensor_if->PCLK(NULL,CUS_PCLK_MIPI_TOP);

    //UartSendTrace("IMX335 Sensor_init_table_HDR_DOL_4lane25fps\n");
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR_DOL_4lane25fps);i++)
    {
        if(Sensor_init_table_HDR_DOL_4lane25fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_HDR_DOL_4lane25fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_HDR_DOL_4lane25fps[i].reg,Sensor_init_table_HDR_DOL_4lane25fps[i].data) != SUCCESS)
            {
                cnt++;
                //UartSendTrace("Sensor_init_table_HDR_DOL_4lane25fps -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //UartSendTrace("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //usleep(10*1000);
            }
            //SensorReg_Read( Sensor_init_table_HDR_DOL_4lane25fps[i].reg, &sen_data );
            //UartSendTrace("IMX335 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_HDR_DOL_4lane25fps[i].reg, Sensor_init_table_HDR_DOL_4lane25fps[i].data, sen_data);
        }
    }

    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;

    //UartSendTrace("[%s:%d]Sensor init success!!\n", __FUNCTION__, __LINE__);
    return SUCCESS;
}

static int pCus_init_mipi4lane20fps_HDR_DOL(ms_cus_sensor *handle)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    //SENSOR_DMSG("\n\n[%s]", __FUNCTION__);
    int i,cnt=0;
    //ISensorIfAPI *sensor_if = &(handle->sensor_if_api);
    //    short sen_data;
    //sensor_if->PCLK( CUS_SNR_PCLK_MIPI_TOP );//sensor_if->PCLK(NULL,CUS_PCLK_MIPI_TOP);

    //UartSendTrace("IMX335 Sensor_init_table_HDR_DOL_4lane20fps\n");
    for(i=0;i< ARRAY_SIZE(Sensor_init_table_HDR_DOL_4lane20fps);i++)
    {
        if(Sensor_init_table_HDR_DOL_4lane20fps[i].reg==0xffff)
        {
            SENSOR_MSLEEP(Sensor_init_table_HDR_DOL_4lane20fps[i].data);
        }
        else
        {
            cnt = 0;
            while(SensorReg_Write(Sensor_init_table_HDR_DOL_4lane20fps[i].reg,Sensor_init_table_HDR_DOL_4lane20fps[i].data) != SUCCESS)
            {
                cnt++;
                //UartSendTrace("Sensor_init_table_HDR_DOL_4lane20fps -> Retry %d...\n",cnt);
                if(cnt>=10)
                {
                    //UartSendTrace("[%s:%d]Sensor init fail!!\n", __FUNCTION__, __LINE__);
                    return FAIL;
                }
                //usleep(10*1000);
            }
            //SensorReg_Read( Sensor_init_table_HDR_DOL_4lane20fps[i].reg, &sen_data );
            //UartSendTrace("IMX335 reg: 0x%x, data: 0x%x, read: 0x%x.\n",Sensor_init_table_HDR_DOL_4lane20fps[i].reg, Sensor_init_table_HDR_DOL_4lane20fps[i].data, sen_data);
        }
    }

    params->tVts_reg[0].data = (params->expo.vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;

    //UartSendTrace("[%s:%d]Sensor init success!!\n", __FUNCTION__, __LINE__);
    return SUCCESS;
}

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
    imx335_params *params = (imx335_params *)handle->private_data;
    u32 num_res = handle->video_res_supported.num_res;

    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            if(lane_num == 2){
                handle->pCus_sensor_init = pCus_init_mipi2lane_linear;
                vts_30fps = 4125;
                Preview_MAX_FPS = 25;
                Preview_line_period  = 9697;
                handle->mclk = CUS_CMU_CLK_27MHZ;
            }
            else if (lane_num == 4){
                handle->pCus_sensor_init = pCus_init_mipi4lane_linear;
                vts_30fps  = 4500;
                Preview_MAX_FPS = 30;
                Preview_line_period  = 7407;
                handle->mclk = CUS_CMU_CLK_37P125MHZ;
            }
            else{
                handle->pCus_sensor_init = pCus_init_mipi4lane_linear;
                vts_30fps  = 4500;
                Preview_MAX_FPS = 30;
                Preview_line_period  = 7407;
                handle->mclk = CUS_CMU_CLK_37P125MHZ;
            }

            params->expo.vts=vts_30fps;
            params->expo.fps=Preview_MAX_FPS;
/*             handle->pCus_sensor_init = pCus_init_mipi4lane_linear;
            params->expo.vts=vts_30fps; */
            break;

        default:
            break;
    }

    return SUCCESS;
}

static int pCus_SetVideoRes_HDR_DOL(ms_cus_sensor *handle, u32 res_idx)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    //ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    u32 num_res = handle->video_res_supported.num_res;
    if (res_idx >= num_res) {
        return FAIL;
    }
    switch (res_idx) {
        case 0:
            handle->video_res_supported.ulcur_res = 0;
            //sensor_if->MCLK(0,1,CUS_CMU_CLK_24MHZ);
            handle->pCus_sensor_init = pCus_init_mipi4lane25fps_HDR_DOL;
            vts_30fps_HDR_DOL = 5400;
            params->expo.vts = vts_30fps_HDR_DOL;
            Preview_MAX_FPS = 25;
            params->expo.fps=Preview_MAX_FPS;
            Preview_line_period_HDR_DOL = 7407;

            break;
        case 1:
            handle->video_res_supported.ulcur_res = 1;
            //sensor_if->MCLK(0,1,CUS_CMU_CLK_24MHZ);
            handle->pCus_sensor_init = pCus_init_mipi4lane20fps_HDR_DOL;
            vts_30fps_HDR_DOL = 6750;
            params->expo.vts = vts_30fps_HDR_DOL;
            Preview_MAX_FPS = 20;
            params->expo.fps=Preview_MAX_FPS;
            Preview_line_period_HDR_DOL = 7407;
            break;
/*             handle->pCus_sensor_init = pCus_init_mipi4lane25fps_HDR_DOL;
            params->expo.vts = vts_30fps_HDR_DOL; */

            break;

        default:
            break;
    }

    return SUCCESS;
}

static int pCus_GetOrien(ms_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
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
                SensorReg_Write(0x304e,0x00);
                SensorReg_Write(0x304f,0x00);
            }
            break;

        case CUS_ORIT_M1F0:
            for(i=seg_length,j=0;i<seg_length*2;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                handle->orient = CUS_ORIT_M1F0;
                SensorReg_Write(0x304e,0x01);
                SensorReg_Write(0x304f,0x00);
            }
            break;

        case CUS_ORIT_M0F1:
            for(i=seg_length*2,j=0;i<seg_length*3;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                handle->orient = CUS_ORIT_M0F1;
                SensorReg_Write(0x304e,0x00);
                SensorReg_Write(0x304f,0x01);
            }
            break;

        case CUS_ORIT_M1F1:
            for(i=seg_length*3,j=0;i<seg_length*4;i++,j++){
                SensorReg_Write(mirr_flip_table[i].reg,mirr_flip_table[i].data);
                handle->orient = CUS_ORIT_M1F1;
                SensorReg_Write(0x304e,0x01);
                SensorReg_Write(0x304e,0x01);
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
    u32 vts = 0;
    imx335_params *params = (imx335_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].min_fps;

    SENSOR_DMSG("\n\n[%s]", __FUNCTION__);

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*(max_fps*1000) + fps * 500 )/ (fps * 1000);
    }else if((fps>=(min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps*(max_fps*1000) + (fps>>1))/fps;
    }else{
        //params->expo.vts=vts_30fps;
        //params->expo.fps=25;
        SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    if(params->expo.expo_lines > params->expo.vts -2){
        vts = params->expo.expo_lines + 8;
    }else{
        vts = params->expo.vts;
    }

    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;
    //SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
    //printf("[%s] params->expo.vts : %d vts : 0x%x, 0x%x, 0x%x fps:%d \r\n",__FUNCTION__,params->expo.vts, params->tVts_reg[0].data,params->tVts_reg[1].data,params->tVts_reg[2].data, params->expo.fps);

    return SUCCESS;
}

static int pCus_SetFPS_HDR_DOL_SEF1(ms_cus_sensor *handle, u32 fps)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR_DOL*(max_fps*1000) + fps * 500 )/ (fps * 1000);
        params->tVts_reg[0].data = (params->expo.vts>> 16) & 0x000f;
        params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
        params->dirty = true; //reg need to update = true;
      return SUCCESS;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR_DOL*(max_fps*1000) + (fps>>1))/fps;
        params->tVts_reg[0].data = (params->expo.vts>> 16) & 0x000f;
        params->tVts_reg[1].data = (params->expo.vts >> 8) & 0x00ff;
        params->tVts_reg[2].data = (params->expo.vts >> 0) & 0x00ff;
        params->dirty = true; //reg need to update = true;
        return SUCCESS;
    }else{
      //params->expo.vts=vts_30fps;
      //params->expo.fps=30;
        //SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
      return FAIL;
    }
}
#if 0
static int pCus_GetSensorCap(ms_cus_sensor *handle, CUS_CAMSENSOR_CAP *cap)
{
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
static int pCus_AEStatusNotify(ms_cus_sensor *handle, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    //ISensorIfAPI2 *sensor_if1 = handle->sensor_if_api2;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             //SensorReg_Write(0x3001,0);
             break;
        case CUS_FRAME_ACTIVE:
            if(params->dirty) {
/*
                if(params->change){

                    // sensor_if1->SetSkipFrame(handle,3);
                     params->change = false;

                    }
*/
                //SensorReg_Write(0x3001,1);
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                //SensorReg_Write(0x3001,0);
               // printf("0x3009=0x%x,0x3014=0x%x,0x3016=0x%x,0x3020=0x%x,0x3021=0x%x\n", params->tGain_reg[1].data,params->tGain_reg[0].data,params->tGain_reg[2].data,params->tExpo_reg[2].data,params->tExpo_reg[1].data);
                params->dirty = false;
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_SEF1(ms_cus_sensor *handle, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
    //imx335_params *params = (imx335_params *)handle->private_data;
    switch(status)
    {
        case CUS_FRAME_INACTIVE:
             //SensorReg_Write(0x3001,0);
             break;
        case CUS_FRAME_ACTIVE:
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs(ms_cus_sensor *handle, u32 *us)
{
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

static int pCus_SetAEUSecs(ms_cus_sensor *handle, u32 us)
{
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
    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_SEF1(ms_cus_sensor *handle, u32 us)
{
    u32 qua_lines = 0, lines = 0, long_lines = 0,vts = 0, fsc = 0;
    u32 rhs1 = 0, shs1 = 0, shs0 = 0;
    u32 max_rhs1 = 0;
    imx335_params *params = (imx335_params *)handle->private_data;

    qua_lines = (1000 * us) / Preview_line_period_HDR_DOL /4;
    vts = (params->tVts_reg[0].data << 16) | (params->tVts_reg[1].data << 8) | (params->tVts_reg[2].data << 0);
    shs0 = (params->tExpo_reg[0].data << 16) | (params->tExpo_reg[1].data << 8) | (params->tExpo_reg[2].data << 0);
    fsc = vts * 2;
    long_lines = fsc - shs0;
    params->expo.expo_lines = long_lines;
    max_rhs1 = 2594;
    rhs1 = max_rhs1;//(params->tExpo_rhs1_reg[0].data << 16) | (params->tExpo_rhs1_reg[1].data << 8) | (params->tExpo_rhs1_reg[2].data << 0);

    if(qua_lines <= 1)
        qua_lines = 1;
    if((4*qua_lines) > (rhs1- 18))
        qua_lines = (rhs1 - 18)/4;

    lines = 4*qua_lines;
    if((rhs1 - 18) <= lines){
        shs1 = 18;
    }
    else if((rhs1 <= max_rhs1) && (rhs1 <= shs0 - 18)){
        shs1 = rhs1 - lines;
        if((shs1 < 18) || (shs1 > (rhs1 - 4))){ //Check boundary
            //shs1 = 0;
            //UartSendTrace("[SEF1 NG1]");
        }
    }
    else{
        //UartSendTrace("[SEF1 NG2]");
    }

    params->tExpo_shr_dol1_reg[0].data = (shs1 >> 16) & 0x000f;
    params->tExpo_shr_dol1_reg[1].data = (shs1 >> 8) & 0x00ff;
    params->tExpo_shr_dol1_reg[2].data = (shs1 >> 0) & 0x00ff;

    params->tExpo_rhs1_reg[0].data = (rhs1 >> 16) & 0x000f;
    params->tExpo_rhs1_reg[1].data = (rhs1 >> 8) & 0x00ff;
    params->tExpo_rhs1_reg[2].data = (rhs1 >> 0) & 0x00ff;

    return SUCCESS;

/*     params->tExpo_reg[0].data = (shs1 >> 16) & 0x0003;
    params->tExpo_reg[1].data = (shs1 >> 8) & 0x00ff;
    params->tExpo_reg[2].data = (shs1 >> 0) & 0x00ff;

    params->tRhs1_reg[0].data = (rhs1 >> 16) & 0x0003;
    params->tRhs1_reg[1].data = (rhs1 >> 8) & 0x00ff;
    params->tRhs1_reg[2].data = (rhs1 >> 0) & 0x00ff; */

}

// Gain: 1x = 1024
static int pCus_GetAEGain(ms_cus_sensor *handle, u32* gain)
{
    //int rc = SensorRegArrayR((I2C_ARRAY*)params->tGain_reg, ARRAY_SIZE(gain_reg));
    unsigned short temp_gain;
  //  *gain=params->expo.final_gain;
    temp_gain=gain_reg[0].data;

    *gain=(u32)(10^((temp_gain*3)/200))*1024;
    if (gain_reg[1].data & 0x10)
       *gain = (*gain) * 2;

    SENSOR_DMSG("[%s] get gain/reg (1024=1X)= %u/0x%x\n", __FUNCTION__, *gain,gain_reg[0].data);
    //return rc;
    return SUCCESS;
}

static int pCus_SetAEGain_cal(ms_cus_sensor *handle, u32 gain)
{
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

    //return CusHW_i2c_array_tx(handle, handle->i2c_cfg, params->tGain_reg, sizeof(gain_reg)/sizeof(CUS_I2C_ARRAY));
    //return SensorRegArrayW((I2C_ARRAY*)params->tGain_reg, sizeof(gain_reg)/sizeof(I2C_ARRAY));
    params->dirty = true;
    return SUCCESS;
}

static int pCus_SetAEGain(ms_cus_sensor *handle, u32 gain)
{
    //extern DBG_ITEM Dbg_Items[DBG_TAG_MAX];
    imx335_params *params = (imx335_params *)handle->private_data;
    u32 i=0;
    CUS_GAIN_GAP_ARRAY* Sensor_Gain_Linearity;
    u64 gain_double;

    params->expo.final_gain = gain;
    if(gain < SENSOR_MIN_GAIN)
        gain = SENSOR_MIN_GAIN;
    else if(gain >= SENSOR_MAX_GAIN)
        gain = SENSOR_MAX_GAIN;
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

    if(gain>=22925)//if gain exceed 2x , enable high conversion gain, >27DB, enable HCG
    {
           if(params->tGain_reg[1].data==0x02){
           // params->change = true;
            // gain_reg[2].data=0x08;
            }
           else{

            }
            //gain_before=gain;
            params->tGain_reg[1].data |= 0x10;
           // gain_reg[2].data=0x08;
            gain /= 2;
    }
    else{
           if(params->tGain_reg[1].data==0x12){
           // params->change = true;
           //  gain_reg[2].data=0x08;
            }
           else{
           // params->change = false;
           // gain_reg[2].data=0x09;
            }
            params->tGain_reg[1].data &= ~0x10;
        }

    gain_double = 20*(intlog10(gain)-intlog10(1024));
    params->tGain_reg[0].data=(u16)(((gain_double*10)>> 24)/3) & 0x00ff;
    params->tGain_reg[1].data=(u16)((((gain_double*10)>> 24)/3) >> 8) & 0x0007;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain,params->tGain_reg[0].data);
    params->dirty = true;
    return SUCCESS;
}

static void pCus_SetAEGainHDR_DOL_Calculate(u32 gain, u16 *gain_reg)
{
    //double gain_double;
    u64 gain_double;

    if(gain < SENSOR_MIN_GAIN){
      gain = SENSOR_MIN_GAIN;
    }
    else if(gain >= SENSOR_MAX_GAIN){
      gain = SENSOR_MAX_GAIN;
    }
    gain_double = 20*(intlog10(gain)-intlog10(1024));
    *gain_reg=(u16)(((gain_double*10)>> 24)/3) & 0x07ff;
}

static int pCus_SetAEGainHDR_DOL_SEF1(ms_cus_sensor *handle, u32 gain)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    u16 gain_reg = 0;


    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_sef_reg[0].data = gain_reg & 0x00ff;
    params->tGain_hdr_dol_sef_reg[1].data = (gain_reg>>8) & 0x0007;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_sef_reg[0].data);

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetAEMinMaxUSecs(ms_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = 1;
    *max = 1000000/Preview_MIN_FPS;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain(ms_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = SENSOR_MIN_GAIN;//handle->sat_mingain;
    *max = SENSOR_MAX_GAIN;//3980*1024;
    return SUCCESS;
}
#if 0
static int pCus_GetDGainRemainder(ms_cus_sensor *handle, u32 *dgain_remainder)
{
    *dgain_remainder = handle->dgain_remainder;

    return SUCCESS;
}
#endif
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

int cus_camsensor_init_handle(ms_cus_sensor* drv_handle)
{
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
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tExpo_rhs1_reg, expo_rhs1_reg, sizeof(expo_rhs1_reg));
    memcpy(params->tExpo_shr_dol1_reg, expo_shr_dol1_reg, sizeof(expo_shr_dol1_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF1_reg, sizeof(gain_HDR_DOL_SEF1_reg));

/*     memcpy(params->tRhs1_reg, expo_RHS1_reg, sizeof(expo_RHS1_reg));
    memcpy(params->tShs2_reg, expo_SHS2_reg, sizeof(expo_SHS2_reg)); */

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
    handle->interface_attr.attr_mipi.mipi_lane_num = lane_num;
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
    if(lane_num == 2)
        handle->video_res_supported.res[0].max_fps= 25;
    else if(lane_num == 4)
        handle->video_res_supported.res[0].max_fps= 30;
    else
        handle->video_res_supported.res[0].max_fps= 30;

    handle->video_res_supported.res[0].min_fps= 3;
    handle->video_res_supported.res[0].crop_start_x= 0;
    handle->video_res_supported.res[0].crop_start_y= 0;
    handle->video_res_supported.res[0].nOutputWidth= 2592;
    handle->video_res_supported.res[0].nOutputHeight= 1944;
    if(lane_num == 2)
        sprintf(handle->video_res_supported.res[0].strResDesc, "2592x1944@25fps");
    else if(lane_num == 4)
        sprintf(handle->video_res_supported.res[0].strResDesc, "2592x1944@30fps");
    else
        sprintf(handle->video_res_supported.res[0].strResDesc, "2592x1944@30fps");

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    if(lane_num == 2)
        handle->mclk                        = Preview_MCLK_SPEED_2lane;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());
    else if(lane_num == 4)
        handle->mclk                        = Preview_MCLK_SPEED_4lane;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());
    else
        handle->mclk                        = Preview_MCLK_SPEED_4lane;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());

    //polarity
    /////////////////////////////////////////////////////
    handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->ae_gain_delay       = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->ae_shutter_delay    = SENSOR_SHUTTER_DELAY_FRAME_COUNT;

    handle->ae_gain_ctrl_num = 1;
    handle->ae_shutter_ctrl_num = 2;

    ///calibration
    handle->sat_mingain = SENSOR_MIN_GAIN;//g_sensor_ae_min_gain;
    //handle->dgain_remainder = 0;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_release     = cus_camsensor_release_handle;
        handle->pCus_sensor_init        = pCus_init_mipi4lane_linear    ;
    //handle->pCus_sensor_powerupseq  = pCus_powerupseq   ;
    handle->pCus_sensor_poweron     = pCus_poweron ;
    handle->pCus_sensor_poweroff    = pCus_poweroff;

    // Normal
    handle->pCus_sensor_GetSensorID       = pCus_GetSensorID   ;

    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes;
    handle->pCus_sensor_GetCurVideoRes  = pCus_GetCurVideoRes;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes;

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
    //handle->pCus_sensor_GetDGainRemainder = pCus_GetDGainRemainder;

     //sensor calibration
    handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal;
    handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity;
    handle->pCus_sensor_GetShutterInfo = IMX335_GetShutterInfo;

    params->expo.vts=vts_30fps;
    params->expo.expo_lines = 5000;
    params->expo.fps = 30;
    params->dirty = false;

    handle->channel_mode = SENSOR_CHANNEL_MODE;

    return SUCCESS;
}

int cus_camsensor_init_handle_hdr_dol_sef1(ms_cus_sensor* drv_handle)
{
    ms_cus_sensor *handle = drv_handle;
    imx335_params *params = NULL;

    cus_camsensor_init_handle(drv_handle);
    params = (imx335_params *)handle->private_data;

    sprintf(handle->model_id,"IMX335_MIPI_HDR_SEF");

    handle->bayer_id    = SENSOR_BAYERID_HDR_DOL;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->data_prec   = SENSOR_DATAPREC_DOL;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_DOL;//hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_SONY_DOL;

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[0].width = 2592;
    handle->video_res_supported.res[0].height = 1944; //TBD. Workaround for Sony DOL HDR mode
    handle->video_res_supported.res[0].max_fps= 25;
    handle->video_res_supported.res[0].min_fps= 3;
    handle->video_res_supported.res[0].crop_start_x= Preview_CROP_START_HDR_X;
    handle->video_res_supported.res[0].crop_start_y= Preview_CROP_START_Y;
    handle->video_res_supported.res[0].nOutputWidth = 2592;
    handle->video_res_supported.res[0].nOutputHeight = 1944;
    sprintf(handle->video_res_supported.res[0].strResDesc, "2592x1944@25fps_HDR");

    handle->video_res_supported.num_res = 2;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[1].width = 2592;
    handle->video_res_supported.res[1].height = 1944;
    handle->video_res_supported.res[1].max_fps= 20;
    handle->video_res_supported.res[1].min_fps= 3;
    handle->video_res_supported.res[1].crop_start_x= Preview_CROP_START_HDR_X;
    handle->video_res_supported.res[1].crop_start_y= Preview_CROP_START_Y;
    handle->video_res_supported.res[1].nOutputWidth = 2592;
    handle->video_res_supported.res[1].nOutputHeight = 1944;
    sprintf(handle->video_res_supported.res[1].strResDesc, "2592x1944@20fps_HDR");

    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_HDR_DOL;
    handle->mclk                        = Preview_MCLK_SPEED_HDR_DOL;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());
    handle->pCus_sensor_init        = pCus_init_mipi4lane25fps_HDR_DOL;

    handle->pCus_sensor_SetFPS          = pCus_SetFPS_HDR_DOL_SEF1;

    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotifyHDR_DOL_SEF1;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_SEF1;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain;
    //handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_DOL_SEF1;

    params->expo.vts = vts_30fps_HDR_DOL;

    params->expo.expo_lines = 5000;

    handle->channel_mode = SENSOR_CHANNEL_MODE_SONY_DOL;

    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    handle->ae_gain_delay       = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->ae_shutter_delay    = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;

    handle->ae_gain_ctrl_num = 2;
    handle->ae_shutter_ctrl_num = 2;

    return SUCCESS;
}
#if 1//(SENSOR_MIPI_LANE_NUM == 4)
static int pCus_init_mipi4lane_linear_isp_calibration(ms_cus_sensor *handle)
{
    return SUCCESS;
}
#endif

static int pCus_poweron_isp_calibration( ms_cus_sensor *handle, u32 idx)
{
    //ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    //SENSOR_DMSG( "[%s] ", __FUNCTION__ );

    return SUCCESS;
}

static int pCus_poweroff_isp_calibration( ms_cus_sensor *handle, u32 idx)
{
    //ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    //SENSOR_DMSG( "[%s] power low\n", __FUNCTION__ );

    return SUCCESS;
}

static int pCus_GetSensorID_isp_calibration( ms_cus_sensor *handle, u32 *id )
{
    *id = 0;
     return SUCCESS;
}

static int imx335_SetPatternMode_isp_calibration( ms_cus_sensor *handle, u32 mode )
{
    return SUCCESS;
}
#if 0
static int pCus_I2CWrite_isp_calibration(ms_cus_sensor *handle, unsigned short usreg, unsigned short usval)
{
    return SUCCESS;
}

static int pCus_I2CRead_isp_calibration(ms_cus_sensor *handle, unsigned short usreg)
{
    return SUCCESS;
}

static int pCus_init_isp_calibration( ms_cus_sensor *handle )
{
    return SUCCESS;
}
#endif
static int pCus_GetVideoRes_isp_calibration( ms_cus_sensor *handle, u32 res_idx, cus_camsensor_res **res )
{
    *res = &handle->video_res_supported.res[res_idx];
    return SUCCESS;
}

static int pCus_SetVideoRes_isp_calibration( ms_cus_sensor *handle, u32 res )
{
    handle->video_res_supported.ulcur_res = 0; //TBD
    return SUCCESS;
}

static int pCus_GetOrien_isp_calibration( ms_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit )
{
    //imx335_params *params = ( imx335_params * ) handle->private_data;
    //return params->mirror_flip.cur;
    return SUCCESS;
}

static int pCus_SetOrien_isp_calibration( ms_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit )
{
    return SUCCESS;
}

static int pCus_GetFPS_isp_calibration( ms_cus_sensor *handle )
{
    return SUCCESS;
}

static int pCus_SetFPS_isp_calibration( ms_cus_sensor *handle, u32 fps )
{
    return SUCCESS;
}
#if 0
static int pCus_GetSensorCap_isp_calibration( ms_cus_sensor *handle, CUS_CAMSENSOR_CAP *cap )
{
    if( cap )
        memcpy( cap, &sensor_cap, sizeof(CUS_CAMSENSOR_CAP) );
    else
        return FAIL;
    return SUCCESS;
}
#endif
static int pCus_AEStatusNotify_isp_calibration( ms_cus_sensor *handle, CUS_CAMSENSOR_AE_STATUS_NOTIFY status )
{
    return SUCCESS;
}

static int pCus_GetAEUSecs_isp_calibration( ms_cus_sensor *handle, u32 *us )
{
    *us = 0;
    return SUCCESS;
}

static int pCus_SetAEUSecs_isp_calibration( ms_cus_sensor *handle, u32 us )
{
    return SUCCESS;
}

// Gain: 1x = 1024
static int pCus_GetAEGain_isp_calibration( ms_cus_sensor *handle, u32* gain )
{
    *gain = 0;
    return SUCCESS;
}

static int pCus_SetAEGain_cal_isp_calibration( ms_cus_sensor *handle, u32 gain )
{
    return SUCCESS;
}
static int pCus_SetAEGain_isp_calibration( ms_cus_sensor *handle, u32 gain )
{
    return SUCCESS;
}

static int pCus_GetAEMinMaxUSecs_isp_calibration( ms_cus_sensor *handle, u32 *min, u32 *max )
{
    *min = 1;
    *max = 1000000 / 30;

    return SUCCESS;
}

static int pCus_GetAEMinMaxGain_isp_calibration( ms_cus_sensor *handle, u32 *min, u32 *max )
{
    *min = 1024;            //1024*1.1*2
    *max = 1024*8;
    return SUCCESS;
}

static int pCus_setCaliData_gain_linearity_isp_calibration( ms_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num )
{
    return SUCCESS;
}

static int IMX335_GetShutterInfo_isp_calibration( struct __ms_cus_sensor* handle, CUS_SHUTTER_INFO *info )
{
    info->max = 1000000000 /Preview_MIN_FPS;
    info->min = Preview_line_period;
    info->step = Preview_line_period;
    return SUCCESS;
}
#if 0
static int pCus_GetDGainRemainder_isp_calibration(ms_cus_sensor *handle, u32 *dgain_remainder)
{
   return SUCCESS;
}
#endif
int cus_camsensor_init_handle_linear_isp_calibration(ms_cus_sensor* drv_handle)
{
    ms_cus_sensor *handle = drv_handle;
    imx335_params *params;
#if defined(__RTK_OS__)
    CamOsRet_e                  eCamOsRet = CAM_OS_OK;
    void*                       pvBufVirt = NULL;
    u64                 u64BufPhy = 0;
    u64                 u64BufMiu = 0;
    u8                   str[20];
#endif
    if (!handle) {
        SENSOR_DMSG("[%s] not enough memory!\n", __FUNCTION__);
        return FAIL;
    }
    SENSOR_DMSG("[%s]", __FUNCTION__);
    //private data allocation & init
#if defined(__RTK_OS__)
    sprintf(str, "IMX335linear");
    eCamOsRet = CamOsDirectMemAlloc(str, sizeof(imx335_params), &pvBufVirt, &u64BufMiu, &u64BufPhy);
    if (eCamOsRet != CAM_OS_OK) {
        //UartSendTrace("[%s:%d] fail!\n", __FUNCTION__, __LINE__);
        return FAIL;
    }
    handle->private_data = (imx335_params *)pvBufVirt;
    memset(handle->private_data, 0, sizeof(imx335_params));
#else
    if (handle->private_data == NULL) {
        SENSOR_EMSG("[%s] Private data is empty!\n", __FUNCTION__);
        return FAIL;
    }
#endif
    params = (imx335_params *)handle->private_data;
    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tExpo_rhs1_reg, expo_rhs1_reg, sizeof(expo_rhs1_reg));
    memcpy(params->tExpo_shr_dol1_reg, expo_shr_dol1_reg, sizeof(expo_shr_dol1_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF1_reg, sizeof(gain_HDR_DOL_SEF1_reg));
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
    handle->interface_attr.attr_mipi.mipi_lane_num = lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_NONE;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num = 1; //Short frame

    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[0].width = Preview_WIDTH;
    handle->video_res_supported.res[0].height = Preview_HEIGHT;
    handle->video_res_supported.res[0].max_fps= 25;
    handle->video_res_supported.res[0].min_fps= 3;
    handle->video_res_supported.res[0].crop_start_x= Preview_CROP_START_X;
    handle->video_res_supported.res[0].crop_start_y= Preview_CROP_START_Y;
    handle->video_res_supported.res[0].nOutputWidth = 2592;
    handle->video_res_supported.res[0].nOutputHeight = 1944;
    sprintf(handle->video_res_supported.res[0].strResDesc, "2592x1944@30fps");

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    if(lane_num == 2)
        handle->mclk                        = Preview_MCLK_SPEED_2lane;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());
    else if(lane_num == 4)
        handle->mclk                        = Preview_MCLK_SPEED_4lane;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());
    else
        handle->mclk                        = Preview_MCLK_SPEED_4lane;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());


    //polarity
    /////////////////////////////////////////////////////
    handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////



    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->ae_gain_delay       = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->ae_shutter_delay    = SENSOR_SHUTTER_DELAY_FRAME_COUNT;

    handle->ae_gain_ctrl_num = 1;
    handle->ae_shutter_ctrl_num = 2;

    ///calibration
    handle->sat_mingain = SENSOR_MIN_GAIN;//g_sensor_ae_min_gain;
    //handle->dgain_remainder = 0;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_release     = cus_camsensor_release_handle;

        handle->pCus_sensor_init        = pCus_init_mipi4lane_linear_isp_calibration    ;

    //handle->pCus_sensor_powerupseq  = pCus_powerupseq   ;
    handle->pCus_sensor_poweron     = pCus_poweron_isp_calibration ;
    handle->pCus_sensor_poweroff    = pCus_poweroff_isp_calibration;

    // Normal
    handle->pCus_sensor_GetSensorID       = pCus_GetSensorID_isp_calibration   ;
    //handle->pCus_sensor_GetStillResCap    = pCus_GetStillResCap;
    //handle->pCus_sensor_GetStillRes       = pCus_GetStillRes   ;
    //handle->pCus_sensor_SetStillRes       = pCus_SetStillRes   ;
    //handle->pCus_sensor_GetVideoResCap    = pCus_GetVideoResCap;
    handle->pCus_sensor_GetVideoResNum = pCus_GetVideoResNum;
    handle->pCus_sensor_GetVideoRes       = pCus_GetVideoRes_isp_calibration   ;
    handle->pCus_sensor_SetVideoRes       = pCus_SetVideoRes_isp_calibration   ;
    handle->pCus_sensor_GetOrien          = pCus_GetOrien_isp_calibration      ;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien_isp_calibration      ;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS_isp_calibration      ;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_isp_calibration      ;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap_isp_calibration;
    handle->pCus_sensor_SetPatternMode = imx335_SetPatternMode_isp_calibration;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotify_isp_calibration;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_isp_calibration;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecs_isp_calibration;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain_isp_calibration;
    //handle->pCus_sensor_SetAEGain       = pCus_SetAEGain;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGain_isp_calibration;

    handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain_isp_calibration;
    handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs_isp_calibration;
    //handle->pCus_sensor_GetDGainRemainder = pCus_GetDGainRemainder_isp_calibration;

     //sensor calibration
    handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal_isp_calibration;
    handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity_isp_calibration;
    handle->pCus_sensor_GetShutterInfo = IMX335_GetShutterInfo_isp_calibration;
#if 0//defined(__MV5_FPGA__)
    handle->pCus_sensor_I2CWrite = pCus_I2CWrite_isp_calibration; //Andy Liu
    handle->pCus_sensor_I2CRead = pCus_I2CRead_isp_calibration; //Andy Liu
#endif
    params->expo.vts=vts_30fps;
    params->expo.expo_lines = 673;
    params->expo.fps = 25;
    params->dirty = false;

    //handle->channel_num = SENSOR_CHANNEL_NUM;
    //handle->channel_mode = CUS_SENSOR_CHANNEL_MODE_RAW_STORE_ISP_CALIBRATION;

    return SUCCESS;
}
//lef functions
static int pCus_init_hdr_dol_lef(ms_cus_sensor *handle)
{
    return SUCCESS;
}

static int pCus_poweron_hdr_dol_lef(ms_cus_sensor *handle, u32 idx)
{
#if 0
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    SENSOR_DMSG( "[%s] ", __FUNCTION__ );

    sensor_if->PowerOff(CUS_SENSOR_PAD_GROUP_B/*TBD?*/, !handle->pwdn_POLARITY );
    MsSleep(RTK_MS_TO_TICK(10));//usleep( 500 );

    //sensor_if->Reset(CUS_SENSOR_PAD_GROUP_B, !handle->reset_POLARITY );
    //MsSleep(RTK_MS_TO_TICK(5));//usleep( 500 );

    // pure power on
    sensor_if->PowerOff(CUS_SENSOR_PAD_GROUP_B/*TBD?*/, !handle->pwdn_POLARITY );
#endif
    return SUCCESS;
}

static int pCus_poweroff_hdr_dol_lef(ms_cus_sensor *handle, u32 idx)
{
#if 0
    ISensorIfAPI *sensor_if = &handle->sensor_if_api;
    SENSOR_DMSG( "[%s] power low\n", __FUNCTION__ );

    sensor_if->PowerOff(CUS_SENSOR_PAD_GROUP_B/*TBD?*/, handle->pwdn_POLARITY );
    MsSleep(RTK_MS_TO_TICK(5));//usleep( 1000 );
#endif
    return SUCCESS;
}

static int pCus_GetSensorID_hdr_dol_lef(ms_cus_sensor *handle, u32 *id)
{
    *id = 0;
     return SUCCESS;
}

static int pCus_GetOrien_hdr_dol_lef(ms_cus_sensor *handle, CUS_CAMSENSOR_ORIT *orit)
{
    *orit = CUS_ORIT_M0F0;
    return SUCCESS;
}

static int pCus_SetOrien_hdr_dol_lef(ms_cus_sensor *handle, CUS_CAMSENSOR_ORIT orit)
{
    return SUCCESS;
}

static int pCus_GetFPS_hdr_dol_lef(ms_cus_sensor *handle)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    //SENSOR_DMSG("[%s]", __FUNCTION__);

    return  params->expo.fps;
}

static int pCus_SetFPS_hdr_dol_lef(ms_cus_sensor *handle, u32 fps)
{
    u32 vts = 0, max_rhs1 = 0;
    imx335_params *params = (imx335_params *)handle->private_data;
    u32 max_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].max_fps;
    u32 min_fps = handle->video_res_supported.res[handle->video_res_supported.ulcur_res].min_fps;

    if(fps>=min_fps && fps <= max_fps){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR_DOL*max_fps)/fps;
    }else if((fps >= (min_fps*1000)) && (fps <= (max_fps*1000))){
        params->expo.fps = fps;
        params->expo.vts=  (vts_30fps_HDR_DOL*(max_fps*1000))/fps;
    }else{
        //params->expo.vts=vts_30fps;
        //params->expo.fps=30;
        //SENSOR_DMSG("[%s] FPS %d out of range.\n",__FUNCTION__,fps);
        return FAIL;
    }

    max_rhs1 = 2594;
    if(params->expo.expo_lines > 2 * params->expo.vts - max_rhs1 -18){
        vts = (params->expo.expo_lines + max_rhs1 + 18) / 2;
    }else{
        vts = params->expo.vts;
    }

    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;
    //printf("[%s] params->expo.vts : %d vts : 0x%x, 0x%x, 0x%x fps:%d \r\n",__FUNCTION__,params->expo.vts, params->tVts_reg[0].data,params->tVts_reg[1].data,params->tVts_reg[2].data, params->expo.fps);

    return SUCCESS;
}
#if 0
static int pCus_GetSensorCap_hdr_dol_lef(ms_cus_sensor *handle, CUS_CAMSENSOR_CAP *cap)
{
    if (cap)
        memcpy(cap, &sensor_cap, sizeof(CUS_CAMSENSOR_CAP));
    else     return FAIL;
    return SUCCESS;
}
#endif
static int imx335_SetPatternMode_hdr_dol_lef(ms_cus_sensor *handle,u32 mode)
{
    return SUCCESS;
}

static int pCus_AEStatusNotifyHDR_DOL_LEF(ms_cus_sensor *handle, CUS_CAMSENSOR_AE_STATUS_NOTIFY status)
{
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
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_shr_dol1_reg, ARRAY_SIZE(expo_shr_dol1_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tVts_reg, ARRAY_SIZE(vts_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_reg, ARRAY_SIZE(expo_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tExpo_rhs1_reg, ARRAY_SIZE(expo_rhs1_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_lef_reg, ARRAY_SIZE(gain_HDR_DOL_LEF_reg));
                SensorRegArrayW((I2C_ARRAY*)params->tGain_hdr_dol_sef_reg, ARRAY_SIZE(gain_HDR_DOL_SEF1_reg));
/*              printk("[%s] vts  : 0x%x  0x%x  0x%x \n", __FUNCTION__, params->tVts_reg[0].data, params->tVts_reg[1].data, params->tVts_reg[2].data);
                printk("[%s] shr0 : 0x%x  0x%x  0x%x \n", __FUNCTION__, params->tExpo_reg[0].data, params->tExpo_reg[1].data, params->tExpo_reg[2].data);
                printk("[%s] shr1 : 0x%x  0x%x  0x%x \n", __FUNCTION__, params->tExpo_shr_dol1_reg[0].data, params->tExpo_shr_dol1_reg[1].data, params->tExpo_shr_dol1_reg[2].data);
                printk("[%s] rhs1 : 0x%x  0x%x  0x%x \n", __FUNCTION__, params->tExpo_rhs1_reg[0].data, params->tExpo_rhs1_reg[1].data, params->tExpo_rhs1_reg[2].data);
 *//*
                if(params->change){

                    // sensor_if1->SetSkipFrame(handle,3);
                    params->change = false;

                }
*/
                params->dirty = false;
            }
            break;
        default :
             break;
    }
    return SUCCESS;
}

static int pCus_GetAEUSecs_hdr_dol_lef(ms_cus_sensor *handle, u32 *us)
{
    *us = 0;
    return SUCCESS;
}

static int pCus_SetAEUSecsHDR_DOL_LEF(ms_cus_sensor *handle, u32 us)
{
    u32 qua_lines = 0,lines = 0, half_vts = 0, vts = 0, shr_dol0 = 0, fsc = 0;
    u32 max_rhs1 = 0;
    imx335_params *params = (imx335_params *)handle->private_data;

    qua_lines = (1000 * us) / Preview_line_period_HDR_DOL / 4;
   // lines=us/Preview_line_period_HDR_DOL;

    max_rhs1 =2594;
    if (4 * qua_lines > 2 * params->expo.vts - max_rhs1 - 18) { // shs2 > max_rhs1 +2
        half_vts = (4 * qua_lines + max_rhs1 + 19) / 4;
    }
    else{
        half_vts = params->expo.vts / 2;
    }

    SENSOR_DMSG("[%s] us %u, qua_lines %u, vts %u\n", __FUNCTION__,
                us,
                qua_lines,
                params->expo.vts
                );

    //exposure limit lines = fsc - (shs2 + 1) = fsc - 1 - ( rhs1 + 2 ~ fsc - 2) = 1 ~ fsc - 104 (rhs1 fix to 101)
    vts = half_vts * 2;
    fsc = half_vts * 4;
    if(qua_lines < 1)  // shs2 < fsc - 2
        qua_lines = 1;
    if(4 * qua_lines > fsc - max_rhs1 - 18)
        qua_lines = (fsc - max_rhs1 - 18) / 4;

    lines = 4*qua_lines;
    params->expo.expo_lines = lines;

    shr_dol0 = fsc - lines;

    params->tExpo_reg[0].data = (shr_dol0 >> 16) & 0x000f;
    params->tExpo_reg[1].data = (shr_dol0 >> 8) & 0x00ff;
    params->tExpo_reg[2].data = (shr_dol0 >> 0) & 0x00ff;

    params->tVts_reg[0].data = (vts >> 16) & 0x000f;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff;

    params->dirty = true;
    return SUCCESS;

/*     params->tShs2_reg[0].data = (shs2 >> 16) & 0x0003;
    params->tShs2_reg[1].data = (shs2 >> 8) & 0x00ff;
    params->tShs2_reg[2].data = (shs2 >> 0) & 0x00ff;

    params->tVts_reg[0].data = (vts >> 16) & 0x0003;
    params->tVts_reg[1].data = (vts >> 8) & 0x00ff;
    params->tVts_reg[2].data = (vts >> 0) & 0x00ff; */
}

static int pCus_GetAEGain_hdr_dol_lef(ms_cus_sensor *handle, u32* gain)
{
    *gain = 0;
    return SUCCESS;
}

static int pCus_SetAEGainHDR_DOL_LEF(ms_cus_sensor *handle, u32 gain)
{
    imx335_params *params = (imx335_params *)handle->private_data;
    u16 gain_reg = 0;


    pCus_SetAEGainHDR_DOL_Calculate(gain, &gain_reg);
    params->tGain_hdr_dol_lef_reg[0].data = gain_reg & 0x00ff;
    params->tGain_hdr_dol_lef_reg[1].data = (gain_reg>>8) & 0x0007;

    SENSOR_DMSG("[%s] set gain/reg=%u/0x%x\n", __FUNCTION__, gain, params->tGain_hdr_dol_lef_reg[0].data);

    params->dirty = true;
    return SUCCESS;
}

static int pCus_GetAEMinMaxGain_hdr_dol_lef(ms_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = handle->sat_mingain;
    *max = 3980*1024;
    return SUCCESS;
}

static int pCus_GetAEMinMaxUSecs_hdr_dol_lef(ms_cus_sensor *handle, u32 *min, u32 *max)
{
    *min = 1;
    *max = 1000000/Preview_MIN_FPS;
    return SUCCESS;
}

//static int pCus_GetDGainRemainder(ms_cus_sensor *handle, u32 *dgain_remainder)
//{
//    *dgain_remainder = handle->dgain_remainder;

//    return SUCCESS;
//}

static int pCus_SetAEGain_cal_hdr_dol_lef(ms_cus_sensor *handle, u32 gain)
{
    return SUCCESS;
}

static int pCus_setCaliData_gain_linearity_hdr_dol_lef(ms_cus_sensor* handle, CUS_GAIN_GAP_ARRAY* pArray, u32 num)
{
    return SUCCESS;
}

static int IMX335_GetShutterInfo_hdr_dol_lef(struct __ms_cus_sensor* handle,CUS_SHUTTER_INFO *info)
{
    info->max = 1000000000/Preview_MIN_FPS;
    info->min = (Preview_line_period_HDR_DOL * 4);
    info->step = Preview_line_period_HDR_DOL;
    return SUCCESS;
}
#if 0
static int pCus_I2CWrite_hdr_dol_lef(ms_cus_sensor *handle, unsigned short usreg, unsigned short usval)
{
    return SUCCESS;
}

static int pCus_I2CRead_hdr_dol_lef(ms_cus_sensor *handle, unsigned short usreg)
{
    return SUCCESS;
}
#endif
//
static int cus_camsensor_init_handle_hdr_dol_lef(ms_cus_sensor* drv_handle)
{
    ms_cus_sensor *handle = drv_handle;
    //ISensorIfAPI *sensor_if = &handle->sensor_if_api;
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

    memcpy(params->tGain_reg, gain_reg, sizeof(gain_reg));
    memcpy(params->tExpo_reg, expo_reg, sizeof(expo_reg));
    memcpy(params->tVts_reg, vts_reg, sizeof(vts_reg));
    memcpy(params->tExpo_rhs1_reg, expo_rhs1_reg, sizeof(expo_rhs1_reg));
    memcpy(params->tExpo_shr_dol1_reg, expo_shr_dol1_reg, sizeof(expo_shr_dol1_reg));
    memcpy(params->tGain_hdr_dol_lef_reg, gain_HDR_DOL_LEF_reg, sizeof(gain_HDR_DOL_LEF_reg));
    memcpy(params->tGain_hdr_dol_sef_reg, gain_HDR_DOL_SEF1_reg, sizeof(gain_HDR_DOL_SEF1_reg));

    ////////////////////////////////////
    //    sensor model ID                           //
    ////////////////////////////////////
    sprintf(handle->model_id,"IMX335_MIPI_HDR_LEF");

    ////////////////////////////////////
    //    sensor interface info       //
    ////////////////////////////////////
    //SENSOR_DMSG("[%s] entering function with id %d\n", __FUNCTION__, id);
    handle->isp_type    = SENSOR_ISP_TYPE;  //ISP_SOC;
    //handle->data_fmt    = SENSOR_DATAFMT;   //CUS_DATAFMT_YUV;
    handle->sif_bus     = SENSOR_IFBUS_TYPE;//CUS_SENIF_BUS_PARL;
    handle->data_prec   = SENSOR_DATAPREC_DOL;  //CUS_DATAPRECISION_8;
    handle->data_mode   = SENSOR_DATAMODE;
    handle->bayer_id    = SENSOR_BAYERID_HDR_DOL;   //CUS_BAYER_GB;
    handle->RGBIR_id    = SENSOR_RGBIRID;
    handle->orient      = SENSOR_ORIT;      //CUS_ORIT_M1F1;
    //handle->YC_ODER     = SENSOR_YCORDER;   //CUS_SEN_YCODR_CY;
    handle->interface_attr.attr_mipi.mipi_lane_num = SENSOR_MIPI_LANE_NUM_DOL;//hdr_lane_num;
    handle->interface_attr.attr_mipi.mipi_data_format = CUS_SEN_INPUT_FORMAT_RGB; // RGB pattern.
    handle->interface_attr.attr_mipi.mipi_yuv_order = 0; //don't care in RGB pattern.
    handle->interface_attr.attr_mipi.mipi_hsync_mode = SENSOR_MIPI_HSYNC_MODE_HDR_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_mode = CUS_HDR_MODE_SONY_DOL;
    handle->interface_attr.attr_mipi.mipi_hdr_virtual_channel_num =  0; //Long frame
    ////////////////////////////////////
    //    resolution capability       //
    ////////////////////////////////////

    handle->video_res_supported.num_res = 1;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[0].width = 2592;
    handle->video_res_supported.res[0].height = 1944; //TBD. Workaround for Sony DOL HDR mode
    handle->video_res_supported.res[0].max_fps= 25;
    handle->video_res_supported.res[0].min_fps= 3;
    handle->video_res_supported.res[0].crop_start_x= Preview_CROP_START_HDR_X;
    handle->video_res_supported.res[0].crop_start_y= Preview_CROP_START_Y;
    handle->video_res_supported.res[0].nOutputWidth = 2592;
    handle->video_res_supported.res[0].nOutputHeight = 1944;
    sprintf(handle->video_res_supported.res[0].strResDesc, "2592x1944@25fps_HDR");

    handle->video_res_supported.num_res = 2;
    handle->video_res_supported.ulcur_res = 0; //default resolution index is 0.
    handle->video_res_supported.res[1].width = 2592;
    handle->video_res_supported.res[1].height = 1944;
    handle->video_res_supported.res[1].max_fps= 20;
    handle->video_res_supported.res[1].min_fps= 3;
    handle->video_res_supported.res[1].crop_start_x= Preview_CROP_START_HDR_X;
    handle->video_res_supported.res[1].crop_start_y= Preview_CROP_START_Y;
    handle->video_res_supported.res[1].nOutputWidth = 2592;
    handle->video_res_supported.res[1].nOutputHeight = 1944;
    sprintf(handle->video_res_supported.res[1].strResDesc, "2592x1944@20fps_HDR");

    // i2c
    handle->i2c_cfg.mode                = SENSOR_I2C_LEGACY;    //(CUS_ISP_I2C_MODE) FALSE;
    handle->i2c_cfg.fmt                 = SENSOR_I2C_FMT;       //CUS_I2C_FMT_A16D16;
    handle->i2c_cfg.address             = SENSOR_I2C_ADDR;      //0x5a;
    handle->i2c_cfg.speed               = SENSOR_I2C_SPEED;     //320000;

    // mclk
    handle->mclk                        = Preview_MCLK_SPEED_HDR_DOL;//UseParaMclk(SENSOR_DRV_PARAM_MCLK());
    //sensor_if->MCLK(0,1,handle->mclk);

    //polarity
    /////////////////////////////////////////////////////
    handle->pwdn_POLARITY               = SENSOR_PWDN_POL;  //CUS_CLK_POL_NEG;
    handle->reset_POLARITY              = SENSOR_RST_POL;   //CUS_CLK_POL_NEG;
    //handle->VSYNC_POLARITY              = SENSOR_VSYNC_POL; //CUS_CLK_POL_POS;
    //handle->HSYNC_POLARITY              = SENSOR_HSYNC_POL; //CUS_CLK_POL_POS;
    //handle->PCLK_POLARITY               = SENSOR_PCLK_POL;  //CUS_CLK_POL_POS);    // use '!' to clear board latch error
    /////////////////////////////////////////////////////



    ////////////////////////////////////////////////////
    // AE parameters
    ////////////////////////////////////////////////////
    handle->ae_gain_delay       = SENSOR_GAIN_DELAY_FRAME_COUNT;
    handle->ae_shutter_delay    = SENSOR_SHUTTER_DELAY_FRAME_COUNT_HDR_DOL;

    handle->ae_gain_ctrl_num = 2;
    handle->ae_shutter_ctrl_num = 2;

    ///calibration
    handle->sat_mingain = SENSOR_MIN_GAIN;//g_sensor_ae_min_gain;
    //handle->dgain_remainder = 0;

    //LOGD("[%s:%d]\n", __FUNCTION__, __LINE__);
    handle->pCus_sensor_release     = cus_camsensor_release_handle;
    handle->pCus_sensor_init        = pCus_init_hdr_dol_lef;
    //handle->pCus_sensor_powerupseq  = pCus_powerupseq   ;
    handle->pCus_sensor_poweron     = pCus_poweron_hdr_dol_lef;
    handle->pCus_sensor_poweroff    = pCus_poweroff_hdr_dol_lef;

    // Normal
    handle->pCus_sensor_GetSensorID       = pCus_GetSensorID_hdr_dol_lef;

    handle->pCus_sensor_GetVideoResNum = NULL;
    handle->pCus_sensor_GetVideoRes       = NULL;
    handle->pCus_sensor_GetCurVideoRes  = NULL;
    handle->pCus_sensor_SetVideoRes       = NULL;

    handle->pCus_sensor_GetOrien          = pCus_GetOrien_hdr_dol_lef;
    handle->pCus_sensor_SetOrien          = pCus_SetOrien_hdr_dol_lef;
    handle->pCus_sensor_GetFPS          = pCus_GetFPS_hdr_dol_lef;
    handle->pCus_sensor_SetFPS          = pCus_SetFPS_hdr_dol_lef;
    //handle->pCus_sensor_GetSensorCap    = pCus_GetSensorCap_hdr_dol_lef;
    handle->pCus_sensor_SetPatternMode = imx335_SetPatternMode_hdr_dol_lef;
    ///////////////////////////////////////////////////////
    // AE
    ///////////////////////////////////////////////////////
    // unit: micro seconds
    //handle->pCus_sensor_GetAETrigger_mode      = pCus_GetAETrigger_mode;
    //handle->pCus_sensor_SetAETrigger_mode      = pCus_SetAETrigger_mode;
    handle->pCus_sensor_AEStatusNotify = pCus_AEStatusNotifyHDR_DOL_LEF;
    handle->pCus_sensor_GetAEUSecs      = pCus_GetAEUSecs_hdr_dol_lef;
    handle->pCus_sensor_SetAEUSecs      = pCus_SetAEUSecsHDR_DOL_LEF;
    handle->pCus_sensor_GetAEGain       = pCus_GetAEGain_hdr_dol_lef;
    //handle->pCus_sensor_SetAEGain       = pCus_SetAEGain_hdr_dol_lef;
    handle->pCus_sensor_SetAEGain       = pCus_SetAEGainHDR_DOL_LEF;

    handle->pCus_sensor_GetAEMinMaxGain = pCus_GetAEMinMaxGain_hdr_dol_lef;
    handle->pCus_sensor_GetAEMinMaxUSecs= pCus_GetAEMinMaxUSecs_hdr_dol_lef;
    //handle->pCus_sensor_GetDGainRemainder = pCus_GetDGainRemainder;

     //sensor calibration
    handle->pCus_sensor_SetAEGain_cal   = pCus_SetAEGain_cal_hdr_dol_lef;
    handle->pCus_sensor_setCaliData_gain_linearity=pCus_setCaliData_gain_linearity_hdr_dol_lef;
    handle->pCus_sensor_GetShutterInfo = IMX335_GetShutterInfo_hdr_dol_lef;

    params->expo.vts = vts_30fps_HDR_DOL;
    params->expo.expo_lines = 673;
    params->expo.fps = 25;

    params->dirty = false;

    //handle->channel_num = SENSOR_CHANNEL_NUM + 1;
    handle->channel_mode = SENSOR_CHANNEL_MODE_SONY_DOL;

    return SUCCESS;
}

static int cus_camsensor_release_handle(ms_cus_sensor *handle)
{
    return SUCCESS;
}

/* static CUS_MCLK_FREQ UseParaMclk(const char *mclk)
{
    if (strcmp(mclk, "27M") == 0) {
        return CUS_CMU_CLK_27MHZ;
    } else if (strcmp(mclk, "12M") == 0) {
        return CUS_CMU_CLK_12MHZ;
    } else if (strcmp(mclk, "36M") == 0) {
        return CUS_CMU_CLK_36MHZ;
    } else if (strcmp(mclk, "48M") == 0) {
        return CUS_CMU_CLK_48MHZ;
    } else if (strcmp(mclk, "54M") == 0) {
        return CUS_CMU_CLK_54MHZ;
    } else if (strcmp(mclk, "24M") == 0) {
        return CUS_CMU_CLK_24MHZ;
    } else if (strcmp(mclk, "37.125M") == 0) {
        return CUS_CMU_CLK_37P125MHZ;
    }

    return Preview_MCLK_SPEED;
} */

SENSOR_DRV_ENTRY_IMPL_END_EX(  IMX335_HDR,
                            cus_camsensor_init_handle,
                            cus_camsensor_init_handle_hdr_dol_sef1,
                            cus_camsensor_init_handle_hdr_dol_lef,
                            imx335_params
                         );
