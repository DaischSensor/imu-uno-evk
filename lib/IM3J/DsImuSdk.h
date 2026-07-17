#ifndef DS_IMU_SDK_H_
#define DS_IMU_SDK_H_

/* ================================================== INCLUDES =================================================== */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef float float32_t;

/* =========================================== ENUM TYPE DEFINITIONS ============================================ */
/**
 * @defgroup Imu Module types codes.
 * @{
 */
typedef enum {
    MODULE_IM3J = 0U,    /**< IMU module type IM3J. */
    MODULE_IM8S = 1U,    /**< IMU module type IM8S. */
    MODULE_MAX_TYPE,     /**< Maximum module type, no support. */
} ModulesType_t;

/**
 * @defgroup Gyroscope full-scale codes.
 * @{
 */
typedef enum {
    IM3J_FS_GY_125_DPS = 0U,                  /**< Gyroscope 125dps full-scale (For IM3J). */
    IM3J_FS_GY_250_DPS = 1U,                  /**< Gyroscope 250dps full-scale (For IM3J). */
    IM3J_FS_GY_500_DPS = 2U,                  /**< Gyroscope 500dps full-scale (For IM3J). */
    IM3J_FS_GY_1000_DPS = 3U,                 /**< Gyroscope 1000dps full-scale (For IM3J). */
    IM3J_FS_GY_2000_DPS = 4U,                 /**< Gyroscope 2000dps full-scale (For IM3J). */
    IM8S_FS_GY_125_DPS = 5U,                  /**< Gyroscope 125dps full-scale (For IM8S). */
    IM8S_FS_GY_250_DPS = 6U,                  /**< Gyroscope 250dps full-scale (For IM8S). */
    IM8S_FS_GY_500_DPS = 7U,                  /**< Gyroscope 500dps full-scale (For IM8S). */
    IM8S_FS_GY_1000_DPS = 8U,                 /**< Gyroscope 1000dps full-scale (For IM8S). */
    IM8S_FS_GY_2000_DPS = 9U                  /**< Gyroscope 2000dps full-scale (For IM8S). */
} FullScaleGyro_t;

/**
 * @defgroup Accelerometer full-scale codes.
 * @{
 */
typedef enum {
    IM3J_FS_XL_3_G = 0U,                      /**< Accelerometer 3g full-scale (For IM3J). */
    IM3J_FS_XL_6_G = 1U,                      /**< Accelerometer 6g full-scale (For IM3J). */
    IM3J_FS_XL_12_G = 2U,                     /**< Accelerometer 12g full-scale (For IM3J). */
    IM3J_FS_XL_24_G = 3U,                     /**< Accelerometer 24g full-scale (For IM3J). */
    IM8S_FS_XL_2_G = 4U,                      /**< Accelerometer 2g full-scale (For IM8S). */
    IM8S_FS_XL_4_G = 5U,                      /**< Accelerometer 4g full-scale (For IM8S). */
    IM8S_FS_XL_8_G = 6U,                      /**< Accelerometer 8g full-scale (For IM8S). */
    IM8S_FS_XL_16_G = 7U                      /**< Accelerometer 16g full-scale (For IM8S). */
} FullScaleXlmt_t;

/**
 * @defgroup Accelerometer output data rate codes.
 * @{
 */
typedef enum {
    IM3J_XL_ODR_12_5_HZ = 0U,                 /**< Accelerometer output data rate: 12.5Hz (For IM3J). */
    IM3J_XL_ODR_25_HZ = 1U,                   /**< Accelerometer output data rate: 25Hz (For IM3J). */
    IM3J_XL_ODR_50_HZ = 2U,                   /**< Accelerometer output data rate: 50Hz (For IM3J). */
    IM3J_XL_ODR_100_HZ = 3U,                  /**< Accelerometer output data rate: 100Hz (For IM3J). */
    IM3J_XL_ODR_200_HZ = 4U,                  /**< Accelerometer output data rate: 200Hz (For IM3J). */
    IM3J_XL_ODR_400_HZ = 5U,                  /**< Accelerometer output data rate: 400Hz (For IM3J). */
    IM3J_XL_ODR_800_HZ = 6U,                  /**< Accelerometer output data rate: 800Hz (For IM3J). */
    IM3J_XL_ODR_1600_HZ = 7U,                 /**< Accelerometer output data rate: 1600Hz (For IM3J). */
    BYPASS = 8U                               /**< No need to set up. */
} Xl_ODR_t;

/**
 * @defgroup Accelerometer low pass filter codes.
 * @{
 */
typedef enum {
    IM3J_XL_LPF_NORMAL = 0U,                  /**< Related to Accelerometer ODR set (For IM3J). */
    IM3J_XL_LPF_OSR2 = 1U,                    /**< Related to Accelerometer ODR set (For IM3J). */
    IM3J_XL_LPF_OSR4 = 2U,                    /**< Related to Accelerometer ODR set (For IM3J). */
    IM8S_XL_LPF_417HZ = 3U,                   /**< Accelerometer LPF2 417Hz (For IM8S). */
    IM8S_XL_LPF_167HZ = 4U,                   /**< Accelerometer LPF2 167Hz (For IM8S). */
    IM8S_XL_LPF_83HZ = 5U,                    /**< Accelerometer LPF2 83Hz (For IM8S). */
    IM8S_XL_LPF_37HZ = 6U,                    /**< Accelerometer LPF2 37Hz (For IM8S). */
    IM8S_XL_LPF_16HZ = 7U,                    /**< Accelerometer LPF2 16Hz (For IM8S). */
    IM8S_XL_LPF_8HZ = 8U,                     /**< Accelerometer LPF2 8Hz (For IM8S). */
    IM8S_XL_LPF_4HZ = 9U,                     /**< Accelerometer LPF2 4Hz (For IM8S). */
    IM8S_XL_LPF_2HZ = 10U,                    /**< Accelerometer LPF2 2Hz (For IM8S). */
    IM8S_XL_LPF_DISABLE = 11U                 /**< Accelerometer LPF2 disable (For IM8S). */
} Xl_LPF_t;

/**
 * @defgroup gyroscope output data rate codes and low pass filter codes.
 * @{
 */
typedef enum {
    IM3J_GY_ODR_2000_LPF_532_HZ = 0U,         /**< gyroscope output data rate: 2000Hz, low pass filter: 532Hz (For IM3J). */
    IM3J_GY_ODR_2000_LPF_230_HZ = 1U,         /**< gyroscope output data rate: 2000Hz, low pass filter: 230Hz (For IM3J). */
    IM3J_GY_ODR_1000_LPF_116_HZ = 2U,         /**< gyroscope output data rate: 1000Hz, low pass filter: 116Hz (For IM3J). */
    IM3J_GY_ODR_400_LPF_47_HZ = 3U,           /**< gyroscope output data rate: 400Hz,  low pass filter: 47Hz (For IM3J). */
    IM3J_GY_ODR_200_LPF_23_HZ = 4U,           /**< gyroscope output data rate: 200Hz, low pass filter: 23Hz (For IM3J). */
    IM3J_GY_ODR_100_LPF_12_HZ = 5U,           /**< gyroscope output data rate: 100Hz, low pass filter: 12Hz (For IM3J). */
    IM3J_GY_ODR_200_LPF_64_HZ = 6U,           /**< gyroscope output data rate: 200Hz, low pass filter: 64Hz (For IM3J). */
    IM3J_GY_ODR_100_LPF_32_HZ = 7U,           /**< gyroscope output data rate: 100Hz, low pass filter: 32Hz (For IM3J). */
    IM8S_GY_LPF_274HZ = 8U,                   /**< Gyroscope LPF1 274Hz (For IM8S). */
    IM8S_GY_LPF_212HZ = 9U,                   /**< Gyroscope LPF1 212Hz (For IM8S). */
    IM8S_GY_LPF_150HZ = 10U,                  /**< Gyroscope LPF1 150Hz (For IM8S). */
    IM8S_GY_LPF_390HZ = 11U,                  /**< Gyroscope LPF1 390Hz (For IM8S). */
    IM8S_GY_LPF_99HZ = 12U,                   /**< Gyroscope LPF1 99Hz (For IM8S). */
    IM8S_GY_LPF_50HZ = 13U,                   /**< Gyroscope LPF1 50Hz (For IM8S). */
    IM8S_GY_LPF_25HZ = 14U,                   /**< Gyroscope LPF1 25Hz (For IM8S). */
    IM8S_GY_LPF_12HZ = 15U,                   /**< Gyroscope LPF1 12Hz (For IM8S). */
    IM8S_GY_LPF_DISABLE = 16U                 /**< Gyroscope LPF1 disable (For IM8S). */
} GY_LPF_t;

/**
 * @defgroup IMU data get frequency codes.
 * @{
 */
typedef enum {
    FREQ_100HZ = 0U,                          /**< IMU Data output frequency: 100Hz. */
    FREQ_200HZ = 1U,                          /**< IMU Data output frequency: 200Hz. */
    FREQ_500HZ = 2U,                          /**< IMU Data output frequency: 500Hz. */
    FREQ_1000HZ = 3U                          /**< IMU Data output frequency: 1000Hz. */
} GetData_freq_t;

/**
 * @defgroup IMU Initialize status codes.
 * @{
 */
typedef enum {
    INIT_SUCCESS = 0U,                        /**< Initialized successfully. */
    INIT_ERR_COMM_IM3J_ACC = 1U,              /**< IM3J accelerometer communication failure during initialization. */
    INIT_ERR_COMM_IM3J_GYR = 2U,              /**< IM3J gyroscope communication failure during initialization. */
    INIT_ERR_COMM_IM8S_IMU = 3U,              /**< IM8S IMU communication failure during init. */
    INIT_ERR_COMM_EEPROM = 4U,                /**< EEPROM communication failure during initialization. */
    INIT_ERR_CALIBRATION_DATA = 5U,           /**< Calibration data verification failed. */
    INIT_ERR_NULL_PARA = 6U,                  /**< The parameter pointer is empty. */
    INIT_ERR_NULL_FUNC = 7U,                  /**< The function interface pointed to is empty. */
    INIT_ERR_SYSTICK = 8U,                    /**< Counter not accumulated. */
    INIT_ERR_PARA_OUT_OF_RANGE = 9U,          /**< Parameter set out of range. */
    INIT_ERR_PARA_POS = 10U,                  /**< Imu Pos Parameter set out of range cannot init. */
    INIT_ERR_PARA_EXTANGLE = 11U,             /**< Extrinsic angle Parameter set out of range cannot init. */
    INIT_ERR_PARA_MNTANGLE = 12U              /**< Mount angle Parameter set out of range cannot init. */
} DsImuInitStatus_t;

/**
 * @defgroup get IMU data status codes.
 * @{
 */
typedef enum {
    GET_SUCCESS = 0U,                         /**< Data get successfully. */
    GET_DATA_NOT_READY = 1U,                  /**< IMU Data not ready. */
    GET_ERR_UNINIT = 2U,                      /**< Uninitialized or failed to initialize. */
    GET_ERR_IM3J_ACC_COMM = 3U,               /**< IM3J accelerometer communication failure during run step. */
    GET_ERR_IM3J_GYR_COMM = 4U,               /**< IM3J gyroscope communication failure during run step. */
    GET_ERR_IM3J_ACC_GYR_COMM = 5U,           /**< IM3J accelerometer&gyroscope communication failure during run step. */
    GET_ERR_IM8S_IMU_COMM = 6U,               /**< IM8S imu communication failure during run step. */
    GET_ERR_NULL_PARA = 7U,                   /**< The parameter pointer is empty. */
    GET_ERR_DT_US = 8U,                       /**< The delta time(us) between two data acquisitions is abnormal. */
    GET_ERR_RESET = 9U,                       /**< Unexpected reset during run step. */
    GET_ERR_INVALID_DATA = 10U                /**< The output IMU data is invalid. */
} DsImuGetDataStatus_t;

/**
 * @defgroup get information status codes.
 * @{
 */
typedef enum {
    GET_INFO_SUCCESS = 0U,               /**< Info get successfully. */
    GET_INFO_ERR_UNINIT = 1U,            /**< Uninitialized or failed to initialize. */
    GET_INFO_NULL_PARA = 2U,             /**< The parameter pointer is empty. */
    GET_INFO_SN_CRC_ERROR = 3U           /**< SN check error. */
} DsImuGetInfoStatus_t;

/**
 * @defgroup get Extrinsic Angle status codes.
 * @{
 */
typedef enum {
    CAL_INPROGRESS = 0U,                 /**< In progress. */
    CAL_SUCCESS = 1U,                    /**< Computation successful. */
    CAL_FAIL = 2U,                       /**< Computation failed (general/unknown reason). */
    CAL_UNSTABLE = 3U,                   /**< Attitude is not stable enough. */
    CAL_NORMALIZEFAIL = 4U,              /**< Normalization failed (possibly a zero vector). */
    CAL_NOTLVL = 5U,                     /**< Attitude not Level (exceeds 5 degrees). */
    CAL_TOOLARGE = 6U,                   /**< Bias is too large. */
    CAL_NOTSTART = 7U,                   /**< Computation has not started. */
    CAL_NULLPTR = 8U                     /**< null pointer parameter. */
} DsImuGetExAngCalibStatus;

/**
 * @defgroup IMU module chip index.
 * @{
 */
typedef enum {
    DS_IM3J_ACC = 0U,                    /**< IM3J-accelerometer Chip ID. */
    DS_IM3J_GYR = 1U,                    /**< IM3J-gyroscope Chip ID. */
    DS_EEPROM = 2U,                      /**< EEPROM Chip ID. */
    DS_IM8S_IMU = 3U,                    /**< IM8S-imu Chip ID. */
    MAX_CHIP_ID                          /**< No related equipment ID, For equipment quantity only. */
} DsImuChipId_t;

/* ============================================= STRUCT TYPE DEFINITIONS =========================================== */
/**
 * @defgroup DsImu_SpiBuffType, spi transfer data structure.
 * @{
 */
typedef struct {
    uint8_t *pTxBuf;                     /**< SPI send data buffer. */
    uint8_t *pRxBuf;                     /**< SPI receive data buffer. */
    uint16_t u16Len;                     /**< SPI transfer data size (Byte). */
} DsImu_SpiBuffType;

/**
 * @defgroup DsImu_ImuRawDataType, IMU raw data structure.
 * @{
 */
typedef struct {
    float32_t fAcceleration[3];          /**< x,y,z acceleration in module frame, unit: m/s/s. */
    float32_t fAngularRate[3];           /**< x,y,z angular rate in module frame, unit: degree/s. */
    float32_t fTemperature;              /**< Module temperature in degrees Celcius. */
} DsImu_ImuRawDataType;

/**
 * @defgroup DsImu_ExAngDataType, IMU Extrin Angle structure.
 * @{
 */
typedef struct {
    float32_t fAng[3];                   /**< Imu x,y,z Calibration Extrin Angle, unit: rad. */
} DsImu_ExAngDataType;

/**
 * @defgroup DsImu_ImuCorrectedDataType, IMU corrected data structure.
 * @{
 */
typedef struct {
    float32_t fAcceleration[3];          /**< x,y,z acceleration in module frame, unit: m/s/s. */
    float32_t fAngularRate[3];           /**< x,y,z angular rate in module frame, unit: degree/s. */
    float32_t fAlignedAcceleration[3];   /**< x,y,z aligned acceleration in module frame, unit: m/s/s. */
    float32_t fAlignedAngularRate[3];    /**< x,y,z aligned angular rate in module frame, unit: degree/s. */
    float32_t fAttitude[3];              /**< [0]: Yaw, [1]: Pitch, [2]: Roll, unit: degree. */
    float32_t fQuat[4];                  /**< Quaternion, q = w + xi + yj + zk. */
    float32_t fTemperature;              /**< Module temperature in degrees Celcius. */
    uint64_t u64dtUs;                    /**< delta time of obtain data scheduling, unit: us. */
    uint16_t u16AhrsStatus;              /**< status info for Ahrs. */
} DsImu_ImuCorrectedDataType;

/**
 * @defgroup DsImu_InfoType, IMU module information structure.
 * @{
 */
typedef struct {
    uint8_t sn[8];                       /**< Serial Number. */
    char version[30];                    /**< SDK software version. */
} DsImu_InfoType;

/**
 * @defgroup DsImu_CfgInfoType configuration information structure.
 * @{
 */
typedef struct {
    uint8_t module_type;                 /**< module type MODULE_IM3J/MODULE_IM8S. */
    uint8_t acc_fs;                      /**< Accelerometer full-scale code. */
    uint8_t gyr_fs;                      /**< Gyroscope full-scale code. */
    uint8_t acc_odr;                     /**< Accelerometer Output data rate configuration. */
    uint8_t acc_lpf;                     /**< Accelerometer Low pass filter configuration. */
    uint8_t gyr_lpf;                     /**< Gyroscope Output data rate and Low pass filter configuration. */
    GetData_freq_t tim_freq;             /**< Get data frequency code. */
    uint32_t u32SpiTimeoutUs[2];         /**< 0: IM3J(accelerometer, gyroscope) or IM8S IMU SPI timeout, 1: EEPROM SPI timeout [unit: us]. */
    uint64_t u64UsTickValueMax;          /**< Accumulated maximum value of microsecond counter [unit: us] (>20000). */
    uint32_t mcu_freq;                   /**< MCU frequency [unit: MHz]. */
    uint8_t (*transfer)(DsImuChipId_t eChipId, DsImu_SpiBuffType *pxSpiBufArray, uint8_t u8ArraySize); /**< Function pointer for SPI reading & writing data to devices. */
    void (*cancel)(DsImuChipId_t eChipId);       /**< Function pointer for cancel SPI transfer sequence. */
    uint64_t (*systick)(void);                   /**< Function pointer for getting system tick [us]. */
    uint8_t (*getStatus)(DsImuChipId_t eChipId); /**< Function pointer for getting SPI transfer sequence result. */
    float32_t fMntAng[3];                /**< Imu Mount Angle Params 0/1/2 x/y/z [unit:rad]. */
    float32_t fExtAng[3];                /**< Imu Extrin Angle Params 0/1/2 x/y/z [unit:rad]. */
    float32_t fPos[3];                   /**< Imu Position Params 0/1/2 x/y/z [unit:meter]. */
} DsImu_CfgInfoType;

/* =============================================== PUBLIC FUNCTIONS ================================================ */
/**
 * @brief Initialize the IMU configure.
 *
 * @param[in] pxCfgInfo: Pointer to the structure containing the configuration information.
 *
 * @return DsImuInitStatus_t.
 */
DsImuInitStatus_t DsImu_Init(const DsImu_CfgInfoType *const pxCfgInfo);

/**
 * @brief Obtain IMU data.
 *
 * @param[out] pxImuCorData: Pointer to the structure of the get IMU Attitude and IMU corrected data.
 * @param[out] pxImuRawData: Pointer to the structure of the get IMU raw data.
 *
 * @return GetImuDataStatus_t.
 */
DsImuGetDataStatus_t DsImu_GetData(DsImu_ImuCorrectedDataType *const pxImuCorData, DsImu_ImuRawDataType *const pxImuRawData);

/**
 * @brief Obtain IMU constant information.
 *
 * @param[out] pInfo: Pointer to the structure containing the IMU constant information.
 *
 * @return: DsImuGetInfoStatus_t, range(0~3).
 */
DsImuGetInfoStatus_t DsImu_GetInfo(DsImu_InfoType *const pxInfo);

/* ================================================= Optional ====================================================== */
/**
 * @brief Calculate Extrin Angle.
 *
 * @return get start status, range(0~1) 0:success 1:fail.
 */
uint8_t DsImu_StartCalculateExtrinAngle(void);

/**
 * @brief Get Imu Extrin Angle.
 *
 * @param[out] pxAngData: Pointer to the structure of the get IMU Calibration Extrin Angle data.
 *
 * @return GetImuExtrinAng status, range(0~8).
 */
DsImuGetExAngCalibStatus DsImu_GetImuExtrinAng(DsImu_ExAngDataType *const pxAngData);

#ifdef __cplusplus
}
#endif

#endif /* DS_IMU_SDK_H_ */
