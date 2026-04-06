/**
 * @file ring_buffer.c
 * @brief 环形缓冲区实现 - 中断安全
 */

#include "ring_buffer.h"
#include <string.h>

/**
 * @brief 初始化环形缓冲区
 */
int ring_buffer_init(ring_buffer_t *rb, uint8_t *buffer, size_t size)
{
    if (rb == NULL || buffer == NULL || size == 0) {
        return -1;
    }
    
    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->overflow = false;
    
    return 0;
}

/**
 * @brief 向环形缓冲区写入数据
 * 
 * 注意：此函数不是原子操作，如果在中断和主循环中同时调用，
 * 需要外部保证同一时刻只有一个生产者。
 */
size_t ring_buffer_write(ring_buffer_t *rb, const uint8_t *data, size_t len)
{
    if (rb == NULL || data == NULL) {
        return 0;
    }
    
    size_t written = 0;
    size_t next_head;
    
    while (written < len) {
        next_head = (rb->head + 1) % rb->size;
        
        // 缓冲区满
        if (next_head == rb->tail) {
            rb->overflow = true;
            break;
        }
        
        rb->buffer[rb->head] = data[written];
        rb->head = next_head;
        written++;
    }
    
    return written;
}

/**
 * @brief 从环形缓冲区读取数据
 */
size_t ring_buffer_read(ring_buffer_t *rb, uint8_t *data, size_t len)
{
    if (rb == NULL || data == NULL) {
        return 0;
    }
    
    size_t read = 0;
    
    while (read < len && rb->head != rb->tail) {
        data[read] = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % rb->size;
        read++;
    }
    
    rb->overflow = false;  // 读取后清除溢出标志
    
    return read;
}

/**
 * @brief 查询缓冲区中可用数据量
 */
size_t ring_buffer_available(ring_buffer_t *rb)
{
    if (rb == NULL) {
        return 0;
    }
    
    if (rb->head >= rb->tail) {
        return rb->head - rb->tail;
    } else {
        return rb->size - rb->tail + rb->head;
    }
}

/**
 * @brief 查询缓冲区剩余空间
 */
size_t ring_buffer_space(ring_buffer_t *rb)
{
    if (rb == NULL) {
        return 0;
    }
    
    // 预留一个字节区分空和满
    return rb->size - ring_buffer_available(rb) - 1;
}

/**
 * @brief 清空环形缓冲区
 */
void ring_buffer_clear(ring_buffer_t *rb)
{
    if (rb == NULL) {
        return;
    }
    
    rb->head = 0;
    rb->tail = 0;
    rb->overflow = false;
}

/**
 * @brief 检查缓冲区是否为空
 */
bool ring_buffer_is_empty(ring_buffer_t *rb)
{
    if (rb == NULL) {
        return true;
    }
    
    return rb->head == rb->tail;
}

/**
 * @brief 检查缓冲区是否已满
 */
bool ring_buffer_is_full(ring_buffer_t *rb)
{
    if (rb == NULL) {
        return false;
    }
    
    return ((rb->head + 1) % rb->size) == rb->tail;
}

/**
 * @brief 单次写入一个字节（中断安全）
 */
int ring_buffer_write_byte(ring_buffer_t *rb, uint8_t data)
{
    if (rb == NULL) {
        return -1;
    }
    
    size_t next_head = (rb->head + 1) % rb->size;
    
    // 缓冲区满
    if (next_head == rb->tail) {
        rb->overflow = true;
        return -1;
    }
    
    rb->buffer[rb->head] = data;
    rb->head = next_head;
    
    return 0;
}

/**
 * @brief 单次读取一个字节（中断安全）
 */
int ring_buffer_read_byte(ring_buffer_t *rb, uint8_t *data)
{
    if (rb == NULL || data == NULL) {
        return -1;
    }
    
    // 缓冲区空
    if (rb->head == rb->tail) {
        return -1;
    }
    
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    rb->overflow = false;
    
    return 0;
}
