/**
 * @file spi_dev.h
 * @brief SPI 设备通用驱动框架
 * 
 * @author Your Name
 * @version 1.0.0
 * @date 2026-04-06
 * 
 * @copyright MIT License
 */

#ifndef SPI_DEV_H
#define SPI_DEV_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * 错误码定义
 *============================================================================*/

#define SPI_OK              0       /**< 成功 */
#define SPI_ERR_TIMEOUT     -1      /**< 超时 */
#define SPI_ERR_BUS         -2      /**< 总线错误 */
#define SPI_ERR_PARAM       -3      /**< 参数错误 */
#define SPI_ERR_NOT_READY   -4      /**< 设备未就绪 */
#define SPI_ERR_CS          -5      /**< 片选错误 */

/*============================================================================
 * SPI 模式定义
 *============================================================================*/

#define SPI_MODE_0          0       /**< CPOL=0, CPHA=0 */
#define SPI_MODE_1          1       /**< CPOL=0, CPHA=1 */
#define SPI_MODE_2          2       /**< CPOL=1, CPHA=0 */
#define SPI_MODE_3          3       /**< CPOL=1, CPHA=1 */

/*============================================================================
 * SPI 设备结构
 *============================================================================*/

/**
 * @brief SPI 设备句柄
 */
typedef struct {
    void *spi_handle;           /**< SPI 总线句柄（由平台实现） */
    void *cs_pin;               /**< 片选引脚（由平台实现） */
    uint32_t freq_hz;           /**< SPI 频率（Hz） */
    uint8_t mode;               /**< SPI 模式（0-3） */
    uint8_t data_bits;          /**< 数据位数（8 或 16） */
    uint32_t timeout;           /**< 超时时间（ms） */
    bool msb_first;             /**< MSB 优先（true）或 LSB 优先（false） */
    void *user_data;            /**< 用户数据 */
} spi_dev_t;

/**
 * @brief SPI 平台操作接口（需要移植）
 */
typedef struct {
    /**
     * @brief 配置 SPI 参数
     * @param handle SPI 总线句柄
     * @param freq_hz 频率（Hz）
     * @param mode SPI 模式（0-3）
     * @param data_bits 数据位数
     * @return SPI_OK 成功，其他失败
     */
    int (*configure)(void *handle, uint32_t freq_hz, uint8_t mode, uint8_t data_bits);
    
    /**
     * @brief SPI 片选有效
     * @param cs_pin 片选引脚
     */
    void (*cs_select)(void *cs_pin);
    
    /**
     * @brief SPI 片选无效
     * @param cs_pin 片选引脚
     */
    void (*cs_deselect)(void *cs_pin);
    
    /**
     * @brief SPI 写操作
     * @param handle SPI 总线句柄
     * @param data 数据指针
     * @param len 数据长度
     * @param timeout 超时时间
     * @return SPI_OK 成功，其他失败
     */
    int (*write)(void *handle, const uint8_t *data, size_t len, uint32_t timeout);
    
    /**
     * @brief SPI 读操作
     * @param handle SPI 总线句柄
     * @param data 数据指针
     * @param len 数据长度
     * @param timeout 超时时间
     * @return SPI_OK 成功，其他失败
     */
    int (*read)(void *handle, uint8_t *data, size_t len, uint32_t timeout);
    
    /**
     * @brief SPI 全双工读写
     * @param handle SPI 总线句柄
     * @param write_data 写入数据指针（NULL 则只读）
     * @param read_data 读取数据指针（NULL 则只写）
     * @param len 数据长度
     * @param timeout 超时时间
     * @return SPI_OK 成功，其他失败
     */
    int (*transfer)(void *handle, const uint8_t *write_data, uint8_t *read_data, 
                    size_t len, uint32_t timeout);
} spi_platform_ops_t;

/*============================================================================
 * 设备管理
 *============================================================================*/

/**
 * @brief 初始化 SPI 设备
 * 
 * @param dev SPI 设备句柄
 * @param spi_handle SPI 总线句柄（由平台提供）
 * @param cs_pin 片选引脚（由平台提供）
 * @param freq_hz SPI 频率（Hz）
 * @param mode SPI 模式（0-3）
 * @param timeout 超时时间（ms）
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_init(spi_dev_t *dev, void *spi_handle, void *cs_pin, 
                 uint32_t freq_hz, uint8_t mode, uint32_t timeout);

/**
 * @brief 设置 SPI 频率
 * 
 * @param dev SPI 设备句柄
 * @param freq_hz 频率（Hz）
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_set_freq(spi_dev_t *dev, uint32_t freq_hz);

/**
 * @brief 设置 SPI 模式
 * 
 * @param dev SPI 设备句柄
 * @param mode SPI 模式（0-3）
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_set_mode(spi_dev_t *dev, uint8_t mode);

/*============================================================================
 * 数据传输
 *============================================================================*/

/**
 * @brief SPI 写入数据
 * 
 * @param dev SPI 设备句柄
 * @param data 数据指针
 * @param len 数据长度
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_write(spi_dev_t *dev, const uint8_t *data, size_t len);

/**
 * @brief SPI 读取数据
 * 
 * @param dev SPI 设备句柄
 * @param data 数据指针
 * @param len 数据长度
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_read(spi_dev_t *dev, uint8_t *data, size_t len);

/**
 * @brief SPI 全双工传输
 * 
 * @param dev SPI 设备句柄
 * @param write_data 写入数据指针（NULL 则只读）
 * @param read_data 读取数据指针（NULL 则只写）
 * @param len 数据长度
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_transfer(spi_dev_t *dev, const uint8_t *write_data, uint8_t *read_data, size_t len);

/**
 * @brief SPI 写入一个字节
 * 
 * @param dev SPI 设备句柄
 * @param data 数据字节
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_write_byte(spi_dev_t *dev, uint8_t data);

/**
 * @brief SPI 读取一个字节
 * 
 * @param dev SPI 设备句柄
 * @param data 数据指针
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_read_byte(spi_dev_t *dev, uint8_t *data);

/**
 * @brief SPI 写入 + 读取（常用命令模式）
 * 
 * @param dev SPI 设备句柄
 * @param cmd 命令字节
 * @param write_data 后续写入数据（可选）
 * @param write_len 后续写入长度
 * @param read_data 读取数据缓冲区
 * @param read_len 读取长度
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_cmd(spi_dev_t *dev, uint8_t cmd, const uint8_t *write_data, size_t write_len,
                uint8_t *read_data, size_t read_len);

/*============================================================================
 * 便捷函数（针对常见设备）
 *============================================================================*/

/**
 * @brief 读取设备 ID（常见于 Flash/SD 卡）
 * 
 * @param dev SPI 设备句柄
 * @param cmd 读取 ID 命令
 * @param id 返回的 ID 缓冲区
 * @param id_len ID 长度
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_read_id(spi_dev_t *dev, uint8_t cmd, uint8_t *id, size_t id_len);

/**
 * @brief 读取设备状态寄存器
 * 
 * @param dev SPI 设备句柄
 * @param cmd 读状态命令
 * @param status 状态值
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_read_status(spi_dev_t *dev, uint8_t cmd, uint8_t *status);

/**
 * @brief 写入设备状态/配置寄存器
 * 
 * @param dev SPI 设备句柄
 * @param cmd 写配置命令
 * @param value 配置值
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_write_config(spi_dev_t *dev, uint8_t cmd, uint8_t value);

/**
 * @brief 等待设备就绪（轮询状态）
 * 
 * @param dev SPI 设备句柄
 * @param cmd 读状态命令
 * @param ready_mask 就绪位掩码
 * @param ready_value 就绪值（与掩码后的比较值）
 * @param timeout_ms 超时时间（ms）
 * @return SPI_OK 成功，其他失败
 */
int spi_dev_wait_ready(spi_dev_t *dev, uint8_t cmd, uint8_t ready_mask, 
                       uint8_t ready_value, uint32_t timeout_ms);

/*============================================================================
 * 平台接口注册（移植用）
 *============================================================================*/

/**
 * @brief 注册 SPI 平台操作接口
 * 
 * @param ops 平台操作接口
 */
void spi_dev_register_platform(const spi_platform_ops_t *ops);

#ifdef __cplusplus
}
#endif

#endif /* SPI_DEV_H */
