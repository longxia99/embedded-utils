/**
 * @file spi_dev.c
 * @brief SPI 设备通用驱动框架实现
 */

#include "spi_dev.h"
#include <string.h>
#include <stdlib.h>

/*============================================================================
 * 全局变量
 *============================================================================*/

static spi_platform_ops_t g_spi_ops = {0};

/*============================================================================
 * 设备管理
 *============================================================================*/

int spi_dev_init(spi_dev_t *dev, void *spi_handle, void *cs_pin, 
                 uint32_t freq_hz, uint8_t mode, uint32_t timeout)
{
    if (dev == NULL || spi_handle == NULL) {
        return SPI_ERR_PARAM;
    }
    
    dev->spi_handle = spi_handle;
    dev->cs_pin = cs_pin;
    dev->freq_hz = freq_hz;
    dev->mode = mode & 0x03;
    dev->data_bits = 8;  // 默认 8 位
    dev->timeout = timeout;
    dev->msb_first = true;  // 默认 MSB 优先
    dev->user_data = NULL;
    
    // 配置 SPI 参数
    if (g_spi_ops.configure != NULL) {
        return g_spi_ops.configure(spi_handle, freq_hz, mode, dev->data_bits);
    }
    
    return SPI_OK;
}

int spi_dev_set_freq(spi_dev_t *dev, uint32_t freq_hz)
{
    if (dev == NULL || g_spi_ops.configure == NULL) {
        return SPI_ERR_PARAM;
    }
    
    dev->freq_hz = freq_hz;
    return g_spi_ops.configure(dev->spi_handle, freq_hz, dev->mode, dev->data_bits);
}

int spi_dev_set_mode(spi_dev_t *dev, uint8_t mode)
{
    if (dev == NULL || g_spi_ops.configure == NULL) {
        return SPI_ERR_PARAM;
    }
    
    dev->mode = mode & 0x03;
    return g_spi_ops.configure(dev->spi_handle, dev->freq_hz, dev->mode, dev->data_bits);
}

/*============================================================================
 * 数据传输
 *============================================================================*/

int spi_dev_write(spi_dev_t *dev, const uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL || g_spi_ops.write == NULL) {
        return SPI_ERR_PARAM;
    }
    
    // 片选有效
    if (g_spi_ops.cs_select != NULL) {
        g_spi_ops.cs_select(dev->cs_pin);
    }
    
    int ret = g_spi_ops.write(dev->spi_handle, data, len, dev->timeout);
    
    // 片选无效
    if (g_spi_ops.cs_deselect != NULL) {
        g_spi_ops.cs_deselect(dev->cs_pin);
    }
    
    return ret;
}

int spi_dev_read(spi_dev_t *dev, uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL || g_spi_ops.read == NULL) {
        return SPI_ERR_PARAM;
    }
    
    // 片选有效
    if (g_spi_ops.cs_select != NULL) {
        g_spi_ops.cs_select(dev->cs_pin);
    }
    
    int ret = g_spi_ops.read(dev->spi_handle, data, len, dev->timeout);
    
    // 片选无效
    if (g_spi_ops.cs_deselect != NULL) {
        g_spi_ops.cs_deselect(dev->cs_pin);
    }
    
    return ret;
}

int spi_dev_transfer(spi_dev_t *dev, const uint8_t *write_data, uint8_t *read_data, size_t len)
{
    if (dev == NULL || g_spi_ops.transfer == NULL) {
        return SPI_ERR_PARAM;
    }
    
    // 片选有效
    if (g_spi_ops.cs_select != NULL) {
        g_spi_ops.cs_select(dev->cs_pin);
    }
    
    int ret = g_spi_ops.transfer(dev->spi_handle, write_data, read_data, len, dev->timeout);
    
    // 片选无效
    if (g_spi_ops.cs_deselect != NULL) {
        g_spi_ops.cs_deselect(dev->cs_pin);
    }
    
    return ret;
}

int spi_dev_write_byte(spi_dev_t *dev, uint8_t data)
{
    return spi_dev_write(dev, &data, 1);
}

int spi_dev_read_byte(spi_dev_t *dev, uint8_t *data)
{
    return spi_dev_read(dev, data, 1);
}

int spi_dev_cmd(spi_dev_t *dev, uint8_t cmd, const uint8_t *write_data, size_t write_len,
                uint8_t *read_data, size_t read_len)
{
    if (dev == NULL) {
        return SPI_ERR_PARAM;
    }
    
    int ret;
    
    // 片选有效
    if (g_spi_ops.cs_select != NULL) {
        g_spi_ops.cs_select(dev->cs_pin);
    }
    
    // 发送命令
    ret = g_spi_ops.write(dev->spi_handle, &cmd, 1, dev->timeout);
    if (ret != SPI_OK) {
        goto done;
    }
    
    // 写入后续数据
    if (write_data != NULL && write_len > 0) {
        ret = g_spi_ops.write(dev->spi_handle, write_data, write_len, dev->timeout);
        if (ret != SPI_OK) {
            goto done;
        }
    }
    
    // 读取数据
    if (read_data != NULL && read_len > 0) {
        ret = g_spi_ops.read(dev->spi_handle, read_data, read_len, dev->timeout);
        if (ret != SPI_OK) {
            goto done;
        }
    }
    
done:
    // 片选无效
    if (g_spi_ops.cs_deselect != NULL) {
        g_spi_ops.cs_deselect(dev->cs_pin);
    }
    
    return ret;
}

/*============================================================================
 * 便捷函数
 *============================================================================*/

int spi_dev_read_id(spi_dev_t *dev, uint8_t cmd, uint8_t *id, size_t id_len)
{
    return spi_dev_cmd(dev, cmd, NULL, 0, id, id_len);
}

int spi_dev_read_status(spi_dev_t *dev, uint8_t cmd, uint8_t *status)
{
    return spi_dev_cmd(dev, cmd, NULL, 0, status, 1);
}

int spi_dev_write_config(spi_dev_t *dev, uint8_t cmd, uint8_t value)
{
    return spi_dev_cmd(dev, cmd, &value, 1, NULL, 0);
}

int spi_dev_wait_ready(spi_dev_t *dev, uint8_t cmd, uint8_t ready_mask, 
                       uint8_t ready_value, uint32_t timeout_ms)
{
    if (dev == NULL) {
        return SPI_ERR_PARAM;
    }
    
    uint32_t start_time = 0;  // 需要平台提供计时函数
    uint8_t status;
    int ret;
    
    // 简单轮询（实际使用需要平台提供计时）
    for (int i = 0; i < timeout_ms / 10; i++) {
        ret = spi_dev_read_status(dev, cmd, &status);
        if (ret != SPI_OK) {
            return ret;
        }
        
        if ((status & ready_mask) == ready_value) {
            return SPI_OK;
        }
        
        // 延时 10ms（需要平台提供）
        // delay_ms(10);
    }
    
    return SPI_ERR_TIMEOUT;
}

/*============================================================================
 * 平台接口注册
 *============================================================================*/

void spi_dev_register_platform(const spi_platform_ops_t *ops)
{
    if (ops != NULL) {
        memcpy(&g_spi_ops, ops, sizeof(spi_platform_ops_t));
    }
}
