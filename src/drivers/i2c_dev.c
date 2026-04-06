/**
 * @file i2c_dev.c
 * @brief I2C 设备通用驱动框架实现
 */

#include "i2c_dev.h"
#include <string.h>

/*============================================================================
 * 全局变量
 *============================================================================*/

static i2c_platform_ops_t g_i2c_ops = {0};

/*============================================================================
 * 设备管理
 *============================================================================*/

int i2c_dev_init(i2c_dev_t *dev, void *i2c_handle, uint16_t dev_addr, uint32_t timeout)
{
    if (dev == NULL || i2c_handle == NULL) {
        return I2C_ERR_PARAM;
    }
    
    dev->i2c_handle = i2c_handle;
    dev->dev_addr = dev_addr & 0x7F;  // 确保 7 位地址
    dev->timeout = timeout;
    dev->auto_inc_addr = false;
    dev->user_data = NULL;
    
    return I2C_OK;
}

int i2c_dev_scan(i2c_dev_t *dev)
{
    if (dev == NULL || g_i2c_ops.write == NULL) {
        return I2C_ERR_PARAM;
    }
    
    // 尝试写入一个字节，检测是否响应
    uint8_t dummy = 0;
    int ret = g_i2c_ops.write(dev->i2c_handle, dev->dev_addr, &dummy, 0, dev->timeout);
    
    return (ret == 0) ? I2C_OK : I2C_ERR_NACK;
}

/*============================================================================
 * 寄存器操作
 *============================================================================*/

int i2c_dev_read_reg(i2c_dev_t *dev, uint8_t reg_addr, uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL || g_i2c_ops.write_read == NULL) {
        return I2C_ERR_PARAM;
    }
    
    return g_i2c_ops.write_read(dev->i2c_handle, dev->dev_addr,
                                &reg_addr, 1, data, len, dev->timeout);
}

int i2c_dev_write_reg(i2c_dev_t *dev, uint8_t reg_addr, const uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL || g_i2c_ops.write == NULL) {
        return I2C_ERR_PARAM;
    }
    
    // 分配缓冲区：寄存器地址 + 数据
    uint8_t *buffer = (uint8_t *)malloc(1 + len);
    if (buffer == NULL) {
        return I2C_ERR_PARAM;
    }
    
    buffer[0] = reg_addr;
    memcpy(&buffer[1], data, len);
    
    int ret = g_i2c_ops.write(dev->i2c_handle, dev->dev_addr, buffer, 1 + len, dev->timeout);
    
    free(buffer);
    return ret;
}

int i2c_dev_read_reg16(i2c_dev_t *dev, uint16_t reg_addr, uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL || g_i2c_ops.write_read == NULL) {
        return I2C_ERR_PARAM;
    }
    
    // 16 位寄存器地址（大端）
    uint8_t addr_buf[2];
    addr_buf[0] = (reg_addr >> 8) & 0xFF;
    addr_buf[1] = reg_addr & 0xFF;
    
    return g_i2c_ops.write_read(dev->i2c_handle, dev->dev_addr,
                                addr_buf, 2, data, len, dev->timeout);
}

int i2c_dev_write_reg16(i2c_dev_t *dev, uint16_t reg_addr, const uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL || g_i2c_ops.write == NULL) {
        return I2C_ERR_PARAM;
    }
    
    // 分配缓冲区：16 位寄存器地址 + 数据
    uint8_t *buffer = (uint8_t *)malloc(2 + len);
    if (buffer == NULL) {
        return I2C_ERR_PARAM;
    }
    
    buffer[0] = (reg_addr >> 8) & 0xFF;
    buffer[1] = reg_addr & 0xFF;
    memcpy(&buffer[2], data, len);
    
    int ret = g_i2c_ops.write(dev->i2c_handle, dev->dev_addr, buffer, 2 + len, dev->timeout);
    
    free(buffer);
    return ret;
}

int i2c_dev_modify_reg(i2c_dev_t *dev, uint8_t reg_addr, uint8_t mask, uint8_t value)
{
    if (dev == NULL) {
        return I2C_ERR_PARAM;
    }
    
    // 读取当前值
    uint8_t current;
    int ret = i2c_dev_read_reg8(dev, reg_addr, &current);
    if (ret != I2C_OK) {
        return ret;
    }
    
    // 修改特定位
    uint8_t new_value = (current & ~mask) | (value & mask);
    
    // 写回
    return i2c_dev_write_reg8(dev, reg_addr, new_value);
}

/*============================================================================
 * 数据读写
 *============================================================================*/

int i2c_dev_read(i2c_dev_t *dev, uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL || g_i2c_ops.read == NULL) {
        return I2C_ERR_PARAM;
    }
    
    return g_i2c_ops.read(dev->i2c_handle, dev->dev_addr, data, len, dev->timeout);
}

int i2c_dev_write(i2c_dev_t *dev, const uint8_t *data, size_t len)
{
    if (dev == NULL || data == NULL || g_i2c_ops.write == NULL) {
        return I2C_ERR_PARAM;
    }
    
    return g_i2c_ops.write(dev->i2c_handle, dev->dev_addr, data, len, dev->timeout);
}

/*============================================================================
 * 便捷函数
 *============================================================================*/

int i2c_dev_read_reg8(i2c_dev_t *dev, uint8_t reg_addr, uint8_t *value)
{
    return i2c_dev_read_reg(dev, reg_addr, value, 1);
}

int i2c_dev_read_reg16_be(i2c_dev_t *dev, uint8_t reg_addr, uint16_t *value)
{
    uint8_t buf[2];
    int ret = i2c_dev_read_reg(dev, reg_addr, buf, 2);
    if (ret != I2C_OK) {
        return ret;
    }
    *value = ((uint16_t)buf[0] << 8) | buf[1];
    return I2C_OK;
}

int i2c_dev_read_reg16_le(i2c_dev_t *dev, uint8_t reg_addr, uint16_t *value)
{
    uint8_t buf[2];
    int ret = i2c_dev_read_reg(dev, reg_addr, buf, 2);
    if (ret != I2C_OK) {
        return ret;
    }
    *value = ((uint16_t)buf[1] << 8) | buf[0];
    return I2C_OK;
}

int i2c_dev_write_reg8(i2c_dev_t *dev, uint8_t reg_addr, uint8_t value)
{
    return i2c_dev_write_reg(dev, reg_addr, &value, 1);
}

int i2c_dev_write_reg16_be(i2c_dev_t *dev, uint8_t reg_addr, uint16_t value)
{
    uint8_t buf[2];
    buf[0] = (value >> 8) & 0xFF;
    buf[1] = value & 0xFF;
    return i2c_dev_write_reg(dev, reg_addr, buf, 2);
}

int i2c_dev_write_reg16_le(i2c_dev_t *dev, uint8_t reg_addr, uint16_t value)
{
    uint8_t buf[2];
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    return i2c_dev_write_reg(dev, reg_addr, buf, 2);
}

/*============================================================================
 * 平台接口注册
 *============================================================================*/

void i2c_dev_register_platform(const i2c_platform_ops_t *ops)
{
    if (ops != NULL) {
        memcpy(&g_i2c_ops, ops, sizeof(i2c_platform_ops_t));
    }
}
