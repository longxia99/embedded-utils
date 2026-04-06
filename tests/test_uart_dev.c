/**
 * @file test_uart_dev.c
 * @brief UART 设备框架单元测试（模拟平台层）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/drivers/uart_dev.h"

/*============================================================================
 * 模拟 UART 平台层
 *============================================================================*/

static uint8_t simulated_uart_tx_buffer[256];
static size_t simulated_uart_tx_count = 0;
static uint8_t simulated_uart_rx_data = 0;
static bool simulated_uart_rx_ready = false;

/**
 * @brief 模拟 UART 初始化
 */
static int mock_uart_init(void *handle, uint32_t baudrate, uart_data_bits_t data_bits,
                          uart_stop_bits_t stop_bits, uart_parity_t parity)
{
    (void)handle;
    (void)baudrate;
    (void)data_bits;
    (void)stop_bits;
    (void)parity;
    return UART_OK;
}

/**
 * @brief 模拟 UART 发送一个字节
 */
static int mock_uart_send_byte(void *handle, uint8_t data, uint32_t timeout)
{
    (void)handle;
    (void)timeout;
    
    if (simulated_uart_tx_count < sizeof(simulated_uart_tx_buffer)) {
        simulated_uart_tx_buffer[simulated_uart_tx_count++] = data;
        return UART_OK;
    }
    
    return UART_ERR_BUSY;
}

/**
 * @brief 模拟 UART 接收一个字节
 */
static int mock_uart_recv_byte(void *handle, uint8_t *data, uint32_t timeout)
{
    (void)handle;
    (void)timeout;
    
    if (simulated_uart_rx_ready) {
        *data = simulated_uart_rx_data;
        simulated_uart_rx_ready = false;
        return UART_OK;
    }
    
    return UART_ERR_TIMEOUT;
}

// 平台操作接口
static const uart_platform_ops_t mock_ops = {
    .init = mock_uart_init,
    .deinit = NULL,
    .send_byte = mock_uart_send_byte,
    .recv_byte = mock_uart_recv_byte,
    .dma_recv_start = NULL,
    .dma_send_start = NULL,
    .dma_get_count = NULL,
    .irq_handler = NULL
};

/*============================================================================
 * 单元测试
 *============================================================================*/

static uart_dev_t test_dev;
static uint8_t rx_buffer[128];
static uint8_t tx_buffer[128];

void test_init(void)
{
    printf("Test: uart_dev_init... ");
    
    // 注册平台接口
    uart_dev_register_platform(&mock_ops);
    
    // 初始化设备
    int ret = uart_dev_init(&test_dev, (void *)0x1234, 
                            rx_buffer, sizeof(rx_buffer),
                            tx_buffer, sizeof(tx_buffer),
                            115200);
    assert(ret == UART_OK);
    
    // 验证设备信息
    assert(test_dev.baudrate == 115200);
    assert(test_dev.rx_size == sizeof(rx_buffer));
    assert(test_dev.tx_size == sizeof(tx_buffer));
    
    // 错误参数
    ret = uart_dev_init(NULL, (void *)0x1234, rx_buffer, sizeof(rx_buffer),
                        tx_buffer, sizeof(tx_buffer), 115200);
    assert(ret == UART_ERR_PARAM);
    
    printf("PASSED\n");
}

void test_set_baudrate(void)
{
    printf("Test: set baudrate... ");
    
    int ret = uart_dev_set_baudrate(&test_dev, 9600);
    assert(ret == UART_OK);
    assert(test_dev.baudrate == 9600);
    
    ret = uart_dev_set_baudrate(&test_dev, 1000000);
    assert(ret == UART_OK);
    assert(test_dev.baudrate == 1000000);
    
    printf("PASSED\n");
}

void test_send_recv_byte(void)
{
    printf("Test: send/recv byte... ");
    
    // 发送一个字节
    int ret = uart_dev_send_byte(&test_dev, 0x5A, 100);
    assert(ret == UART_OK);
    assert(simulated_uart_tx_count == 1);
    assert(simulated_uart_tx_buffer[0] == 0x5A);
    
    // 接收一个字节（模拟有数据）
    simulated_uart_rx_data = 0xA5;
    simulated_uart_rx_ready = true;
    
    uint8_t recv_data;
    ret = uart_dev_recv_byte(&test_dev, &recv_data, 100);
    assert(ret == UART_OK);
    assert(recv_data == 0xA5);
    
    printf("PASSED\n");
}

void test_send_string(void)
{
    printf("Test: send string... ");
    
    // 清空模拟缓冲区
    simulated_uart_tx_count = 0;
    
    // 发送字符串
    const char *test_str = "Hello UART";
    int ret = uart_dev_send_string(&test_dev, test_str, 100);
    assert(ret == (int)strlen(test_str));
    assert(simulated_uart_tx_count == strlen(test_str));
    assert(memcmp(simulated_uart_tx_buffer, test_str, strlen(test_str)) == 0);
    
    printf("PASSED\n");
}

void test_send_recv_bulk(void)
{
    printf("Test: bulk send/recv... ");
    
    // 清空
    simulated_uart_tx_count = 0;
    
    // 批量发送
    uint8_t send_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    int ret = uart_dev_send(&test_dev, send_data, sizeof(send_data), 100);
    assert(ret == (int)sizeof(send_data));
    assert(simulated_uart_tx_count == sizeof(send_data));
    assert(memcmp(simulated_uart_tx_buffer, send_data, sizeof(send_data)) == 0);
    
    printf("PASSED\n");
}

void test_write_read_nonblocking(void)
{
    printf("Test: non-blocking write/read... ");
    
    // 清空缓冲区
    uart_dev_flush_rx(&test_dev);
    uart_dev_flush_tx(&test_dev);
    
    // 非阻塞写入
    uint8_t write_data[] = {0x11, 0x22, 0x33};
    size_t written = uart_dev_write(&test_dev, write_data, sizeof(write_data));
    assert(written == sizeof(write_data));
    
    // 查询可用数据（应该为 0，因为还没模拟接收）
    size_t available = uart_dev_available(&test_dev);
    assert(available == 0);
    
    // 模拟接收中断
    uart_dev_isr_rx(&test_dev, 0xAA);
    uart_dev_isr_rx(&test_dev, 0xBB);
    
    // 查询可用数据
    available = uart_dev_available(&test_dev);
    assert(available == 2);
    
    // 非阻塞读取
    uint8_t read_data[4];
    size_t read = uart_dev_read(&test_dev, read_data, sizeof(read_data));
    assert(read == 2);
    assert(read_data[0] == 0xAA);
    assert(read_data[1] == 0xBB);
    
    printf("PASSED\n");
}

void test_flush(void)
{
    printf("Test: flush buffers... ");
    
    // 写入一些数据
    uart_dev_write(&test_dev, (const uint8_t *)"test", 4);
    
    // 模拟接收
    uart_dev_isr_rx(&test_dev, 0x01);
    uart_dev_isr_rx(&test_dev, 0x02);
    
    // 刷新
    uart_dev_flush_rx(&test_dev);
    uart_dev_flush_tx(&test_dev);
    
    // 验证清空
    assert(uart_dev_available(&test_dev) == 0);
    assert(uart_dev_tx_space(&test_dev) == test_dev.tx_size - 1);
    
    printf("PASSED\n");
}

void test_isr(void)
{
    printf("Test: ISR handlers... ");
    
    // 清空
    uart_dev_flush_rx(&test_dev);
    
    // 模拟多个接收中断
    for (int i = 0; i < 10; i++) {
        uart_dev_isr_rx(&test_dev, i);
    }
    
    // 验证接收缓冲区
    assert(uart_dev_available(&test_dev) == 10);
    
    uint8_t data[10];
    size_t read = uart_dev_read(&test_dev, data, sizeof(data));
    assert(read == 10);
    
    for (int i = 0; i < 10; i++) {
        assert(data[i] == i);
    }
    
    // 测试发送中断处理
    uart_dev_flush_tx(&test_dev);
    uart_dev_write(&test_dev, (const uint8_t *)"ABC", 3);
    
    uint8_t tx_data;
    size_t count = uart_dev_isr_tx(&test_dev, &tx_data, 1);
    assert(count == 1);
    assert(tx_data == 'A');
    
    printf("PASSED\n");
}

void test_buffer_overflow(void)
{
    printf("Test: buffer overflow handling... ");
    
    uart_dev_flush_rx(&test_dev);
    
    // 填满接收缓冲区
    for (size_t i = 0; i < test_dev.rx_size - 1; i++) {
        uart_dev_isr_rx(&test_dev, i & 0xFF);
    }
    
    // 再写入应该失败（溢出）
    size_t before_count = uart_dev_available(&test_dev);
    uart_dev_isr_rx(&test_dev, 0xFF);  // 应该被丢弃
    size_t after_count = uart_dev_available(&test_dev);
    
    assert(before_count == after_count);  // 数量不变
    
    printf("PASSED\n");
}

int main(void)
{
    printf("=== UART Device Framework Unit Tests ===\n\n");
    
    // 清空模拟数据
    memset(simulated_uart_tx_buffer, 0, sizeof(simulated_uart_tx_buffer));
    simulated_uart_tx_count = 0;
    simulated_uart_rx_ready = false;
    
    test_init();
    test_set_baudrate();
    test_send_recv_byte();
    test_send_string();
    test_send_recv_bulk();
    test_write_read_nonblocking();
    test_flush();
    test_isr();
    test_buffer_overflow();
    
    printf("\n=== All tests PASSED ===\n");
    
    return 0;
}
