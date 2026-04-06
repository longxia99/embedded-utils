/**
 * @file test_fifo.c
 * @brief FIFO 单元测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/utils/fifo.h"

#define FIFO_ITEM_COUNT  8
#define FIFO_ITEM_SIZE   sizeof(int)

static uint8_t fifo_buffer[FIFO_ITEM_COUNT * FIFO_ITEM_SIZE];
static fifo_t fifo;

void test_init(void)
{
    printf("Test: fifo_init... ");
    
    // 正常初始化
    assert(fifo_init(&fifo, fifo_buffer, FIFO_ITEM_SIZE, FIFO_ITEM_COUNT) == 0);
    assert(fifo_is_empty(&fifo) == true);
    assert(fifo_count(&fifo) == 0);
    assert(fifo_space(&fifo) == FIFO_ITEM_COUNT);
    
    // 错误参数
    assert(fifo_init(NULL, fifo_buffer, FIFO_ITEM_SIZE, FIFO_ITEM_COUNT) == -1);
    assert(fifo_init(&fifo, NULL, FIFO_ITEM_SIZE, FIFO_ITEM_COUNT) == -1);
    assert(fifo_init(&fifo, fifo_buffer, 0, FIFO_ITEM_COUNT) == -1);
    assert(fifo_init(&fifo, fifo_buffer, FIFO_ITEM_SIZE, 0) == -1);
    
    printf("PASSED\n");
}

void test_push_pop(void)
{
    printf("Test: push/pop... ");
    
    fifo_init(&fifo, fifo_buffer, FIFO_ITEM_SIZE, FIFO_ITEM_COUNT);
    
    // 写入 5 个元素
    for (int i = 0; i < 5; i++) {
        assert(fifo_push(&fifo, &i) == 0);
    }
    assert(fifo_count(&fifo) == 5);
    assert(fifo_is_empty(&fifo) == false);
    
    // 读取 5 个元素
    for (int i = 0; i < 5; i++) {
        int value;
        assert(fifo_pop(&fifo, &value) == 0);
        assert(value == i);
    }
    assert(fifo_is_empty(&fifo) == true);
    
    printf("PASSED\n");
}

void test_overflow(void)
{
    printf("Test: overflow handling... ");
    
    fifo_init(&fifo, fifo_buffer, FIFO_ITEM_SIZE, FIFO_ITEM_COUNT);
    
    // 写满 FIFO
    for (int i = 0; i < FIFO_ITEM_COUNT; i++) {
        assert(fifo_push(&fifo, &i) == 0);
    }
    assert(fifo_is_full(&fifo) == true);
    assert(fifo_space(&fifo) == 0);
    
    // 尝试再写入，应该失败
    int extra = 999;
    assert(fifo_push(&fifo, &extra) == -1);
    assert(fifo.overflow == true);
    
    printf("PASSED\n");
}

void test_wrap_around(void)
{
    printf("Test: wrap around... ");
    
    fifo_init(&fifo, fifo_buffer, FIFO_ITEM_SIZE, FIFO_ITEM_COUNT);
    
    // 写入 5 个元素
    for (int i = 0; i < 5; i++) {
        fifo_push(&fifo, &i);
    }
    
    // 读取 3 个元素
    for (int i = 0; i < 3; i++) {
        int value;
        fifo_pop(&fifo, &value);
    }
    
    // 再写入 5 个元素（会回绕）
    for (int i = 10; i < 15; i++) {
        assert(fifo_push(&fifo, &i) == 0);
    }
    
    // 读取剩余的 7 个元素
    int expected[] = {3, 4, 10, 11, 12, 13, 14};
    for (int i = 0; i < 7; i++) {
        int value;
        assert(fifo_pop(&fifo, &value) == 0);
        assert(value == expected[i]);
    }
    
    printf("PASSED\n");
}

void test_peek(void)
{
    printf("Test: peek... ");
    
    fifo_init(&fifo, fifo_buffer, FIFO_ITEM_SIZE, FIFO_ITEM_COUNT);
    
    // 空 FIFO peek 失败
    int value;
    assert(fifo_peek(&fifo, &value) == -1);
    
    // 写入一个元素
    int test_value = 42;
    fifo_push(&fifo, &test_value);
    
    // peek 不取出
    assert(fifo_peek(&fifo, &value) == 0);
    assert(value == 42);
    assert(fifo_count(&fifo) == 1);  // 数量不变
    
    // 再次 peek 还是同一个值
    assert(fifo_peek(&fifo, &value) == 0);
    assert(value == 42);
    
    printf("PASSED\n");
}

void test_batch_operations(void)
{
    printf("Test: batch operations... ");
    
    fifo_init(&fifo, fifo_buffer, FIFO_ITEM_SIZE, FIFO_ITEM_COUNT);
    
    // 批量写入
    int write_data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    size_t pushed = fifo_push_many(&fifo, write_data, 8);
    assert(pushed == 8);
    assert(fifo_is_full(&fifo) == true);
    
    // 批量读取 4 个
    int read_data[4];
    size_t popped = fifo_pop_many(&fifo, read_data, 4);
    assert(popped == 4);
    assert(memcmp(read_data, write_data, 4 * sizeof(int)) == 0);
    
    // 再批量写入 4 个
    int write_more[] = {10, 11, 12, 13};
    pushed = fifo_push_many(&fifo, write_more, 4);
    assert(pushed == 4);
    
    // 读取全部
    int final_data[8];
    popped = fifo_pop_many(&fifo, final_data, 8);
    assert(popped == 8);
    
    // 验证数据
    int expected[] = {4, 5, 6, 7, 10, 11, 12, 13};
    assert(memcmp(final_data, expected, 8 * sizeof(int)) == 0);
    
    printf("PASSED\n");
}

void test_clear(void)
{
    printf("Test: clear... ");
    
    fifo_init(&fifo, fifo_buffer, FIFO_ITEM_SIZE, FIFO_ITEM_COUNT);
    
    // 写入一些数据
    for (int i = 0; i < 5; i++) {
        fifo_push(&fifo, &i);
    }
    assert(fifo_is_empty(&fifo) == false);
    
    // 清空
    fifo_clear(&fifo);
    assert(fifo_is_empty(&fifo) == true);
    assert(fifo_count(&fifo) == 0);
    assert(fifo_space(&fifo) == FIFO_ITEM_COUNT);
    
    printf("PASSED\n");
}

int main(void)
{
    printf("=== FIFO Unit Tests ===\n\n");
    
    test_init();
    test_push_pop();
    test_overflow();
    test_wrap_around();
    test_peek();
    test_batch_operations();
    test_clear();
    
    printf("\n=== All tests PASSED ===\n");
    
    return 0;
}
