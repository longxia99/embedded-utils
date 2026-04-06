/**
 * @file i2c_dev.h
 * @brief I2C 设备通用驱动框架
 * 
 * @author Your Name
 * @version 1.0.0
 * @date 2026-04-06
 * 
 * @copyright MIT License
 */

#ifndef I2C_DEV_H
#define I2C_DEV_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * 错误码定义
 *============================================================================*/

#define I2C_OK              0       /**< 成功 */
#define I2C_ERR_TIMEOUT     -1      /**< 超时 */
#define I2C_ERR_NACK        -2      /**< 无应答 */
#define I2C_ERR_BUS         -3      /**< 总线错误 */
#define I2C_ERR_PARAM       -4      /**< 参数错误 */
#define I2C_ERR_NOT_READY   -5      /**< 设备未就绪 */

/*============================================================================
 * I2C 设备结构
 *============================================================================*/

/**
 * @brief I2C 设备句柄
 */
typedef struct {
    void *i2c_handle;           /**< I2C 总线句柄（由平台实现） */
    uint16_t dev_addr;          /**< 设备地址（7 位） */
    uint32_t timeout;           /**< 超时时间（ms） */
    bool auto_inc_addr;         /**< 是否自动递增地址（用于连续读写） */
    void *user_data;            /**< 用户数据 */
} i2c_dev_t;

/**
 * @brief I2C 平台操作接口（需要移植）
 */
typedef struct {
    /**
     * @brief I2C 写操作
     * @param handle I2C 总线句柄
     * @param dev_addr 设备地址
     * @param data 数据指针
     * @param len 数据长度
     * @param timeout 超时时间
     * @return I2C_OK 成功，其他失败
     */
    int (*write)(void *handle, uint16_t dev_addr, const uint8_t *data, size_t len, uint32_t timeout);
    
    /**
     * @brief I2C 读操作
     * @param handle I2C 总线句柄
     * @param dev_addr 设备地址
     * @param data 数据指针
     * @param len 数据长度
     * @param timeout 超时时间
     * @return I2C_OK 成功，其他失败
     */
    int (*read)(void *handle, uint16_t dev_addr, uint8_t *data, size_t len, uint32_t timeout);
    
    /**
     * @brief I2C 写 + 读操作（重复 START）
     * @param handle I2C 总线句柄
     * @param dev_addr 设备地址
     * @param write_data 写入数据指针
     * @param write_len 写入长度
     * @param read_data 读取数据指针
     * @param read_len 读取长度
     * @param timeout 超时时间
     * @return I2C_OK 成功，其他失败
     */
    int (*write_read)(void *handle, uint16_t dev_addr, 
                      const uint8_t *write_data, size_t write_len,
                      uint8_t *read_data, size_t read_len, 
                      uint32_t timeout);
} i2c_platform_ops_t;

/*============================================================================
 * 设备管理
 *============================================================================*/

/**
 * @brief 初始化 I2C 设备
 * 
 * @param dev I2C 设备句柄
 * @param i2c_handle I2C 总线句柄（由平台提供）
 * @param dev_addr 设备地址（7 位）
 * @param timeout 超时时间（ms）
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_init(i2c_dev_t *dev, void *i2c_handle, uint16_t dev_addr, uint32_t timeout);

/**
 * @brief 检测设备是否存在
 * 
 * @param dev I2C 设备句柄
 * @return I2C_OK 存在，其他失败
 */
int i2c_dev_scan(i2c_dev_t *dev);

/*============================================================================
 * 寄存器操作（针对有寄存器映射的设备）
 *============================================================================*/

/**
 * @brief 读取设备寄存器
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址
 * @param data 数据指针
 * @param len 读取长度
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_read_reg(i2c_dev_t *dev, uint8_t reg_addr, uint8_t *data, size_t len);

/**
 * @brief 写入设备寄存器
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址
 * @param data 数据指针
 * @param len 写入长度
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_write_reg(i2c_dev_t *dev, uint8_t reg_addr, const uint8_t *data, size_t len);

/**
 * @brief 读取寄存器（16 位地址）
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址（16 位）
 * @param data 数据指针
 * @param len 读取长度
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_read_reg16(i2c_dev_t *dev, uint16_t reg_addr, uint8_t *data, size_t len);

/**
 * @brief 写入寄存器（16 位地址）
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址（16 位）
 * @param data 数据指针
 * @param len 写入长度
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_write_reg16(i2c_dev_t *dev, uint16_t reg_addr, const uint8_t *data, size_t len);

/**
 * @brief 修改寄存器特定位（读 - 改 - 写）
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址
 * @param mask 位掩码
 * @param value 新值（已移位）
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_modify_reg(i2c_dev_t *dev, uint8_t reg_addr, uint8_t mask, uint8_t value);

/*============================================================================
 * 数据读写
 *============================================================================*/

/**
 * @brief 从设备读取数据（无寄存器地址）
 * 
 * @param dev I2C 设备句柄
 * @param data 数据指针
 * @param len 读取长度
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_read(i2c_dev_t *dev, uint8_t *data, size_t len);

/**
 * @brief 向设备写入数据（无寄存器地址）
 * 
 * @param dev I2C 设备句柄
 * @param data 数据指针
 * @param len 写入长度
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_write(i2c_dev_t *dev, const uint8_t *data, size_t len);

/*============================================================================
 * 便捷函数
 *============================================================================*/

/**
 * @brief 读取 8 位寄存器
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址
 * @param value 读取的值
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_read_reg8(i2c_dev_t *dev, uint8_t reg_addr, uint8_t *value);

/**
 * @brief 读取 16 位寄存器（大端）
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址
 * @param value 读取的值
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_read_reg16_be(i2c_dev_t *dev, uint8_t reg_addr, uint16_t *value);

/**
 * @brief 读取 16 位寄存器（小端）
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址
 * @param value 读取的值
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_read_reg16_le(i2c_dev_t *dev, uint8_t reg_addr, uint16_t *value);

/**
 * @brief 写入 8 位寄存器
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址
 * @param value 写入的值
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_write_reg8(i2c_dev_t *dev, uint8_t reg_addr, uint8_t value);

/**
 * @brief 写入 16 位寄存器（大端）
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址
 * @param value 写入的值
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_write_reg16_be(i2c_dev_t *dev, uint8_t reg_addr, uint16_t value);

/**
 * @brief 写入 16 位寄存器（小端）
 * 
 * @param dev I2C 设备句柄
 * @param reg_addr 寄存器地址
 * @param value 写入的值
 * @return I2C_OK 成功，其他失败
 */
int i2c_dev_write_reg16_le(i2c_dev_t *dev, uint8_t reg_addr, uint16_t value);

/*============================================================================
 * 平台接口注册（移植用）
 *============================================================================*/

/**
 * @brief 注册 I2C 平台操作接口
 * 
 * @param ops 平台操作接口
 */
void i2c_dev_register_platform(const i2c_platform_ops_t *ops);

#ifdef __cplusplus
}
#endif

#endif /* I2C_DEV_H */
