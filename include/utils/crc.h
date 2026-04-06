/**
 * @file crc.h
 * @brief CRC 校验计算 - CRC8/CRC16/CRC32
 * 
 * @author Your Name
 * @version 1.0.0
 * @date 2026-04-06
 * 
 * @copyright MIT License
 */

#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * CRC8
 *============================================================================*/

/**
 * @brief CRC8 模型配置
 */
typedef struct {
    uint8_t polynomial;     /**< 多项式 */
    uint8_t init;           /**< 初始值 */
    uint8_t xorout;         /**< 结果异或值 */
    bool reflect_in;        /**< 输入是否反转 */
    bool reflect_out;       /**< 输出是否反转 */
} crc8_config_t;

/**
 * @brief CRC8 标准模型
 */
extern const crc8_config_t crc8_ccitt;      /**< CRC8-CCITT: poly=0x07, init=0x00 */
extern const crc8_config_t crc8_maxim;      /**< CRC8/MAXIM: poly=0x31, init=0x00, xorout=0x00 */
extern const crc8_config_t crc8_dallas;     /**< CRC8-DALLAS: poly=0x31, init=0x00 */

/**
 * @brief 计算 CRC8
 * 
 * @param data 数据指针
 * @param length 数据长度
 * @param config CRC 配置（NULL 使用默认 CRC8-CCITT）
 * @return uint8_t CRC8 结果
 */
uint8_t crc8_calc(const uint8_t *data, size_t length, const crc8_config_t *config);

/**
 * @brief 计算 CRC8-CCITT（快捷方式）
 * 
 * @param data 数据指针
 * @param length 数据长度
 * @return uint8_t CRC8 结果
 */
uint8_t crc8_ccitt_calc(const uint8_t *data, size_t length);

/**
 * @brief 计算 CRC8-MAXIM（快捷方式）
 * 
 * @param data 数据指针
 * @param length 数据长度
 * @return uint8_t CRC8 结果
 */
uint8_t crc8_maxim_calc(const uint8_t *data, size_t length);

/*============================================================================
 * CRC16
 *============================================================================*/

/**
 * @brief CRC16 模型配置
 */
typedef struct {
    uint16_t polynomial;    /**< 多项式 */
    uint16_t init;          /**< 初始值 */
    uint16_t xorout;        /**< 结果异或值 */
    bool reflect_in;        /**< 输入是否反转 */
    bool reflect_out;       /**< 输出是否反转 */
} crc16_config_t;

/**
 * @brief CRC16 标准模型
 */
extern const crc16_config_t crc16_ibm;      /**< CRC16-IBM: poly=0x8005, init=0x0000 */
extern const crc16_config_t crc16_ccitt;    /**< CRC16-CCITT: poly=0x1021, init=0xFFFF */
extern const crc16_config_t crc16_modbus;   /**< CRC16-MODBUS: poly=0x8005, init=0xFFFF */
extern const crc16_config_t crc16_usb;      /**< CRC16-USB: poly=0x8005, init=0xFFFF, xorout=0xFFFF */

/**
 * @brief 计算 CRC16
 * 
 * @param data 数据指针
 * @param length 数据长度
 * @param config CRC 配置（NULL 使用默认 CRC16-CCITT）
 * @return uint16_t CRC16 结果
 */
uint16_t crc16_calc(const uint8_t *data, size_t length, const crc16_config_t *config);

/**
 * @brief 计算 CRC16-CCITT（快捷方式）
 * 
 * @param data 数据指针
 * @param length 数据长度
 * @return uint16_t CRC16 结果
 */
uint16_t crc16_ccitt_calc(const uint8_t *data, size_t length);

/**
 * @brief 计算 CRC16-MODBUS（快捷方式）
 * 
 * @param data 数据指针
 * @param length 数据长度
 * @return uint16_t CRC16 结果
 */
uint16_t crc16_modbus_calc(const uint8_t *data, size_t length);

/*============================================================================
 * CRC32
 *============================================================================*/

/**
 * @brief CRC32 模型配置
 */
typedef struct {
    uint32_t polynomial;    /**< 多项式 */
    uint32_t init;          /**< 初始值 */
    uint32_t xorout;        /**< 结果异或值 */
    bool reflect_in;        /**< 输入是否反转 */
    bool reflect_out;       /**< 输出是否反转 */
} crc32_config_t;

/**
 * @brief CRC32 标准模型
 */
extern const crc32_config_t crc32_ethernet; /**< CRC32-ETHERNET: poly=0x04C11DB7 */
extern const crc32_config_t crc32_iso;      /**< CRC32-ISO/HDLC: poly=0x04C11DB7, init=0xFFFFFFFF */

/**
 * @brief 计算 CRC32
 * 
 * @param data 数据指针
 * @param length 数据长度
 * @param config CRC 配置（NULL 使用默认 CRC32-ISO）
 * @return uint32_t CRC32 结果
 */
uint32_t crc32_calc(const uint8_t *data, size_t length, const crc32_config_t *config);

/**
 * @brief 计算 CRC32-ISO（快捷方式）
 * 
 * @param data 数据指针
 * @param length 数据长度
 * @return uint32_t CRC32 结果
 */
uint32_t crc32_iso_calc(const uint8_t *data, size_t length);

/**
 * @brief CRC32 查表初始化（可选，提高性能）
 */
void crc32_init_table(void);

#ifdef __cplusplus
}
#endif

#endif /* CRC_H */
