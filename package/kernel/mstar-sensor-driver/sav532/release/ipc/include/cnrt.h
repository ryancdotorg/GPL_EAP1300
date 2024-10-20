/*************************************************************************
 * Copyright (C) [2018] by Cambricon, Inc.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *************************************************************************/

/************************************************************************
 *
 *  @file cnrt.h
 *
 *  @brief Runtime APIs provide programmable interfaces for users to develop
 *  their-owned programs, which includes device managment, context
 *  management, memory managment of both sides (devices and hosts), etc.
 *
 **************************************************************************/

#ifndef __CNRT_H
#define __CNRT_H

/************************************************************************
 *  Include files
 ************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif /*__cplusplus*/

/************************************************************************
 *  Definitions
 ************************************************************************/
/**< DLL exports controller. */
#if defined(WIN32) || defined(WINDOWS)
#ifdef USE_CNRT_DLL
#  ifdef CNRT_DLL_EXPORTS
#    define CNRT_DLL_API __declspec(dllexport)
#  else /*CNRT_DLL_EXPORTS*/
#    define CNRT_DLL_API __declspec(dllimport)
#  endif /*CNRT_DLL_EXPORTS*/
#else
#define CNRT_DLL_API
#endif /*USE_CNRT_DLL*/
#else /*WIN32 || WINDOWS*/
#  define CNRT_DLL_API
#endif /*WIN32 || WINDOWS*/

/**< Code will be compiled for running on Host */
#ifndef __R_HOST
#define __R_HOST   // something like __attribute__ or #pragma
#endif /*__R_HOST*/

/**< Code will be compiled for running on MLU */
#ifndef __R_MLU
#define __R_MLU
#endif /*__R_MLU*/

/**< Interaction interface */
#ifndef __R_GLOBAL
#define __R_GLOBAL
#endif /*__R_GLOBAL*/

/**< Stream priorities */
#define CNRT_STREAM_PRIORITY_LOW        -16
#define CNRT_STREAM_PRIORITY_MID        0
#define CNRT_STREAM_PRIORITY_HIGH       16

/**< Memory permissions */
#define CNRT_MEMORY_PERMS_READ           0
#define CNRT_MEMORY_PERMS_WRITE          1
#define CNRT_MEMORY_PERMS_EXEC           2

/**< channel core num */
#define CNRT_CORE_NUM_PER_CHANNEL 8

/**< struct tailed */
#define CNRT_PARAM_END (void *)0xFFFFFFFF



/************************************************************************
 *  Data type declaration
 ************************************************************************/

#ifndef __CAMB_TYPES_H
#define __CAMB_TYPES_H
#if defined(WIN32) || defined(WINDOWS)
typedef unsigned __int64 u64_t;
typedef          __int64 i64_t;
typedef unsigned __int64 camb_u64_t;
typedef          __int64 camb_i64_t;
typedef unsigned __int32 u32_t;
typedef unsigned __int16 u16_t;
typedef unsigned __int8  u8_t;
typedef signed   __int32 i32_t;
typedef signed   __int16 i16_t;
typedef signed   __int8 i8_t;
typedef int           bool_t;
typedef u64_t    camb_size_t;

#else /*!WIN32 || WINDOWS*/

typedef uint64_t u64_t;
typedef int64_t  i64_t;
typedef uint64_t camb_u64_t;
typedef int64_t  camb_i64_t;
typedef uint32_t u32_t;
typedef uint16_t u16_t;
typedef uint8_t  u8_t;
typedef int32_t  i32_t;
typedef int16_t  i16_t;
typedef int8_t   i8_t;
typedef int           bool_t;
typedef u64_t    camb_size_t;

#endif /*WIN32||WINDOWS*/
#endif /*__CAMB_TYPES*/



typedef char camb_bool;

#define CNRT_CHECK(statment) do {            \
  int ret_code = (statment);                 \
  if (ret_code != CNRT_RET_SUCCESS) {        \
    printf("[%s:%d] CNRT error, code: %d\n", \
        __FILE__, __LINE__, ret_code);       \
    exit(1);                                 \
  }                                          \
} while (false);                             \

/**< Error codes */
typedef enum {
  CNRT_RET_SUCCESS = 0,                      /**< No error */
  CNRT_RET_WARNING_FAKE_DEVICE = 1,          /**< Use fake device */
  CNRT_RET_ERR_INVALID = 632007,             /**< Invalid argument */
  CNRT_RET_ERR_NOMEM = 632008,               /**< Out of memory */
  CNRT_RET_ERR_NODEV = 632009,               /**< No such device */
  CNRT_RET_ERR_IO = 632010,                  /**< I/O error */
  CNRT_RET_ERR_SYS = 632011,                 /**< System error */
  CNRT_RET_ERR_ACCES = 632012,               /**< Permission denied */
  CNRT_RET_ERR_FAULT = 632013,               /**< Bad address */
  CNRT_RET_ERR_BUSY = 632014,                /**< Device or resource busy */
  CNRT_RET_ERR_TIMEOUT = 632015,             /**< Time expired */
  CNRT_RET_ERR_EXIST = 632016,               /**< Resource or file already exists */
  CNRT_RET_ERR_NOSYS = 632017,               /**< Function not implemenmted */
  CNRT_RET_ERR_AGAIN = 632018,               /**< try again later */
  CNRT_RET_ERR_NORES = 632019,               /**< Out of resource */
  CNRT_RET_ERR_UNSUPPORTED = 632020,         /**< Unsupported operation */
  CNRT_RET_ERR_INVALID_POINTER = 632021,     /**< Invalid pointer */
  CNRT_RET_ERR_NO_EXIST = 632022,            /**< Resource or file doesn't exist */
  CNRT_RET_ERR_BROKEN   = 632023,            /**< Data transmission is broken */
  CNRT_RET_ERR_INIT     = 632024,            /**< Uninitialized */
  CNRT_RET_ERR_STREAM   = 632025,            /**< Failure on Stream */
  CNRT_RET_ERR_OUT_RANGE = 632026,           /**< Number out of range */
  CNRT_RET_ERR_MATH_OVERFLOW  = 632027,      /**< Math result not representable */
  CNRT_RET_ERR_FUNC_CALL  = 632028,          /**< Failure to call runtime functions */
  CNRT_RET_ERR_UNHANDLED  = 632029,          /**< Unhandled error */
  CNRT_RET_ERR_INVALID_TYPE = 632030,        /**< Invalid type */
  CNRT_RET_ERR_INVALID_OP = 632031,          /**< Invalid operation */
  CNRT_RET_ERR_MLU        = 632032,          /**< MLU error */
  CNRT_RET_ERR_ONCHIP_CORE = 632033,         /**< Onchip core error */
  CNRT_RET_ERR_EVENT      = 632034,          /**< Failure on event operation */
  CNRT_RET_ERR_RESHAPE = 632035,             /**< Failure on data reshape */
  CNRT_RET_ERR_MEMCPY = 632036,              /**< Failure on memory copy */
  CNRT_RET_ERR_ENCRYPT = 632037,             /**< Failure on encrypt */
  CNRT_RET_ERR_INVALID_DATADESC = 632038,    /**< Invalid data descriptor */
  CNRT_RET_ERR_UNKNOWN = 999991,             /**< Unknown error */
  CNRT_RET_ERR_MAX,                          /**< The last one */
} cnrtRet_t;

/**< Memory types available for allocator */
typedef enum {
  CNRT_MEMTYPE_DEFAULT = 0,/**< User space pagable memory */
  CNRT_MEMTYPE_LOCKED,     /**< Pinned memory */
  CNRT_MEMTYPE_MAPPED,     /**< Mapped on device's address space*/
  CNRT_MEMTYPE_WC,         /**< Write-combined memory*/
  CNRT_MEMTYPE_CR,         /**< CR BUS address*/
  CNRT_MEMTYPE_REG,        /**< Registered external memory */
  CNRT_MEMTYPE_DEV,        /**< Device memory */
} cnrtMemType_t;

/**< Malloc types available for cnrtMallocBufferEx */
typedef enum {
    CNRT_MALLOC_EX_PARALLEL_FRAMEBUFFER = 1
}cnrtMallocExType_t;


/**< Execution modes of tasks on MLU */
typedef enum {
  CNRT_FUNC_TYPE_BLOCK = 1,
  CNRT_FUNC_TYPE_BLOCK0 = CNRT_FUNC_TYPE_BLOCK,
  CNRT_FUNC_TYPE_BLOCK1 = CNRT_FUNC_TYPE_BLOCK0 + 1,
  CNRT_FUNC_TYPE_UNION1 = 4,
  CNRT_FUNC_TYPE_UNION2,
  CNRT_FUNC_TYPE_UNION4,
  CNRT_FUNC_TYPE_UNION8,
  CNRT_FUNC_TYPE_MUTABLE,
} cnrtFunctionType_t;

/**< Execution modes of tasks on MLU */
typedef enum {
  CNRT_CHANNEL_TYPE_NONE = -1,
  CNRT_CHANNEL_TYPE_0 = 0,
  CNRT_CHANNEL_TYPE_1,
  CNRT_CHANNEL_TYPE_2,
  CNRT_CHANNEL_TYPE_3,
} cnrtChannelType_t;


/**< Direction of data transmission */
typedef enum {
  CNRT_MEM_TRANS_DIR_HOST2DEV = 0,   /**< Host to Device*/
  CNRT_MEM_TRANS_DIR_DEV2DEV,        /**< Device to Device */
  CNRT_MEM_TRANS_DIR_DEV2HOST,       /**< Device to Host */
  CNRT_MEM_TRANS_DIR_HOST2HOST,      /**< Host to Host */
  CNRT_MEM_TRANS_DIR_NODIR,          /**< no direction for init */
} cnrtMemTransDir_t;

/**< Event attributes */
typedef enum {
  CNRT_EVENT_NORMAL = 0,              /**< Enables blocking operation */
  CNRT_EVENT_CROSS_PROCESS,           /**< Cross-processes event */
} cnrtEventAttr_t;

/**< Stream attributes */
typedef enum {
  CNRT_STREAM_ASYNC = 0,              /**< Enables nonblocking operation */
  CNRT_STREAM_SYNC,                   /**< Enables blocking operation */
} cnrtStreamAttr_t;


/**< Parameter for function call */
typedef struct {
  unsigned x;  /**< x aixs */
  unsigned y;  /**< y aixs */
  unsigned z;  /**< x aixs */
} cnrtDim3_t;

/**Parameter for init function call*/
typedef struct {
  bool *muta;              /*mutable option*/
  int *data_parallelism;   /*data parallelism*/
  u32_t *affinity;         /*affinity*/
  void *end;               /*end of struct*/
} cnrtInitFuncParam_t;

/**Parameter for invoke function call*/
typedef struct {
  int *data_parallelism;   /*data parallelism*/
  u32_t *affinity;         /*affinity*/
  void *end;               /*end of struct*/
} cnrtInvokeFuncParam_t;

/**< Version and revision  */
typedef struct {
  u32_t   cnrtPlatformVersion;     /**< platform version */
  u32_t   cnrtPlatformRevision;    /**< platform revision */
  u32_t   cnrtCoreVersion;         /**< core version */
  u32_t   cnrtCoreRevision;        /**< core revision */
} cnrtHardwareVersion_t;

/**< Device capabilities for compatibilities  */
typedef struct {
  u32_t   cnrtCapsNumberOfCore;           /**< Number of cores */
  u32_t   cnrtCapsMaxNumberOfStream;      /**< Maximum of stream that user can create */
} cnrtCap_t;

/**< Data type and data order*/
typedef enum cnrtDataType {
  CNRT_INVALID = 0x0,
  CNRT_FLOAT16 = 0x12,
  CNRT_FLOAT32 = 0x13,
  CNRT_FLOAT64 = 0x14,
  CNRT_INT8    = 0x21,
  CNRT_INT16   = 0x22,
  CNRT_INT32   = 0x23,
  CNRT_UINT8   = 0x31,
  CNRT_UINT32  = 0x33,
  CNRT_FIX8    = 0x41,
  CNRT_QUANT8  = 0x51,
  CNRT_BOOL    = 0x61,
} cnrtDataType_t;

typedef enum cnrtDimOrder {
  CNRT_NCHW = 0x0123,
  CNRT_NHWC = 0x0231,
  CNRT_HWCN = 0x2310,
} cnrtDimOrder_t;

typedef enum cnrtCoreVersion {
    CNRT_1H8  = 0,
    CNRT_1H16 = 1,
    CNRT_C10  = 3,
} cnrtCoreVersion_t;

/**< Execution priority of tasks on device */
typedef enum {
  CNRT_PRIORITY_TYPE_0 = 0,
  CNRT_PRIORITY_TYPE_1 = 1,
} cnrtPriorityType_t;

/**< Model and function */
struct cnrtModel;
typedef struct cnrtModel *cnrtModel_t;

struct cnrtFunction;
typedef struct cnrtFunction *cnrtFunction_t;

struct cnrtDataDesc;
typedef struct cnrtDataDesc *cnrtDataDesc_t, **cnrtDataDescArray_t;

struct cnrtFilterDesc;
typedef struct cnrtFilterDesc *cnrtFilterDesc_t, **cnrtFilterDescArray_t;

struct cnrtWeightDesc;
typedef struct cnrtWeightDesc *cnrtWeightDesc_t, **cnrtWeightDescArray_t;

struct cnrtStream;
typedef struct cnrtStream *cnrtStream_t;

struct cnrtEvent;
typedef struct cnrtEvent *cnrtEvent_t;

typedef u64_t cnrtDev_t;

/**< Compiler */
struct cnrtKernelParamsBuffer;
typedef struct cnrtKernelParamsBuffer *cnrtKernelParamsBuffer_t;

typedef struct cnrtKernelParamsBuffer {
  void *host_ptr;
  unsigned int max_param;
  unsigned int cur_param;

  // for plugin op
  // mark the position of kernel input/output/static ptr in param
  int *input_index;
  int num_input;
  int *output_index;
  int num_output;
  int *static_index;
  int num_static;
} *cnrtKernelParamsBuffer_t;


/************************************************************************
 * Function prototype declaration
 ************************************************************************/

/************************************************************************
 * Error handling
 ************************************************************************/

/**
 * @brief Return string pointer that describes
 *     the error code passed in the argument errCode.
 *
 * The function returns a read only string that is corresponding
 * to the argument @p errcode.
 *
 * @param  errCode[in] the error code was returned by previous function call.
 * @return a pointer that points to a constant string.
 */
extern __R_HOST
CNRT_DLL_API const char *cnrtGetErrorStr(cnrtRet_t errCode);

/**
 * @brief Get the error code set by any runtime calls.
 *     Its value is meaningful only when the return value indicating an error.
 *
 * @return error code of the last call of runtime functions.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetLastErr(void);

/*************************************************************************
 * Initialization and destroy
 *************************************************************************/

/**
 * @brief Initialize runtime environment in current process space.
 *
 * Initializes this API must be called before any other runtime API calls.
 *
 * @param  Flags[in] reserved for further use, pass 0 as well.
 * @return CNRT_RET_SUCCESS if success, otherwise with the error code.
 */

extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtInit(unsigned int Flags);

/**
 * @brief Destroy everything that allocated by runtime API calls.
 *
 * This API should be called after any other runtime API calls.
 *
 * @return void (None).
 */

extern __R_HOST
CNRT_DLL_API void cnrtDestroy(void);

/******************************************************************************
 * Version and revision
 ******************************************************************************/

/**
 * @brief Return the version of the cnrt software.
 *
 * Higher version usually offers more features provided by this library.
 *
 * @param  NULL.
 * @return unsigned int for version number.
 */

extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetVersion(unsigned int *ver);

/**
 * @brief Return the version of the MLU hardware.
 *
 * Higher version usually offers more features provided by this library.
 *
 * @param  ver[out] pointer to retrieve the version.
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */

extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetHardwareVersion(cnrtHardwareVersion_t *ver);

/******************************************************************************
 * Device managment
 ******************************************************************************/

/**
 * @brief Get the device handle by a given device ordinal.
 *
 *  The function returns the device handle given a specific device ordinal.
 *
 * @param  dev[out] pointer to retrieve the device handle.
 * @param  ordinal[in] the device ordinal to get the device handle.
 * @note   the ordinal should be in the range [0~cnrtGetDeviceCount() - 1].
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */

extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetDeviceHandle(cnrtDev_t *dev, int ordinal);

/**
 * @brief Set the device handle for current thread execution context.
 *
 *  It implies that any subsequent runtime API calls are for this device.
 *
 * @param  dev[in] the device handle.
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtSetCurrentDevice(cnrtDev_t dev);

/**
 * @brief Get the cnrtDevice handle from current thread execution context.
 *
 * The handle has been set by calling cnrtSetCurrentDevice().
 *
 * @param  dev[out] pointer to retrieve the device handle.
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetCurrentDevice(cnrtDev_t *dev);

/**
 * @brief Get capability of a device which is specified by device handle.
 *
 *  The function returns the device capabilities.
 *
 * @param  cap[out] pointer to retrieve the device capability.
 * @param  dev[in] the device handle to get the device capability.
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetDeviceCapability(cnrtCap_t *cap, cnrtDev_t dev);

/**
 * @brief Get the number of MLU devices in the system.
 *
 * @param  devnum[out] pointer to retrieve the number of devices.
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetDeviceCount(unsigned *devnum);

/**
 * @brief Get the total memory size in MByte.
 *
 * @param  sz[out] pointer to retrieve the memory amount on device.
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetDeviceMemorySize(u64_t *sz);

/**
 * @brief  Wait for the device to complete precedent tasks.
 *
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtSyncDevice(void);

/******************************************************************************
 * Stream managment
 ******************************************************************************/

/**
 * @brief Create a new stream after calling this funcation,
 *        it works in asynchronous mode by default.
 *
 * @param pStream[out] pointer to retrieve the new created Stream handle.
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtCreateStream(cnrtStream_t *pStream);

/**
 * @brief Create a new stream after calling this funcation,
 *        it works in asynchronous mode by default.
 *
 * @note  Task in a higher priority stream might preempt the tasks running in a low priority stream.
 * @param pStream[out] pointer to retrieve the newly created stream handle.
 * @param pri[in]      priority of the stream is being created. it should be in range [CNRT_STREAM_PRIORITY_LOW ~ CNRT_HIGHEST_PRIORITY_HIGH].
 *                     Small numbers represent higher priorities. CNRT_STREAM_PRIORITY_MID is the default priority.
 * @param attr[in]     Behaviors of the stream, simply passing 0 leads to the default behavior.
 *                     @see cnrtStreamAttr_t
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtCreateStreamEx(cnrtStream_t *pStream, int pri, cnrtStreamAttr_t attr);

/**
 * @brief Destroy a stream created by calling cnrtCreateStream or cnrtCreateStreamEx.
 *
 * @param stream[in]   stream handle created by calling cnrtCreateStream or cnrtCreateStreamEx.
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtDestroyStream(cnrtStream_t stream);

/**
 * @brief Function should be blocked until all precedent tasks in the stream are completed.
 *
 * @param stream[in]   stream handle created by calling cnrtCreateStream or cnrtCreateStreamEx.
 * @return CNRT_RET_SUCCESS if success, otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtSyncStream(cnrtStream_t stream);

/**
 * @brief Function to query stream status on the completion of all precedent tasks.
 *
 * @param stream[in]   stream handle created by calling cnrtCreateStream or cnrtCreateStreamEx.
 * @return CNRT_RET_SUCCESS if all precedent tasks in the stream are completed,
 *         CNRT_RET_ERR_BUSY if the procedure is still in progress,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtQueryStream(cnrtStream_t stream);

/*********************************************************************************
 * Event/Signaling
 *********************************************************************************/

/**
 * @brief Create an event corresponding to the current device.
 *
 * @param event[out]  point to an event handle to retrieve newly created event.
 * @param attr[in]    specify the behaviors of the newly created event.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtCreateEvent(cnrtEvent_t *event);

/**
 * @brief Destroy an event that was created by calling cnrtCreateEvent.
 *
 * @param event[out]   event handle to be destroyed.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtDestroyEvent(cnrtEvent_t *event);

/**
 * @brief Wait until the specified event object is in the signaled state or exceeds the time-out interval.
 *
 * @param event[in]   event handle created by calling cnrtCreateEvent.
 * @return CNRT_RET_SUCCESS if success.
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtWaitEvent(cnrtEvent_t event);

/**
 * @brief Query the status of placed notification, which has been placed to stream recently by calling
 *        cnrtPlaceNotifyEvent.
 *
 * @param event[in]   event handle created by calling cnrtCreateEvent.
 *
 * @return CNRT_RET_SUCCESS if notification instruction has been executed,
 *         CNRT_RET_ERR_BUSY if the preceding tasks is still in progress,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtQueryEvent(cnrtEvent_t event);

/**
 * @brief Place an event in specified stream. This function will not block the CPU thread.
 *        All computation tasks submitted to the stream will wait until event reports
 *        completion before starting execution.
 *
 * @param event[in] signal handle created by calling cnrtCreateEvent.
 * @param stream[in] stream handle created by calling cnrtCreateStream or cnrtCreateStreamEx.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtPlaceEvent(cnrtEvent_t event, cnrtStream_t stream);

/**
 * @brief Make the specified stream wait for an event. This function is designed for
 *        cross stream synchronization.
 *
 * @param event[in] signal handle created by calling cnrtCreateEvent.
 * @param stream[in] stream handle created by calling cnrtCreateStream or cnrtCreateStreamEx.
 * @param flag[in] flags control operation.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtStreamWaitEvent(cnrtEvent_t event, cnrtStream_t
      stream, unsigned int flag);
/**
 * @brief Get elapsed time of two events.
 *
 * @param event[in] signal handle created by calling cnrtCreateEvent.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtEventElapsedTime(cnrtEvent_t event_start,
      cnrtEvent_t event_end, float* ptv);

/**< Compiler */
/*********************************************************************************
 * Execution control
 *********************************************************************************/


/**
 * @brief Get a parameter buffer for cnrtInvokeKernel.
 *
 * @param params[in] pointer to a param buffer
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetKernelParamsBuffer(cnrtKernelParamsBuffer_t *params);


/**
 * @brief Copy Parambuffer from src_params_buf to dst_params_buf
 *
 * @param dst_params_buf[in] pointer to an allocated param buffer
 * @param src_params_buf[in] pointer to an allocated param buffer
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern  __R_HOST
CNRT_DLL_API cnrtRet_t cnrtCopyKernelParamsBuffer(
    cnrtKernelParamsBuffer_t dst_params_buf,
    cnrtKernelParamsBuffer_t src_params_buf);

/**
 * @brief Add a parameter to a specific parameter buffer.
 *
 * @param params[in] destination parameter buffer
 * @param data[in] pointer to host memory
 * @param nBytes[in] size in bytes
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtKernelParamsBufferAddParam(cnrtKernelParamsBuffer_t
      params, void *data, size_t nBytes);

/**
 * @brief Add a InputPtr place holder to a specific parameter buffer.
 *
 * @param params[in] destination parameter buffer
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t
cnrtKernelParamsBufferMarkInput(cnrtKernelParamsBuffer_t params);

/**
 * @brief Add a OutputPtr place holder to a specific parameter buffer.
 *
 * @param params[in] destination parameter buffer
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t
cnrtKernelParamsBufferMarkOutput(cnrtKernelParamsBuffer_t params);

/**
 * @brief Add a StaticPtr place holder to a specific parameter buffer.
 *
 * @param params[in] destination parameter buffer
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t
cnrtKernelParamsBufferMarkStatic(cnrtKernelParamsBuffer_t params);

/**
 * @brief Destroy a parameter buffer returned by cnrtGetKernelParamsBuffer.
 *
 * @param params[in] pointer to a param buffer
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtDestroyKernelParamsBuffer(cnrtKernelParamsBuffer_t params);

/**
 * @brief Invoke a kernel written in Bang with given params on MLU.
 *
 * @param function[in] point to the MLU function.
 * @param dim[in]      how many grid dimentions.
 * @param params[in]   point to arguments.
 * @param c[in]        function type. @see cnrtFunctionType_t.
 * @param stream[in]   stream associated to the function call.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtInvokeKernel(const void *function, cnrtDim3_t dim,
      cnrtKernelParamsBuffer_t params, cnrtFunctionType_t c, cnrtStream_t stream);

/*********************************************************************************
 * Model load and Function call
 *********************************************************************************/

/**
 * @brief Load a model from a given model file.
 *
 * @param model[out] point to a cnrtModel_t.
 * @param fname[in]  file name of a cambricon model.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtLoadModel(cnrtModel_t *model, const char *fname);

/**
 * @brief Load a model from memory
 *
 * @param model[out] point to a cnrtModel_t.
 * @param ptr[in] memory ptr.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtLoadModelFromMem(cnrtModel_t *model, char* ptr);

/**
 * @brief Unload a model.
 *
 * @param model[in] point to a cnrtModel_t.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtUnloadModel(cnrtModel_t model);

/**
 * @brief Get function number of a given model
 *
 * @param model[in] pointer of a cnrt model
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetFunctionNumber(cnrtModel_t model, int *func_num);

/**
 * @brief Extract the symbol from the given model if symbol exists,
 *        otherwise error code will be returned.
 *
 * @param function[out] point to a cnrtFunction_t.
 * @param model[in]  point to a loaded model.
 * @param symbol[in] symbol name.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtExtractFunction(cnrtFunction_t *function,
      cnrtModel_t model, const char *symbol);

/**
 * @brief Create a mlu function.
 * @param function[in] pointer of cnrtFunction_t.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtCreateFunction(cnrtFunction_t *function);

/**
 * @brief Destroy a function.
 *
 * @param function[in] point to a function generated by cnrtExtractFunction.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtDestroyFunction(cnrtFunction_t function);

/**
 * @brief Initialize instrucion and runtime data of a function on current MLU device.
 *
 *        cnrtInitFunctionMemory has two modes, distinguished by func_type.
 *        The first mode is func_type == CNRT_FUNC_TYPE_MUTABLE.
 *        The second mode is func_type != CNRT_FUNC_TYPE_MUTABLE.
 *
 *        The first mode is more flexble.
 *        Under this mode, the same function can be invoked by different
 *        cnrtInvokeFunction with different parallelism simultaneously.
 *        For example, you can write the following code:
 *        cnrtInitFunctionMemory(function, CNRT_FUNC_TYPE_MUTABLE);
 *        cnrtInvokeFunction(function, ..., CNRT_FUNC_TYPE_UNION2, ...);
 *        cnrtInvokeFunction(function, ..., CNRT_FUNC_TYPE_BLOCK, ...);
 *
 *        The second mode is more efficient.
 *        Under this mode, the same function can also be invoked by different
 *        cnrtInvokeFunction. But the parallelism is limited. It should be
 *        the same as func_type.
 *        For example, you can write the following code:
 *        cnrtInitFunctionMemory(function, CNRT_FUNC_TYPE_UNION2);
 *        cnrtInvokeFunction(function, ..., CNRT_FUNC_TYPE_UNION2, ...);
 *        cnrtInvokeFunction(function, ..., CNRT_FUNC_TYPE_UNION2, ...);
 *
 *        notice: cnrtInitFunctionMemory should be called before
 *        cnrtInvokeFunction and after cnrtSetCurrentDevice.
 *
 * @param function[in] pointer of cnrtFunction_t.
 * @param func_type[in] parallelism of function.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtInitFunctionMemory(cnrtFunction_t function,
    cnrtFunctionType_t func_type);

/**
* @brief Initialize instrucion and runtime data of a function on current MLU device.
*
*        cnrtInitFunctionMemory has two modes, distinguished by affinity.
*        The first mode is affinity == CNRT_FUNC_TYPE_MUTABLE.
*        The second mode is affinity != CNRT_FUNC_TYPE_MUTABLE.
*
*        The first mode is more flexble.
*        Under this mode, the same function can be invoked by different
*        cnrtInvokeFunction with different parallelism simultaneously.
*        For example, you can write the following code:
*        data_parallelism int the cnrtInitFunctionMemory_V2 is just initialization.
*        cnrtInitFuncParam_t init_func_param;
*        bool muta = true;
*        init_func_param.muta = &muta;
*        int data_parallelism = 1;
*        init_func_param.data_parallelism = &data_parallelism;
*        init_func_param.end = CNRT_PARAM_END;
*        cnrtInitFunctionMemory_V2(function, &init_func_param);
*
*        data_parallelism in the cnrtInvokeFunction is mutalble.
*        cnrtInvokeFuncParam_t invoke_func_param;
*        int data_parallelism = 2;
*        invoke_func_param.data_parallelism = &data_parallelism;
*        invoke_func_param.end = CNRT_PARAM_END;
*        cnrtFunctionType_t func_type = (cnrtFunctionType_t)0;
*        cnrtInvokeFunction(function, ..., (void *)&invoke_func_param);
*
*        The second mode is more efficient.
*        Under this mode, the same function can also be invoked by different
*        cnrtInvokeFunction. But the parallelism is limited. It should be
*        the same as affinity.
*        For example, you can write the following code:
*        data_parallelism in the cnrtInvokeFunction is not mutalble.
*        cnrtInitFuncParam_t init_func_param;
*        bool muta = false;
*        init_func_param.muta = &muta;
*        int data_parallelism = 1;
*        init_func_param.data_parallelism = &data_parallelism;
*        init_func_param.end = CNRT_PARAM_END;
*        cnrtInitFunctionMemory_V2(function, &init_func_param);
*
*        data_parallelism in the cnrtInvokeFunction should be same as the
*        data_parallelism int the cnrtInitFunctionMemory_V2.
*        cnrtInvokeFuncParam_t invoke_func_param;
*        int data_parallelism = 1;
*        invoke_func_param.data_parallelism = &data_parallelism;
*        invoke_func_param.end = CNRT_PARAM_END;
*        cnrtFunctionType_t func_type = (cnrtFunctionType_t)0;
*        cnrtInvokeFunction(function, ..., (void *)&invoke_func_param);
*
*        notice: cnrtInitFunctionMemory should be called before
*        cnrtInvokeFunction and after cnrtSetCurrentDevice.
*
* @param function[in] pointer of cnrtFunction_t.
* @param param[in] pointer of cnrtInitfuncParam_t.
* @return CNRT_RET_SUCCESS if success,
*         otherwise the error code is returned.
*/
extern __R_HOST
CNRT_DLL_API cnrtRet_t
    cnrtInitFunctionMemory_V2(cnrtFunction_t function, cnrtInitFuncParam_t *param);

/**
 * @brief Invoke a function with given params on MLU.
 * @param function[in] point to the MLU function.
 * @param dim[in]      how many grid dimentions.
 * @param params[in]   point to arguments.
 * @param func_type[in]        function type. @see cnrtFunctionType_t.
 * @param stream[in]   stream associated to the function call.
 * @param cnrtInvokeFuncParam[in]     private pointer that is for cnrtInvokeFuncParam_t.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtInvokeFunction(cnrtFunction_t function, cnrtDim3_t dim, void **params,
    cnrtFunctionType_t func_type, cnrtStream_t stream, void *cnrtInvokeFuncParam);

/**
 * @brief Generate a copy of source MLU function. src and dst function share the
 *        same kernel on host, but they have different device space, so model
 *        data(include instruction) is doubled on device.
 *
 * @param src[in] Pointer to a source MLU function
 * @param dst[out] Pointer to a destination MLU function pointer
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtCopyFunction(cnrtFunction_t *dst, cnrtFunction_t src);

/*********************************************************************************
 * Memory management
 *********************************************************************************/

/**
 * @brief Allocate nByte bytes and place a pointer to pointer
 *        in pPtr to the allocated host memory. If nBytes is 0, then
 *        cnrtMallocHost returns either NULL, or a unique pointer value
 *        that can later be passed to cnrtFreeHost.
 *
 * @param pPtr[out]  a pointer to pointer for retrieving allocated host memory.
 * @param nBytes[in] number bytes of memory to be allocated.
 * @param type[in]   memory type to be allocated,
 *                   @see CNRT_HOST_MEMORY_TYPE_LOCK and CNRT_HOST_MEMORY_TYPE_MAPPED.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMallocHost(void **pPtr, size_t nBytes, cnrtMemType_t type);

/**
 * @brief Free the memory space pointed by ptr, which must be
 *        returned by a previous call of cnrtMallocHost.
 *
 * @param ptr[in]  point to the address of memory to be free.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtFreeHost(void *ptr);

/**
 * @brief Lock an existing unlocked host memory and make it accessable to MLU.
 *
 * @param ptr[in]    a pointer pointed to an existing unlocked host memory.
 * @param nBytes[in] number byte of memory to be locked and mapped.
 * @param type[in]   type of the memory to be locked.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtLockHostMem(void *ptr, size_t nBytes, cnrtMemType_t type);

/**
 * @brief Unlock an existing host memory that has
 *        been locked by calling cnrtLockHostMem.
 *
 * @param ptr[in]  a pointer pointed to a locked memory.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtUnlockHostMem(void *ptr);

/**
 * @brief Allocate memory on MLU device.
 *
 * @param pPtr[out] a pointer to pointer for retrieving allocated device memory.
 * @param nBytes[in] allocate size.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMalloc(void **pPtr, size_t nBytes);

/**
 * @brief Allocate continuous memory for muti-way data on MLU device.
 *        This API should be used under data parallel mode.
 *        Data size of each way will be aligned automatically for sake of
 *        high memory access performance. So the truely allocate size is
 *        align(nBytes) * data_parallelism.
 *
 * @param pPtr[out] a pointer to pointer for retrieving allocated device memory.
 * @param nBytes[in] allocate size.
 * @param data_parallelism[in] data parallelism
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMallocBatch(void **pPtr, size_t nBytes, int data_parallelism);

/**
 * @brief Allocate memory on MLU device.
 *        Compared with cnrtMalloc, cnrtMallocByDesc use cnrtDataDesc_t
 *        object to determine the allocate size.
 *
 * @param pPtr[out] point to allocated memory.
 * @param dataDesc[in] data descriptor.
 * @return CNRT_RET_SUCCESS if success.
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMallocByDesc(void **pPtr, cnrtDataDesc_t dataDesc);

/**
 * @brief Allocate continuous memory for muti-way data on MLU device.
 *        Compared with cnrtMallocBatch, cnrtMallocBatchByDesc use
 *        cnrtDataDesc_t object to determine the allocate size.
 *
 * @param pPtr[out] point to allocated memory.
 * @param dataDesc[in] data descriptor.
 * @param data_parallelism[in] data parallelism
 * @return CNRT_RET_SUCCESS if success.
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t
    cnrtMallocBatchByDesc(void **pPtr, cnrtDataDesc_t dataDesc, int data_parallelism);

/**
 * @brief Allocate multiple addresses for multiple data objects on MLU device.
 *        Multiple addresses and data descriptors is present in array format.
 *        This API is a reinforced version of cnrtMallocByDesc. You can call
 *        cnrtMallocByDesc more than once to realize the same function.
 *
 * @param pPtrArray[out] point to the allocated memory array.
 * @param dataDescArray[in] data descriptor array.
 * @param lentgh[in] array length.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMallocByDescArray(void ***pPtrArray,
    cnrtDataDescArray_t dataDescArray, int length);

/**
 * @brief Allocate multiple addresses for multiple data objects
 *        of multiple way on MLU device. This API is a mult-way
 *        version of cnrtMallocByDescArray.
 *
 * @param pPtrArray[out] point to the allocated memory array.
 * @param dataDescArray[in] data descriptor array.
 * @param lentgh[in] array length.
 * @param data_parallelism[in] way size of data.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMallocBatchByDescArray(void ***pPtrArray,
      cnrtDataDescArray_t dataDescArray, int length, int data_parallelism);

/**
 * @brief Allocate memory on MLU device. For P2P.
 *
 * @param pPtr[out] a pointer to pointer for retrieving allocated device memory.
 * @param nBytes[in] allocate size.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMallocFrameBuffer(void **pPtr, size_t nBytes);

/**
 * @brief Allocate memory on MLU device, for extension
 *
 * @param pPtr[out] a pointer to pointer for retrieving allocated device memory.
 * @param param[in] parameter buffer allocated by cnrtAllocParam
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMallocBufferEx(void **pPtr, void *param);

/**
 * @brief Deallocate MLU device Memory.
 *
 * @param ptr[in] point to the memory to be free.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtFree(void *ptr);

/**
 * @brief Deallocate MLU multiple device memory addresses allocated
 *        by cnrtMallocBatchByDescArray, cnrtMallocByDescArray.
 *
 * @param ptr[in] a pointer array.
 * @param length[in] array length.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtFreeArray(void **ptr, int length);

/**
 * @brief Copy data from src address to dst address. The copy direction
 *        is specified by input parameter dir. The copy operation is
 *        always performed on current device which is set by cnrtSetCurrentDevice.
 *
 * @param dest[in] destination address.
 * @param src[in] source address.
 * @param nBytes[in] number of bytes to be copied.
 * @param dir[in] direction of transfer.
 *                @see  CNRT_MEM_TRANS_DIR_HOST2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2HOST,
 *                      CNRT_MEM_TRANS_DIR_HOST2HOST,
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMemcpy(void *dest, void *src, size_t nBytes, cnrtMemTransDir_t dir);

/**
 * @brief Copy multi-way data from src address to dst address.
 *        The device address should be allocated by cnrtMallocBatch.
 *        The host address should contain data_parallelism number of data arranged
 *        continuously. This API should be used under data parallel mode.
 *        More infomation about device address @see cnrtMallocBatch.
 *
 * @param dest[in] destination address.
 * @param src[in] source address.
 * @param nBytes[in] size of single way data.
 * @param data_parallelism[in] data parallelism.
 * @param dir[in] direction of transfer.
 *                @see  CNRT_MEM_TRANS_DIR_HOST2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2HOST,
 *                      CNRT_MEM_TRANS_DIR_HOST2HOST,
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMemcpyBatch(void *dest, void *src, size_t nBytes,
      int data_parallelism, cnrtMemTransDir_t dir);

/**
 * @brief Copy data from src address to dst address.
 *        Compared with cnrtMemcpy, cnrtMemcpyByDesc receives data descriptor as
 *        a input parameter. Because we need to carry out data layout optimization
 *        for MLU device. This API is typically used in image process situation.
 *
 * @param dest[in] destination address.
 * @param src[in] source address.
 * @param dataDesc[in] data descriptor.
 * @param dir[in] direction of transfer.
 *                @see  CNRT_MEM_TRANS_DIR_HOST2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2HOST,
 *                      CNRT_MEM_TRANS_DIR_HOST2HOST,
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMemcpyByDesc(void *dest, void *src,
      cnrtDataDesc_t dataDesc, cnrtMemTransDir_t dir);

/**
 * @brief Copy multi-way data from src address to dst address.
 *        This is a multi-way version of cnrtMemcpyByDesc.
 *        The host address should contain data_parallelism number of data arranged
 *        continuously.
 *        The device address should be allocated by cnrtMallocBatchByDesc.
 *        To get more infomation about multi-way, @see cnrtMallocBatchByDesc.
 *
 * @param dest[in] destination address.
 * @param src[in] source address.
 * @param dataDesc[in] data descriptor.
 * @param data_parallelism[in] data parallelism.
 * @param dir[in] direction of transfer.
 *                @see  CNRT_MEM_TRANS_DIR_HOST2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2HOST,
 *                      CNRT_MEM_TRANS_DIR_HOST2HOST,
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMemcpyBatchByDesc(void *dest, void *src,
      cnrtDataDesc_t dataDesc, int data_parallelism, cnrtMemTransDir_t dir);

/**
 * @brief Copy multiple data objects from src addresses to dst addresses.
 *        Multiple addresses and data descriptors is present in array format.
 *        This API is a reinforced version of cnrtMemcpyByDesc. You can call
 *        cnrtMemcpyByDesc more than once to realize the same function.
 *
 * @param destArray[in] pointer to destination address array.
 * @param srcArray[in] pointer to source address array.
 * @param dataDescArray[in] data descriptor array.
 * @param length[in] array length.
 * @param dir[in] direction of transfer.
 *                @see  CNRT_MEM_TRANS_DIR_HOST2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2HOST,
 *                      CNRT_MEM_TRANS_DIR_HOST2HOST,
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMemcpyByDescArray(void **destArray, void **srcArray,
      cnrtDataDescArray_t dataDescArray, int length, cnrtMemTransDir_t dir);

/**
 * @brief Copy multiple data objects of multi-way from src
 *        addresses to dst addresses.
 *        This API is the multi-way of cnrtMemcpyByDescArray.
 *
 * @param destArray[in] pointer to destination address array.
 * @param srcArray[in] pointer to source address array.
 * @param dataDescArray[in] data descriptor array.
 * @param length[in] array length.
 * @param data_parallelism[in] data parallelism.
 * @param dir[in] direction of transfer.
 *                @see  CNRT_MEM_TRANS_DIR_HOST2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2DEV,
 *                      CNRT_MEM_TRANS_DIR_DEV2HOST,
 *                      CNRT_MEM_TRANS_DIR_HOST2HOST,
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMemcpyBatchByDescArray(void **destArray, void **srcArray,
      cnrtDataDescArray_t dataDescArray, int length,
      int data_parallelism, cnrtMemTransDir_t dir);

/**
 * @brief Fill the nBytes of the device memory space
 *        pointed by devPtr with the constant value c.
 *
 * @param dest[in] device memory address.
 * @param c[in] value to be filled.
 * @param nBytes[in] number of bytes to be filled.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtMemset(void *devPtr, int c, size_t nBytes);

/**
 * @brief set mlu stack space memory to stack_size(MB).
 *
 * @param stack_size[in] the size of mlu stack space memory will be set.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise CNRT_RET_ERR_MLU is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtSetStackMem(unsigned int stack_size);

/**
 * @brief get mlu stack space memory to stack_size(MB).
 *
 * @param stack_size_ptr[out] the size of mlu stack space memory will be get.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise CNRT_RET_ERR_MLU is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetStackMem(unsigned int *stack_size_ptr);

/**
 * @brief Map an allocated buffer from the host side to device's address space.
 *
 * @param pDevPtr[out] pointer to receive the mapped address.
 * @param hostPtr[in]  pointer to host side memory.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetHostAddrMappedOnDevice(void **pDevPtr, void *hostPtr);

/**
 * @brief Set a new permission of the device memory.
 *
 * @param devPtr[in] pointer to the device memory.
 * @param nBytes[in] size of that memory in byte.
 * @param perms[in] permissions @see CNRT_MEMORY_PERMS_READ
 *                              @see CNRT_MEMORY_PERMS_WRITE
 *                              @see CNRT_MEMORY_PERMS_EXEC
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtSetMemoryPerms(void *devPtr, size_t nBytes, unsigned perms);

/**
 * @brief cnrt get max memory used
 * @param function[in] point to the MLU function.
 * @brief cnrt get max memory used
 * @param function[in] point to the model.
 * @param function[in] return value.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetFunctionMemUsed(cnrtFunction_t function, int64_t* functionMem);

/**
 * @brief cnrt get max memory used
 * @param function[in] point to the model.
 * @param function[in] return value.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetModelMemUsed(cnrtModel_t model, int64_t* totalMem);

/*********************************************************************************
 * Channel control
 *********************************************************************************/

/**
 * @brief Set memory and computation channel on current MLU device. Once
 *        a channel is configured, all memory allocation(eg. cnrtMalloc)
 *        will be performed on this channel. And all function invokation
 *        (cnrtInvokeFunction) will be performed on this channel too.
 *        Attention: The above policy only take effect when model parallelism
 *        is 1.
 *        This function is base on CPU thread context. So it's action scope
 *        is within current CPU thread. This function should be called after
 *        cnrtSetCurrentDevice;
 *
 * @param cnrtChannelType_t[in] channel.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtSetCurrentChannel(cnrtChannelType_t channel);


 /**
 * @brief Get current channel of current CPU thread.
 *
 * @param channel[out] Pointer to channel.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetCurrentChannel(cnrtChannelType_t *channel);

/*********************************************************************************
 * Data descriptor
 *********************************************************************************/

/**
 * @brief Get a series of input data descriptors from a given function.
 *
 * @param descArray[out] point to Data descriptor array.
 * @param num[out] length of the data descriptor array.
 * @param function[in] MLU function pointer.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetInputDataDesc(cnrtDataDescArray_t *descArray,
    int *num, cnrtFunction_t function);

/**
 * @brief Get a series of input data descriptors from a given function.
 *
 * @param descArray[out] point to the data descriptor array.
 * @param num[out] length of the data descriptor array.
 * @param function[in] MLU function pointer.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetOutputDataDesc(cnrtDataDescArray_t *descArray,
    int *num, cnrtFunction_t function);

/**
 * @brief Set data layout(eg. type, dim order) on the host according to the data descriptor.
 *
 * @param desc[in] point to data descriptor.
 * @param dtype[in] host data type. @see cnrtDataDesc_t.
 * @param order[in] host data dim order. @see cnrtDimOrder_t.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtSetHostDataLayout(cnrtDataDesc_t desc,
    cnrtDataType_t dtype, cnrtDimOrder_t order);

/**
 * @brief get a DataDesc's n, c, h ,w.
 *
 * @param desc[in] point to the data descriptor pointer.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetDataShape(cnrtDataDesc_t desc,
    unsigned * n, unsigned * c, unsigned * h, unsigned * w);

/**
 * @brief Get host data count (e.g. for tensor with dim nchw, the count is n*c*h*w).
 *
 * @param count[out] host data count.
 * @param desc[in] point to the data descriptor.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetHostDataCount(cnrtDataDesc_t desc, int *count);

/**
 * @brief Close reshape of one cnrtDataDesc_t,
 *        in order to reduce CPU run time.
 * @param desc[in] point to the data descriptor.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtCloseReshapeOfOneDataDesc(cnrtDataDesc_t desc);

/**
 * @brief Set weight layout(eg. type, dim order) on the host according to the weight descriptor.
 *
 * @param desc[out] point to weight descriptor.
 * @param dtype[in] host weight type. @see cnrtDataDesc_t.
 * @param order[in] host weight dim order. @see cnrtDimOrder_t.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtSetHostWeightLayout(cnrtWeightDesc_t desc, cnrtDataType_t dtype,
    cnrtDimOrder_t order);

/**
 * @brief Get host weight count (e.g. dim nchw, count is n*c*h*w).
 *
 * @param count[out] host weight count.
 * @param desc[in]   point to the weight descriptor.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetHostWeightCount(cnrtWeightDesc_t desc, int *count);

/**
 * @brief Get a weight descriptors from a given function by name.
 *
 * @param weightDesc[out] point to weight descriptor.
 * @param update_name[in] name of weightdesc that you want to update.
 * @param function[in]    MLU function pointer.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetWeightDesc(cnrtWeightDesc_t* weightDesc, char* update_name,
    cnrtFunction_t function);

/**
 * @brief update weight data in given function.

 * @param weightDesc[in]   the weight descriptor that you want to update.
 * @param weightCpuPtr[in] the weight data that you want to update
 * @param function[in]     NLU function pointer.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtUpdateWeightData(cnrtWeightDesc_t weightDesc, void* weightCpuPtr,
    cnrtFunction_t function);

/**
 * @brief Get a series of weight descriptors from a given function by name array.
 *
 * @param descArray[out] point to weight descriptor array.
 * @param name_num[in]   length of the weight descriptor array.
 * @param name_array[in] a array of name you want to update
 * @param function[in]   MLU function pointer.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetWeightDescArray(cnrtWeightDescArray_t descArray,
    int name_num, char* name_array[], cnrtFunction_t function);

/**
 * @brief update a series of weight data in given function.
 *
 * @param descArray[in]  a series of weight descriptors that you want to update.
 * @param weightCpuPtrS[in] the weight data that you want to update.
 * @param update_num[in] the number of weight data array that you want to update.
 * @param function[in]   MLU function pointer.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtUpdateWeightDataArray(cnrtWeightDescArray_t descArray,
    void** weightCpuPtrS, int update_num, cnrtFunction_t function);

/**
 * @ brief  Write twins file according to offline file
 * @ param  offlineFileName [in] offline model file name.
 * @ return CNRT_RET_SUCCESS if success,
 *          otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtWriteTwinsFileAccordingToOfflineFile(const char* offlineFileName);

/**
 * @brief  Query model's core version, 1H8 or 1H16.
 *
 * @param model[in] point to a loaded model.
 * @param coreversion[out] pointer to model's core version.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtQueryCoreVersion(cnrtModel_t model, cnrtCoreVersion_t *coreversion);

/**
 * @brief  Query model's core number.
 *
 * @param model[in] point to a loaded model.
 * @param core_num[out] pointer to model's core number.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtQueryCoreNumber(cnrtModel_t model, int *core_num);

/**
 * @brief  Get actual size of model in offline file.
 *
 * @param fname[in] file name of a cambricon model.
 * @param size[out] pointer to model's actual size.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetModelSize(const char *fname, int *size);

/**
 * @brief  Query model's parallelism, which means the core number
 * involved to compute this model.
 *
 * @param model[in] point to a loaded model.
 * @param parallelism[out] pointer to model's parallelism.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtQueryModelParallelism(cnrtModel_t model, int *model_parallelism);

/**
 * @brief  Query model's stack size, which is the biggest stack size(MB)
 * in all the kernels in the model.
 *
 * @param model[in] point to a loaded model.
 * @param size[out] pointer to the stack size.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtQueryModelStackSize(cnrtModel_t model, uint64_t *stack_size);
/**
 *@brief Convert a float/double to float16, store it at specific position (*f16 = (f16)d)
 *
 *@param d[in] number to convert
 *@param f16[out] place to store
 *@return error code of the last call of runtime functions
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtConvertDoubleToHalf(uint16_t *f16, double d);

extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtConvertFloatToHalf(uint16_t *f16, float d);

/**
 *@brief Convert a float16 to float/double, store it at specific position (*d = (float/double)(f16))
 *
 *@param f16[in] number to convert
 *@param d[out] place to store
 *@return error code of the last call of runtime functions
 */

extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtConvertHalfToDouble(double *d, uint16_t f16);

extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtConvertHalfToFloat(float *d, uint16_t f16);

extern __R_HOST
CNRT_DLL_API int cnrtDataTypeSize(cnrtDataType_t dt);

/*********************************************************************************
 * Task Priority control
 *********************************************************************************/

/**
 * @brief Set the priority of task on  device. Once This function is base on
 *        CPU thread context. So it's action scope is within current CPU thread.
 *        This function should be called after cnrtSetCurrentDevice;
 *
 * @param cnrtPriorityType_t[in] priority.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtSetTaskPriority(cnrtPriorityType_t priority);

/**
 * @brief Get current priority of task.
 *
 * @param priority[out] Pointer to channel.
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetTaskPriority(cnrtPriorityType_t *priority);

/**
 * @brief Reshape filter data from src address to dst address.
 *        The origin src data layout is src[N][H][W][C]
 *
 * @param dst[out] destination address.
 * @param src[in] source address.
 * @param n/h/w/c[in] the origin data layout.
 * @param type[in] the data type of dst[out] and src[in].
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtFilterReshape(void *dst, void *src, int n, int h,
                                         int w, int c, cnrtDataType_t type);

/**
 * @brief Reshape data from src address to dst address.
 *        only between NHWC and NCHW
 *
 * @param dst[out] destination address.
 * @param src[in] source address.
 * @param n/h/w/c[in] the origin data layout.
 * @param type[in] the data type of dst[out] and src[in].
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtReshapeNCHWToNHWC(void *dst, void *src,
                  int n, int h, int w, int c, cnrtDataType_t type);

extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtReshapeNHWCToNCHW(void *dst, void *src,
                      int n, int h, int w, int c, cnrtDataType_t type);

/****************************************************************************
 * Generic parameters handling
 ***************************************************************************/

/**
 * @brief Allocate a cnrt parameter context buffer
 *
 * @param pParam[out] pointer to the parameter context buffer pointer
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtAllocParam(void **pParam);

/**
 * @brief Destory a cnrt parameter context buffer
 *
 * @param param[in] the parameter context buffer pointer
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtDestoryParam(void *param);

/**
 * @brief Add one parameter to parameter context buffer
 *
 * @param param[in] the parameter context buffer pointer
 * @param name[in] name of the parameter
 * @param len[in] length of the parameter
 * @param data[in] pointer to the parameter
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtAddParam(void *param, char *name, int len, void *data);

/**
 * @brief Get one parameter from parameter context buffer
 *
 * @param param[in] the parameter context buffer pointer
 * @param name[in] name of the parameter
 * @param out[out] result buffer
 * @param outlen[in] result buffer length
 *
 *
 * @return CNRT_RET_SUCCESS if success,
 *         CNRT_RET_ERR_MEMCPY if parameter actual length is larger than result buffer length
 *         CNRT_RET_ERR_NO_EXIST if "name" is not found in param context
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtGetParam(void *param, char *name, void *out, int outlen);

/**
 * @brief Malloc instrucion addr on device and copy instruction to device.
 *        update barrier of kernel.
 *        If copy complete and return CNRT_RET_SUCCESS.
 * @param function[in] pointer of mlu function.
 * @param inst_addr[out] get the pointer of mlu instrucion memory.
 * @param param[in] pointer of cnrtInitFuncParam_t.
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtInitFunctionInstMemory(cnrtFunction_t function,
    void **inst_addr, cnrtInitFuncParam_t *param);

/**
 * @brief Malloc const addr on device and copy const data to device.
 *        If copy complete and return CNRT_RET_SUCCESS.
 * @param function[in] pointer of mlu function.
 * @param const_addr[out] get the pointer of mlu const memory.
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtInitFunctionConstMemory(cnrtFunction_t function, void **const_addr);

/*
 * @brief Malloc intermediate addr on device and copy intermediate to device.
 *        If copy complete and return CNRT_RET_SUCCESS.
 * @param function[in] pointer of mlu function.
 * @param intmd_addr[out] get the pointer of intermediate addr.
 * @param data_parallelism[in] data_parallelism size.
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t
    cnrtInitFunctionIntmdMemory(cnrtFunction_t function, void **intmd_addr, int data_parallelism);

/**
 * @brief Invoke a function with for data and inst multiplexing on MLU.
 * @param function[in] point to the MLU function.
 * @param dim[in]      how many grid dimentions.
 * @param params[in]   point to arguments.
 * @param func_type[in]        function type. @see cnrtFunctionType_t.
 * @param stream[in]   stream associated to the function call.
 * @param inst_addr[in]  the instruction address.
 * @param const_addr[in] the const data address.
 * @param intmd_addr[in] the intermediate address.
 * @param cnrtInvokeFuncParam[in] private pointer that is for cnrtInvokeFuncParam_t.
 *
 * @return CNRT_RET_SUCCESS if success,
 *         otherwise the error code is returned.
 */
extern __R_HOST
CNRT_DLL_API cnrtRet_t cnrtInvokeFunctionExtra(cnrtFunction_t function, cnrtDim3_t dim,
        void **params, cnrtFunctionType_t func_type, cnrtStream_t stream,
        void *inst_addr, void *const_addr, void *intmd_addr,
        void *cnrtInvokeFuncParam);

#if defined(__cplusplus)
}
#endif /*__cplusplus*/
#endif /*__CNRT_H*/
