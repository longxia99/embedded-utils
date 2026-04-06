/**
 * @file test_i2c_dev.c
 * @brief I2C 设备框架单元测试（模拟平台层）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/drivers/i2c_dev.h"

/*============================================================================
 * 模拟 I2C 平台层
 *============================================================================*/

// 模拟 EEPROM 数据
static uint8_t simulated_eeprom[256];
static uint16_t simulated_reg_addr = 0;

/**
 * @brief 模拟 I2C 写操作
 */
static int mock_i2c_write(void *handle, uint16_t dev_addr, const uint8_t *data, size_t len, uint32_t timeout)
{
    (void)handle;
    (void)dev_addr;
    (void)timeout;
    
    // 简单模拟：第一个字节是寄存器地址
    if (len > 0) {
        simulated_reg_addr = data[0];
        
        // 剩余字节写入 EEPROM
        for (size_t i = 1; i < len; i++) {
            simulated_eeprom[simulated_reg_addr++] = data[i];
        }
    }
    
    return I2C_OK;
}

/**
 * @brief 模拟 I2C 读操作
 */
static int mock_i2c_read(void *handle, uint16_t dev_addr, uint8_t *data, size_t len, uint32_t timeout)
{
    (void)handle;
    (void)dev_addr;
    (void)timeout;
    
    // 从模拟 EEPROM 读取
    for (size_t i = 0; i < len; i++) {
        data[i] = simulated_eeprom[simulated_reg_addr++];
    }
    
    return I2C_OK;
}

/**
 * @brief 模拟 I2C 写 + 读操作
 */
static int mock_i2c_write_read(void *handle, uint16_t dev_addr, 
                               const uint8_t *write_data, size_t write_len,
                               uint8_t *read_data, size_t read_len, 
                               uint32_t timeout)
{
    (void)handle;
    (void)timeout;
    
    // 先写入寄存器地址
    if (write_len > 0) {
        simulated_reg_addr = write_data[0];
    }
    
    // 再读取数据
    for (size_t i = 0; i < read_len; i++) {
        read_data[i] = simulated_eeprom[simulated_reg_addr++];
    }
    
    return I2C_OK;
}

// 平台操作接口
static const i2c_platform_ops_t mock_ops = {
    .write = mock_i2c_write,
    .read = mock_i2c_read,
    .write_read = mock_i2c_write_read
};

/*============================================================================
 * 单元测试
 *============================================================================*/

static i2c_dev_t test_dev;

void test_init(void)
{
    printf("Test: i2c_dev_init... ");
    
    // 注册平台接口
    i2c_dev_register_platform(&mock_ops);
    
    // 初始化设备
    int ret = i2c_dev_init(&test_dev, (void *)0x1234, 0x50, 100);
    assert(ret == I2C_OK);
    
    // 验证设备信息
    assert(test_dev.dev_addr == 0x50);
    assert(test_dev.timeout == 100);
    
    // 错误参数
    ret = i2c_dev_init(NULL, (void *)0x1234, 0x50, 100);
    assert(ret == I2C_ERR_PARAM);
    
    printf("PASSED\n");
}

void test_scan(void)
{
    printf("Test: i2c_dev_scan... ");
    
    int ret = i2c_dev_scan(&test_dev);
    assert(ret == I2C_OK);  // 模拟设备总是存在
    
    printf("PASSED\n");
}

void test_write_read_reg8(void)
{
    printf("Test: write/read reg8... ");
    
    // 写入一个字节
    int ret = i2c_dev_write_reg8(&test_dev, 0x10, 0x5A);
    assert(ret == I2C_OK);
    
    // 读取一个字节
    uint8_t value;
    ret = i2c_dev_read_reg8(&test_dev, 0x10, &value);
    assert(ret == I2C_OK);
    assert(value == 0x5A);
    
    printf("PASSED\n");
}

void test_write_read_reg16_be(void)
{
    printf("Test: write/read reg16_be... ");
    
    // 写入 16 位（大端）
    int ret = i2c_dev_write_reg16_be(&test_dev, 0x20, 0x1234);
    assert(ret == I2C_OK);
    
    // 读取 16 位（大端）
    uint16_t value;
    ret = i2c_dev_read_reg16_be(&test_dev, 0x20, &value);
    assert(ret == I2C_OK);
    assert(value == 0x1234);
    
    printf("PASSED\n");
}

void test_write_read_reg16_le(void)
{
    printf("Test: write/read reg16_le... ");
    
    // 写入 16 位（小端）
    int ret = i2c_dev_write_reg16_le(&test_dev, 0x30, 0xABCD);
    assert(ret == I2C_OK);
    
    // 读取 16 位（小端）
    uint16_t value;
    ret = i2c_dev_read_reg16_le(&test_dev, 0x30, &value);
    assert(ret == I2C_OK);
    assert(value == 0xABCD);
    
    printf("PASSED\n");
}

void test_modify_reg(void)
{
    printf("Test: modify register... ");
    
    // 先写入初始值
    i2c_dev_write_reg8(&test_dev, 0x40, 0xFF);
    
    // 修改低 4 位为 0x0A，保留高 4 位
    int ret = i2c_dev_modify_reg(&test_dev, 0x40, 0x0F, 0x0A);
    assert(ret == I2C_OK);
    
    // 读取验证
    uint8_t value;
    ret = i2c_dev_read_reg8(&test_dev, 0x40, &value);
    assert(ret == I2C_OK);
    assert(value == 0xFA);  // 0xFF & ~0x0F | 0x0A = 0xF0 | 0x0A = 0xFA
    
    printf("PASSED\n");
}

void test_multi_bytes(void)
{
    printf("Test: multi-bytes read/write... ");
    
    // 写入多字节
    uint8_t write_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    int ret = i2c_dev_write_reg(&test_dev, 0x50, write_data, 5);
    assert(ret == I2C_OK);
    
    // 读取多字节
    uint8_t read_data[5];
    ret = i2c_dev_read_reg(&test_dev, 0x50, read_data, 5);
    assert(ret == I2C_OK);
    assert(memcmp(read_data, write_data, 5) == 0);
    
    printf("PASSED\n");
}

void test_16bit_reg_addr(void)
{
    printf("Test: 16-bit register address... ");
    
    // 写入 16 位寄存器地址
    uint8_t write_data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    int ret = i2c_dev_write_reg16(&test_dev, 0x1000, write_data, 4);
    assert(ret == I2C_OK);
    
    // 读取
    uint8_t read_data[4];
    ret = i2c_dev_read_reg16(&test_dev, 0x1000, read_data, 4);
    assert(ret == I2C_OK);
    assert(memcmp(read_data, write_data, 4) == 0);
    
    printf("PASSED\n");
}

int main(void)
{
    printf("=== I2C Device Framework Unit Tests ===\n\n");
    
    // 清空模拟 EEPROM
    memset(simulated_eeprom, 0, sizeof(simulated_eeprom));
    
    test_init();
    test_scan();
    test_write_read_reg8();
    test_write_read_reg16_be();
    test_write_read_reg16_le();
    test_modify_reg();
    test_multi_bytes();
    test_16bit_reg_addr();
    
    printf("\n=== All tests PASSED ===\n");
    
    return 0;
}
