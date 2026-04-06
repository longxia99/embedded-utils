/**
 * @file uart_dev.c
 * @brief UART 环形缓冲收发框架实现
 */

#include "uart_dev.h"
#include <string.h>
#include <stdlib.h>

/*============================================================================
 * 全局变量
 *============================================================================*/

static uart_platform_ops_t g_uart_ops = {0};

/*============================================================================
 * 辅助函数
 *============================================================================*/

/**
 * @brief 环形缓冲区写入（内部使用）
 */
static size_t ring_buffer_write(uint8_t *buffer, size_t size, 
                                volatile size_t *head, volatile size_t *tail,
                                const uint8_t *data, size_t len)
{
    size_t written = 0;
    size_t next_head;
    
    for (size_t i = 0; i < len; i++) {
        next_head = (*head + 1) % size;
        
        // 缓冲区满
        if (next_head == *tail) {
            break;
        }
        
        buffer[*head] = data[i];
        *head = next_head;
        written++;
    }
    
    return written;
}

/**
 * @brief 环形缓冲区读取（内部使用）
 */
static size_t ring_buffer_read(uint8_t *buffer, size_t size,
                               volatile size_t *head, volatile size_t *tail,
                               uint8_t *data, size_t len)
{
    size_t read = 0;
    
    while (read < len && *head != *tail) {
        data[read] = buffer[*tail];
        *tail = (*tail + 1) % size;
        read++;
    }
    
    return read;
}

/**
 * @brief 查询环形缓冲区数据量
 */
static size_t ring_buffer_count(size_t head, size_t tail, size_t size)
{
    if (head >= tail) {
        return head - tail;
    } else {
        return size - tail + head;
    }
}

/**
 * @brief 查询环形缓冲区剩余空间
 */
static size_t ring_buffer_space(size_t head, size_t tail, size_t size)
{
    return size - ring_buffer_count(head, tail, size) - 1;
}

/*============================================================================
 * 设备管理
 *============================================================================*/

int uart_dev_init(uart_dev_t *dev, void *uart_handle, 
                  uint8_t *rx_buffer, size_t rx_size,
                  uint8_t *tx_buffer, size_t tx_size,
                  uint32_t baudrate)
{
    if (dev == NULL || uart_handle == NULL || rx_buffer == NULL || tx_buffer == NULL) {
        return UART_ERR_PARAM;
    }
    
    dev->uart_handle = uart_handle;
    dev->baudrate = baudrate;
    dev->data_bits = UART_DATA_8;
    dev->stop_bits = UART_STOP_1;
    dev->parity = UART_PARITY_NONE;
    dev->hw_flow_ctrl = false;
    dev->rx_timeout_ms = 100;
    dev->user_data = NULL;
    
    // 初始化环形缓冲区
    dev->rx_buffer = rx_buffer;
    dev->rx_size = rx_size;
    dev->rx_head = 0;
    dev->rx_tail = 0;
    
    dev->tx_buffer = tx_buffer;
    dev->tx_size = tx_size;
    dev->tx_head = 0;
    dev->tx_tail = 0;
    
    dev->rx_busy = false;
    dev->tx_busy = false;
    
    // 调用平台初始化
    if (g_uart_ops.init != NULL) {
        return g_uart_ops.init(uart_handle, baudrate, dev->data_bits, 
                               dev->stop_bits, dev->parity);
    }
    
    return UART_OK;
}

int uart_dev_configure(uart_dev_t *dev, uart_data_bits_t data_bits,
                       uart_stop_bits_t stop_bits, uart_parity_t parity,
                       bool hw_flow_ctrl)
{
    if (dev == NULL || g_uart_ops.init == NULL) {
        return UART_ERR_PARAM;
    }
    
    dev->data_bits = data_bits;
    dev->stop_bits = stop_bits;
    dev->parity = parity;
    dev->hw_flow_ctrl = hw_flow_ctrl;
    
    return g_uart_ops.init(dev->uart_handle, dev->baudrate, dev->data_bits,
                           dev->stop_bits, dev->parity);
}

int uart_dev_set_baudrate(uart_dev_t *dev, uint32_t baudrate)
{
    if (dev == NULL) {
        return UART_ERR_PARAM;
    }
    
    dev->baudrate = baudrate;
    
    // 重新初始化 UART
    if (g_uart_ops.init != NULL) {
        return g_uart_ops.init(dev->uart_handle, baudrate, dev->data_bits,
                               dev->stop_bits, dev->parity);
    }
    
    return UART_OK;
}

/*============================================================================
 * 数据收发（阻塞）
 *============================================================================*/

int uart_dev_send(uart_dev_t *dev, const uint8_t *data, size_t len, uint32_t timeout)
{
    if (dev == NULL || data == NULL || g_uart_ops.send_byte == NULL) {
        return UART_ERR_PARAM;
    }
    
    size_t sent = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (g_uart_ops.send_byte(dev->uart_handle, data[i], timeout) != UART_OK) {
            return sent > 0 ? (int)sent : UART_ERR_TIMEOUT;
        }
        sent++;
    }
    
    return (int)sent;
}

int uart_dev_recv(uart_dev_t *dev, uint8_t *data, size_t len, uint32_t timeout)
{
    if (dev == NULL || data == NULL || g_uart_ops.recv_byte == NULL) {
        return UART_ERR_PARAM;
    }
    
    size_t received = 0;
    
    for (size_t i = 0; i < len; i++) {
        if (g_uart_ops.recv_byte(dev->uart_handle, &data[i], timeout) != UART_OK) {
            return received > 0 ? (int)received : UART_ERR_TIMEOUT;
        }
        received++;
    }
    
    return (int)received;
}

int uart_dev_send_byte(uart_dev_t *dev, uint8_t data, uint32_t timeout)
{
    if (dev == NULL || g_uart_ops.send_byte == NULL) {
        return UART_ERR_PARAM;
    }
    
    return g_uart_ops.send_byte(dev->uart_handle, data, timeout);
}

int uart_dev_recv_byte(uart_dev_t *dev, uint8_t *data, uint32_t timeout)
{
    if (dev == NULL || data == NULL || g_uart_ops.recv_byte == NULL) {
        return UART_ERR_PARAM;
    }
    
    return g_uart_ops.recv_byte(dev->uart_handle, data, timeout);
}

int uart_dev_send_string(uart_dev_t *dev, const char *str, uint32_t timeout)
{
    if (dev == NULL || str == NULL) {
        return UART_ERR_PARAM;
    }
    
    return uart_dev_send(dev, (const uint8_t *)str, strlen(str), timeout);
}

/*============================================================================
 * 数据收发（非阻塞）
 *============================================================================*/

size_t uart_dev_write(uart_dev_t *dev, const uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL) {
        return 0;
    }
    
    // 写入发送缓冲区
    size_t written = ring_buffer_write(dev->tx_buffer, dev->tx_size,
                                       &dev->tx_head, &dev->tx_tail,
                                       data, len);
    
    // 如果发送器空闲，启动发送
    if (!dev->tx_busy && written > 0) {
        // 这里应该触发发送中断或启动 DMA
        dev->tx_busy = true;
    }
    
    return written;
}

size_t uart_dev_read(uart_dev_t *dev, uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL) {
        return 0;
    }
    
    return ring_buffer_read(dev->rx_buffer, dev->rx_size,
                            &dev->rx_head, &dev->rx_tail,
                            data, len);
}

size_t uart_dev_available(uart_dev_t *dev)
{
    if (dev == NULL) {
        return 0;
    }
    
    return ring_buffer_count(dev->rx_head, dev->rx_tail, dev->rx_size);
}

size_t uart_dev_tx_space(uart_dev_t *dev)
{
    if (dev == NULL) {
        return 0;
    }
    
    return ring_buffer_space(dev->tx_head, dev->tx_tail, dev->tx_size);
}

void uart_dev_flush_rx(uart_dev_t *dev)
{
    if (dev == NULL) {
        return;
    }
    
    dev->rx_head = 0;
    dev->rx_tail = 0;
    dev->rx_busy = false;
}

void uart_dev_flush_tx(uart_dev_t *dev)
{
    if (dev == NULL) {
        return;
    }
    
    dev->tx_head = 0;
    dev->tx_tail = 0;
    dev->tx_busy = false;
}

int uart_dev_wait_tx_complete(uart_dev_t *dev, uint32_t timeout)
{
    if (dev == NULL) {
        return UART_ERR_PARAM;
    }
    
    // 简单轮询（实际使用需要平台提供计时）
    uint32_t count = timeout / 10;  // 假设 10ms 一次
    
    while (count > 0) {
        if (!dev->tx_busy && dev->tx_head == dev->tx_tail) {
            return UART_OK;
        }
        
        // delay_ms(10);
        count--;
    }
    
    return UART_ERR_TIMEOUT;
}

/*============================================================================
 * 中断处理
 *============================================================================*/

void uart_dev_isr_rx(uart_dev_t *dev, uint8_t data)
{
    if (dev == NULL) {
        return;
    }
    
    // 写入接收缓冲区
    size_t next_head = (dev->rx_head + 1) % dev->rx_size;
    
    if (next_head != dev->rx_tail) {
        dev->rx_buffer[dev->rx_head] = data;
        dev->rx_head = next_head;
    } else {
        // 缓冲区溢出
        dev->rx_busy = false;
    }
}

size_t uart_dev_isr_tx(uart_dev_t *dev, uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL) {
        return 0;
    }
    
    // 从发送缓冲区读取数据
    return ring_buffer_read(dev->tx_buffer, dev->tx_size,
                            &dev->tx_head, &dev->tx_tail,
                            data, len);
}

void uart_dev_isr_error(uart_dev_t *dev, int error)
{
    if (dev == NULL) {
        return;
    }
    
    // 错误处理（可以记录日志或通知上层）
    (void)error;
}

/*============================================================================
 * 平台接口注册
 *============================================================================*/

void uart_dev_register_platform(const uart_platform_ops_t *ops)
{
    if (ops != NULL) {
        memcpy(&g_uart_ops, ops, sizeof(uart_platform_ops_t));
    }
}
