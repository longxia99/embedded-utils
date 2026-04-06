/**
 * @file test_ring_buffer.c
 * @brief 环形缓冲区单元测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/utils/ring_buffer.h"

#define TEST_BUFFER_SIZE  16

static uint8_t test_buffer[TEST_BUFFER_SIZE];
static ring_buffer_t rb;

void test_init(void)
{
    printf("Test: ring_buffer_init... ");
    
    // 正常初始化
    assert(ring_buffer_init(&rb, test_buffer, TEST_BUFFER_SIZE) == 0);
    assert(ring_buffer_is_empty(&rb) == true);
    assert(ring_buffer_available(&rb) == 0);
    assert(ring_buffer_space(&rb) == TEST_BUFFER_SIZE - 1);
    
    // 错误参数
    assert(ring_buffer_init(NULL, test_buffer, TEST_BUFFER_SIZE) == -1);
    assert(ring_buffer_init(&rb, NULL, TEST_BUFFER_SIZE) == -1);
    assert(ring_buffer_init(&rb, test_buffer, 0) == -1);
    
    printf("PASSED\n");
}

void test_write_read(void)
{
    printf("Test: write/read... ");
    
    ring_buffer_init(&rb, test_buffer, TEST_BUFFER_SIZE);
    
    uint8_t write_data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    uint8_t read_data[5] = {0};
    
    // 写入 5 字节
    assert(ring_buffer_write(&rb, write_data, 5) == 5);
    assert(ring_buffer_available(&rb) == 5);
    assert(ring_buffer_is_empty(&rb) == false);
    
    // 读取 5 字节
    assert(ring_buffer_read(&rb, read_data, 5) == 5);
    assert(memcmp(write_data, read_data, 5) == 0);
    assert(ring_buffer_is_empty(&rb) == true);
    
    printf("PASSED\n");
}

void test_overflow(void)
{
    printf("Test: overflow handling... ");
    
    ring_buffer_init(&rb, test_buffer, TEST_BUFFER_SIZE);
    
    // 写入 15 字节（最大容量）
    uint8_t write_data[15];
    for (int i = 0; i < 15; i++) {
        write_data[i] = i;
    }
    
    assert(ring_buffer_write(&rb, write_data, 15) == 15);
    assert(ring_buffer_is_full(&rb) == true);
    assert(ring_buffer_space(&rb) == 0);
    
    // 尝试再写入 1 字节，应该失败
    uint8_t extra = 0xFF;
    assert(ring_buffer_write(&rb, &extra, 1) == 0);
    assert(rb.overflow == true);
    
    printf("PASSED\n");
}

void test_wrap_around(void)
{
    printf("Test: wrap around... ");
    
    ring_buffer_init(&rb, test_buffer, TEST_BUFFER_SIZE);
    
    // 写入 10 字节
    uint8_t write_data[10];
    for (int i = 0; i < 10; i++) {
        write_data[i] = i;
    }
    ring_buffer_write(&rb, write_data, 10);
    
    // 读取 5 字节
    uint8_t read_data[5];
    ring_buffer_read(&rb, read_data, 5);
    
    // 再写入 10 字节（会回绕）
    ring_buffer_write(&rb, write_data, 10);
    
    // 读取剩余的 15 字节
    uint8_t final_data[15];
    size_t read = ring_buffer_read(&rb, final_data, 15);
    assert(read == 15);
    
    // 验证数据
    assert(memcmp(final_data, write_data, 5) == 0);  // 第一次的后 5 字节
    assert(memcmp(&final_data[5], write_data, 10) == 0);  // 第二次的全部
    
    printf("PASSED\n");
}

void test_byte_operations(void)
{
    printf("Test: byte operations... ");
    
    ring_buffer_init(&rb, test_buffer, TEST_BUFFER_SIZE);
    
    // 单字节写入
    for (int i = 0; i < 10; i++) {
        assert(ring_buffer_write_byte(&rb, i) == 0);
    }
    
    assert(ring_buffer_available(&rb) == 10);
    
    // 单字节读取
    for (int i = 0; i < 10; i++) {
        uint8_t data;
        assert(ring_buffer_read_byte(&rb, &data) == 0);
        assert(data == i);
    }
    
    // 空缓冲区读取失败
    uint8_t data;
    assert(ring_buffer_read_byte(&rb, &data) == -1);
    
    printf("PASSED\n");
}

void test_clear(void)
{
    printf("Test: clear... ");
    
    ring_buffer_init(&rb, test_buffer, TEST_BUFFER_SIZE);
    
    // 写入一些数据
    uint8_t data = 0x55;
    ring_buffer_write(&rb, &data, 1);
    assert(ring_buffer_is_empty(&rb) == false);
    
    // 清空
    ring_buffer_clear(&rb);
    assert(ring_buffer_is_empty(&rb) == true);
    assert(ring_buffer_available(&rb) == 0);
    
    printf("PASSED\n");
}

int main(void)
{
    printf("=== Ring Buffer Unit Tests ===\n\n");
    
    test_init();
    test_write_read();
    test_overflow();
    test_wrap_around();
    test_byte_operations();
    test_clear();
    
    printf("\n=== All tests PASSED ===\n");
    
    return 0;
}
