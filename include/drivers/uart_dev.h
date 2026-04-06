/**
 * @file uart_dev.h
 * @brief UART 环形缓冲收发框架
 * 
 * @author Your Name
 * @version 1.0.0
 * @date 2026-04-06
 * 
 * @copyright MIT License
 */

#ifndef UART_DEV_H
#define UART_DEV_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * 错误码定义
 *============================================================================*/

#define UART_OK             0       /**< 成功 */
#define UART_ERR_TIMEOUT    -1      /**< 超时 */
#define UART_ERR_BUSY       -2      /**< 忙 */
#define UART_ERR_PARAM      -3      /**< 参数错误 */
#define UART_ERR_OVERFLOW   -4      /**< 接收溢出 */
#define UART_ERR_FRAME      -5      /**< 帧错误 */
#define UART_ERR_NOISE      -6      /**< 噪声错误 */

/*============================================================================
 * UART 配置
 *============================================================================*/

/**
 * @brief 数据位长度
 */
typedef enum {
    UART_DATA_7 = 7,      /**< 7 位数据 */
    UART_DATA_8 = 8,      /**< 8 位数据 */
    UART_DATA_9 = 9       /**< 9 位数据 */
} uart_data_bits_t;

/**
 * @brief 停止位长度
 */
typedef enum {
    UART_STOP_1   = 0,    /**< 1 位停止位 */
    UART_STOP_1_5 = 1,    /**< 1.5 位停止位 */
    UART_STOP_2   = 2     /**< 2 位停止位 */
} uart_stop_bits_t;

/**
 * @brief 校验位
 */
typedef enum {
    UART_PARITY_NONE   = 0, /**< 无校验 */
    UART_PARITY_EVEN   = 1, /**< 偶校验 */
    UART_PARITY_ODD    = 2, /**< 奇校验 */
    UART_PARITY_MARK   = 3, /**< 恒 1 校验 */
    UART_PARITY_SPACE  = 4  /**< 恒 0 校验 */
} uart_parity_t;

/**
 * @brief UART 设备句柄
 */
typedef struct {
    void *uart_handle;          /**< UART 句柄（由平台实现） */
    uint32_t baudrate;          /**< 波特率 */
    uart_data_bits_t data_bits; /**< 数据位 */
    uart_stop_bits_t stop_bits; /**< 停止位 */
    uart_parity_t parity;       /**< 校验位 */
    bool hw_flow_ctrl;          /**< 硬件流控（RTS/CTS） */
    uint32_t rx_timeout_ms;     /**< 接收超时（ms） */
    void *user_data;            /**< 用户数据 */
    
    // 环形缓冲区（内部使用）
    uint8_t *rx_buffer;         /**< 接收缓冲区 */
    size_t rx_size;             /**< 接收缓冲区大小 */
    size_t rx_head;             /**< 接收写指针 */
    size_t rx_tail;             /**< 接收读指针 */
    
    uint8_t *tx_buffer;         /**< 发送缓冲区 */
    size_t tx_size;             /**< 发送缓冲区大小 */
    size_t tx_head;             /**< 发送写指针 */
    size_t tx_tail;             /**< 发送读指针 */
    
    volatile bool rx_busy;      /**< 接收忙标志 */
    volatile bool tx_busy;      /**< 发送忙标志 */
} uart_dev_t;

/**
 * @brief UART 平台操作接口（需要移植）
 */
typedef struct {
    /**
     * @brief UART 初始化
     * @param handle UART 句柄
     * @param baudrate 波特率
     * @param data_bits 数据位
     * @param stop_bits 停止位
     * @param parity 校验位
     * @return UART_OK 成功，其他失败
     */
    int (*init)(void *handle, uint32_t baudrate, uart_data_bits_t data_bits,
                uart_stop_bits_t stop_bits, uart_parity_t parity);
    
    /**
     * @brief UART 去初始化
     * @param handle UART 句柄
     */
    void (*deinit)(void *handle);
    
    /**
     * @brief 发送一个字节（阻塞）
     * @param handle UART 句柄
     * @param data 数据字节
     * @param timeout 超时时间
     * @return UART_OK 成功，其他失败
     */
    int (*send_byte)(void *handle, uint8_t data, uint32_t timeout);
    
    /**
     * @brief 接收一个字节（阻塞）
     * @param handle UART 句柄
     * @param data 数据指针
     * @param timeout 超时时间
     * @return UART_OK 成功，其他失败
     */
    int (*recv_byte)(void *handle, uint8_t *data, uint32_t timeout);
    
    /**
     * @brief 启动 DMA 接收（可选）
     * @param handle UART 句柄
     * @param buffer 接收缓冲区
     * @param size 缓冲区大小
     * @return UART_OK 成功，其他失败
     */
    int (*dma_recv_start)(void *handle, uint8_t *buffer, size_t size);
    
    /**
     * @brief 启动 DMA 发送（可选）
     * @param handle UART 句柄
     * @param buffer 发送缓冲区
     * @param size 发送大小
     * @return UART_OK 成功，其他失败
     */
    int (*dma_send_start)(void *handle, const uint8_t *buffer, size_t size);
    
    /**
     * @brief 获取 DMA 接收计数（可选）
     * @param handle UART 句柄
     * @return 已接收字节数
     */
    size_t (*dma_get_count)(void *handle);
    
    /**
     * @brief UART 中断处理函数（在 ISR 中调用）
     * @param dev UART 设备句柄
     */
    void (*irq_handler)(uart_dev_t *dev);
} uart_platform_ops_t;

/*============================================================================
 * 设备管理
 *============================================================================*/

/**
 * @brief 初始化 UART 设备
 * 
 * @param dev UART 设备句柄
 * @param uart_handle UART 硬件句柄（由平台提供）
 * @param rx_buffer 接收缓冲区
 * @param rx_size 接收缓冲区大小
 * @param tx_buffer 发送缓冲区
 * @param tx_size 发送缓冲区大小
 * @param baudrate 波特率
 * @return UART_OK 成功，其他失败
 */
int uart_dev_init(uart_dev_t *dev, void *uart_handle, 
                  uint8_t *rx_buffer, size_t rx_size,
                  uint8_t *tx_buffer, size_t tx_size,
                  uint32_t baudrate);

/**
 * @brief 配置 UART 参数
 * 
 * @param dev UART 设备句柄
 * @param data_bits 数据位
 * @param stop_bits 停止位
 * @param parity 校验位
 * @param hw_flow_ctrl 硬件流控
 * @return UART_OK 成功，其他失败
 */
int uart_dev_configure(uart_dev_t *dev, uart_data_bits_t data_bits,
                       uart_stop_bits_t stop_bits, uart_parity_t parity,
                       bool hw_flow_ctrl);

/**
 * @brief 设置波特率
 * 
 * @param dev UART 设备句柄
 * @param baudrate 波特率
 * @return UART_OK 成功，其他失败
 */
int uart_dev_set_baudrate(uart_dev_t *dev, uint32_t baudrate);

/*============================================================================
 * 数据收发（阻塞）
 *============================================================================*/

/**
 * @brief 发送数据（阻塞）
 * 
 * @param dev UART 设备句柄
 * @param data 数据指针
 * @param len 数据长度
 * @param timeout 超时时间
 * @return 实际发送字节数，负值失败
 */
int uart_dev_send(uart_dev_t *dev, const uint8_t *data, size_t len, uint32_t timeout);

/**
 * @brief 接收数据（阻塞）
 * 
 * @param dev UART 设备句柄
 * @param data 数据指针
 * @param len 数据长度
 * @param timeout 超时时间
 * @return 实际接收字节数，负值失败
 */
int uart_dev_recv(uart_dev_t *dev, uint8_t *data, size_t len, uint32_t timeout);

/**
 * @brief 发送一个字节（阻塞）
 * 
 * @param dev UART 设备句柄
 * @param data 数据字节
 * @param timeout 超时时间
 * @return UART_OK 成功，其他失败
 */
int uart_dev_send_byte(uart_dev_t *dev, uint8_t data, uint32_t timeout);

/**
 * @brief 接收一个字节（阻塞）
 * 
 * @param dev UART 设备句柄
 * @param data 数据指针
 * @param timeout 超时时间
 * @return UART_OK 成功，其他失败
 */
int uart_dev_recv_byte(uart_dev_t *dev, uint8_t *data, uint32_t timeout);

/**
 * @brief 发送字符串（阻塞）
 * 
 * @param dev UART 设备句柄
 * @param str 字符串指针
 * @param timeout 超时时间
 * @return 实际发送字节数，负值失败
 */
int uart_dev_send_string(uart_dev_t *dev, const char *str, uint32_t timeout);

/*============================================================================
 * 数据收发（非阻塞 + 中断/DMA）
 *============================================================================*/

/**
 * @brief 写入数据到发送缓冲区（非阻塞）
 * 
 * @param dev UART 设备句柄
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际写入字节数
 */
size_t uart_dev_write(uart_dev_t *dev, const uint8_t *data, size_t len);

/**
 * @brief 从接收缓冲区读取数据（非阻塞）
 * 
 * @param dev UART 设备句柄
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际读取字节数
 */
size_t uart_dev_read(uart_dev_t *dev, uint8_t *data, size_t len);

/**
 * @brief 查询接收缓冲区可用数据量
 * 
 * @param dev UART 设备句柄
 * @return 可读字节数
 */
size_t uart_dev_available(uart_dev_t *dev);

/**
 * @brief 查询发送缓冲区剩余空间
 * 
 * @param dev UART 设备句柄
 * @return 可写字节数
 */
size_t uart_dev_tx_space(uart_dev_t *dev);

/**
 * @brief 清空接收缓冲区
 * 
 * @param dev UART 设备句柄
 */
void uart_dev_flush_rx(uart_dev_t *dev);

/**
 * @brief 清空发送缓冲区
 * 
 * @param dev UART 设备句柄
 */
void uart_dev_flush_tx(uart_dev_t *dev);

/**
 * @brief 等待发送完成
 * 
 * @param dev UART 设备句柄
 * @param timeout 超时时间
 * @return UART_OK 成功，UART_ERR_TIMEOUT 超时
 */
int uart_dev_wait_tx_complete(uart_dev_t *dev, uint32_t timeout);

/*============================================================================
 * 中断处理（在 ISR 中调用）
 *============================================================================*/

/**
 * @brief UART 接收中断处理
 * 
 * @param dev UART 设备句柄
 * @param data 接收到的数据
 */
void uart_dev_isr_rx(uart_dev_t *dev, uint8_t data);

/**
 * @brief UART 发送中断处理
 * 
 * @param dev UART 设备句柄
 * @param data 待发送的数据指针
 * @param len 最大发送长度
 * @return 实际发送字节数
 */
size_t uart_dev_isr_tx(uart_dev_t *dev, uint8_t *data, size_t len);

/**
 * @brief UART 错误中断处理
 * 
 * @param dev UART 设备句柄
 * @param error 错误类型
 */
void uart_dev_isr_error(uart_dev_t *dev, int error);

/*============================================================================
 * 平台接口注册（移植用）
 *============================================================================*/

/**
 * @brief 注册 UART 平台操作接口
 * 
 * @param ops 平台操作接口
 */
void uart_dev_register_platform(const uart_platform_ops_t *ops);

#ifdef __cplusplus
}
#endif

#endif /* UART_DEV_H */
