/**
 * @file test_spi_dev.c
 * @brief SPI 设备框架单元测试（模拟平台层）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/drivers/spi_dev.h"

/*============================================================================
 * 模拟 SPI 平台层
 *============================================================================*/

// 模拟 SPI 缓冲区（全双工）
static uint8_t simulated_spi_buffer[256];
static size_t simulated_spi_len = 0;
static bool simulated_cs_active = false;

/**
 * @brief 模拟 SPI 配置
 */
static int mock_spi_configure(void *handle, uint32_t freq_hz, uint8_t mode, uint8_t data_bits)
{
    (void)handle;
    (void)freq_hz;
    (void)mode;
    (void)data_bits;
    return SPI_OK;
}

/**
 * @brief 模拟 SPI 片选有效
 */
static void mock_spi_cs_select(void *cs_pin)
{
    (void)cs_pin;
    simulated_cs_active = true;
    simulated_spi_len = 0;
}

/**
 * @brief 模拟 SPI 片选无效
 */
static void mock_spi_cs_deselect(void *cs_pin)
{
    (void)cs_pin;
    simulated_cs_active = false;
}

/**
 * @brief 模拟 SPI 写操作
 */
static int mock_spi_write(void *handle, const uint8_t *data, size_t len, uint32_t timeout)
{
    (void)handle;
    (void)timeout;
    
    if (!simulated_cs_active) {
        return SPI_ERR_CS;
    }
    
    // 保存到模拟缓冲区
    if (simulated_spi_len + len <= sizeof(simulated_spi_buffer)) {
        memcpy(&simulated_spi_buffer[simulated_spi_len], data, len);
        simulated_spi_len += len;
    }
    
    return SPI_OK;
}

/**
 * @brief 模拟 SPI 读操作
 */
static int mock_spi_read(void *handle, uint8_t *data, size_t len, uint32_t timeout)
{
    (void)handle;
    (void)timeout;
    
    if (!simulated_cs_active) {
        return SPI_ERR_CS;
    }
    
    // 从模拟缓冲区读取（简单模拟）
    memset(data, 0xFF, len);  // 默认返回 0xFF
    
    return SPI_OK;
}

/**
 * @brief 模拟 SPI 全双工传输
 */
static int mock_spi_transfer(void *handle, const uint8_t *write_data, uint8_t *read_data, 
                             size_t len, uint32_t timeout)
{
    (void)handle;
    (void)timeout;
    
    if (!simulated_cs_active) {
        return SPI_ERR_CS;
    }
    
    // 写入数据
    if (write_data != NULL) {
        if (simulated_spi_len + len <= sizeof(simulated_spi_buffer)) {
            memcpy(&simulated_spi_buffer[simulated_spi_len], write_data, len);
            simulated_spi_len += len;
        }
    }
    
    // 读取数据（模拟）
    if (read_data != NULL) {
        memset(read_data, 0xFF, len);
    }
    
    return SPI_OK;
}

// 平台操作接口
static const spi_platform_ops_t mock_ops = {
    .configure = mock_spi_configure,
    .cs_select = mock_spi_cs_select,
    .cs_deselect = mock_spi_cs_deselect,
    .write = mock_spi_write,
    .read = mock_spi_read,
    .transfer = mock_spi_transfer
};

/*============================================================================
 * 单元测试
 *============================================================================*/

static spi_dev_t test_dev;

void test_init(void)
{
    printf("Test: spi_dev_init... ");
    
    // 注册平台接口
    spi_dev_register_platform(&mock_ops);
    
    // 初始化设备
    int ret = spi_dev_init(&test_dev, (void *)0x1234, (void *)0x5678, 10000000, SPI_MODE_0, 100);
    assert(ret == SPI_OK);
    
    // 验证设备信息
    assert(test_dev.freq_hz == 10000000);
    assert(test_dev.mode == SPI_MODE_0);
    assert(test_dev.timeout == 100);
    
    // 错误参数
    ret = spi_dev_init(NULL, (void *)0x1234, (void *)0x5678, 10000000, SPI_MODE_0, 100);
    assert(ret == SPI_ERR_PARAM);
    
    printf("PASSED\n");
}

void test_set_freq_mode(void)
{
    printf("Test: set frequency/mode... ");
    
    // 设置频率
    int ret = spi_dev_set_freq(&test_dev, 20000000);
    assert(ret == SPI_OK);
    assert(test_dev.freq_hz == 20000000);
    
    // 设置模式
    ret = spi_dev_set_mode(&test_dev, SPI_MODE_3);
    assert(ret == SPI_OK);
    assert(test_dev.mode == SPI_MODE_3);
    
    printf("PASSED\n");
}

void test_write_read(void)
{
    printf("Test: write/read... ");
    
    // 写入数据
    uint8_t write_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    int ret = spi_dev_write(&test_dev, write_data, sizeof(write_data));
    assert(ret == SPI_OK);
    
    // 验证数据被写入模拟缓冲区
    assert(simulated_spi_len == sizeof(write_data));
    assert(memcmp(simulated_spi_buffer, write_data, sizeof(write_data)) == 0);
    
    // 读取数据
    uint8_t read_data[5];
    ret = spi_dev_read(&test_dev, read_data, sizeof(read_data));
    assert(ret == SPI_OK);
    
    printf("PASSED\n");
}

void test_transfer(void)
{
    printf("Test: full-duplex transfer... ");
    
    // 全双工传输
    uint8_t write_data[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t read_data[4];
    
    int ret = spi_dev_transfer(&test_dev, write_data, read_data, sizeof(write_data));
    assert(ret == SPI_OK);
    
    // 验证写入
    assert(memcmp(&simulated_spi_buffer[simulated_spi_len - 4], write_data, 4) == 0);
    
    // 验证读取（模拟返回 0xFF）
    for (int i = 0; i < 4; i++) {
        assert(read_data[i] == 0xFF);
    }
    
    printf("PASSED\n");
}

void test_byte_operations(void)
{
    printf("Test: byte operations... ");
    
    // 写入一个字节
    int ret = spi_dev_write_byte(&test_dev, 0x5A);
    assert(ret == SPI_OK);
    
    // 读取一个字节
    uint8_t read_byte;
    ret = spi_dev_read_byte(&test_dev, &read_byte);
    assert(ret == SPI_OK);
    assert(read_byte == 0xFF);  // 模拟返回 0xFF
    
    printf("PASSED\n");
}

void test_cmd(void)
{
    printf("Test: command mode... ");
    
    // 发送命令 + 写入数据 + 读取数据
    uint8_t cmd = 0x9F;  // 读 ID 命令
    uint8_t write_data[] = {0x00, 0x00, 0x00};  // 地址
    uint8_t read_data[3];
    
    int ret = spi_dev_cmd(&test_dev, cmd, write_data, sizeof(write_data), read_data, sizeof(read_data));
    assert(ret == SPI_OK);
    
    // 验证命令和地址被发送
    assert(simulated_spi_buffer[simulated_spi_len - 6] == cmd);
    assert(memcmp(&simulated_spi_buffer[simulated_spi_len - 5], write_data, 3) == 0);
    
    printf("PASSED\n");
}

void test_read_id_status(void)
{
    printf("Test: read ID/status... ");
    
    // 读取 ID
    uint8_t id[3];
    int ret = spi_dev_read_id(&test_dev, 0x9F, id, sizeof(id));
    assert(ret == SPI_OK);
    
    // 读取状态
    uint8_t status;
    ret = spi_dev_read_status(&test_dev, 0x05, &status);
    assert(ret == SPI_OK);
    
    printf("PASSED\n");
}

void test_write_config(void)
{
    printf("Test: write config... ");
    
    // 写入配置
    uint8_t config = 0x3C;
    int ret = spi_dev_write_config(&test_dev, 0x01, config);
    assert(ret == SPI_OK);
    
    printf("PASSED\n");
}

void test_cs_handling(void)
{
    printf("Test: chip select handling... ");
    
    // 验证片选在操作前被拉低，操作后被拉高
    assert(simulated_cs_active == false);  // 初始状态
    
    spi_dev_write_byte(&test_dev, 0x55);
    
    assert(simulated_cs_active == false);  // 操作后应该释放
    
    printf("PASSED\n");
}

int main(void)
{
    printf("=== SPI Device Framework Unit Tests ===\n\n");
    
    // 清空模拟缓冲区
    memset(simulated_spi_buffer, 0, sizeof(simulated_spi_buffer));
    simulated_spi_len = 0;
    simulated_cs_active = false;
    
    test_init();
    test_set_freq_mode();
    test_write_read();
    test_transfer();
    test_byte_operations();
    test_cmd();
    test_read_id_status();
    test_write_config();
    test_cs_handling();
    
    printf("\n=== All tests PASSED ===\n");
    
    return 0;
}
