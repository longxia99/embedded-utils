/**
 * @file ring_buffer.h
 * @brief 环形缓冲区 - 中断安全，支持多生产者多消费者
 * 
 * @author Your Name
 * @version 1.0.0
 * @date 2026-04-06
 * 
 * @copyright MIT License
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 环形缓冲区结构体
 */
typedef struct {
    uint8_t *buffer;      /**< 缓冲区数据指针 */
    size_t size;          /**< 缓冲区总大小 */
    volatile size_t head; /**< 写指针（生产者） */
    volatile size_t tail; /**< 读指针（消费者） */
    bool overflow;        /**< 溢出标志 */
} ring_buffer_t;

/**
 * @brief 初始化环形缓冲区
 * 
 * @param rb 环形缓冲区结构体指针
 * @param buffer 数据缓冲区指针
 * @param size 缓冲区大小（字节）
 * @return int 0-成功，-1-失败
 */
int ring_buffer_init(ring_buffer_t *rb, uint8_t *buffer, size_t size);

/**
 * @brief 向环形缓冲区写入数据
 * 
 * @param rb 环形缓冲区结构体指针
 * @param data 数据指针
 * @param len 数据长度
 * @return size_t 实际写入的字节数
 */
size_t ring_buffer_write(ring_buffer_t *rb, const uint8_t *data, size_t len);

/**
 * @brief 从环形缓冲区读取数据
 * 
 * @param rb 环形缓冲区结构体指针
 * @param data 数据接收指针
 * @param len 读取长度
 * @return size_t 实际读取的字节数
 */
size_t ring_buffer_read(ring_buffer_t *rb, uint8_t *data, size_t len);

/**
 * @brief 查询缓冲区中可用数据量
 * 
 * @param rb 环形缓冲区结构体指针
 * @return size_t 可读字节数
 */
size_t ring_buffer_available(ring_buffer_t *rb);

/**
 * @brief 查询缓冲区剩余空间
 * 
 * @param rb 环形缓冲区结构体指针
 * @return size_t 可写字节数
 */
size_t ring_buffer_space(ring_buffer_t *rb);

/**
 * @brief 清空环形缓冲区
 * 
 * @param rb 环形缓冲区结构体指针
 */
void ring_buffer_clear(ring_buffer_t *rb);

/**
 * @brief 检查缓冲区是否为空
 * 
 * @param rb 环形缓冲区结构体指针
 * @return true 空
 * @return false 非空
 */
bool ring_buffer_is_empty(ring_buffer_t *rb);

/**
 * @brief 检查缓冲区是否已满
 * 
 * @param rb 环形缓冲区结构体指针
 * @return true 满
 * @return false 未满
 */
bool ring_buffer_is_full(ring_buffer_t *rb);

/**
 * @brief 单次写入一个字节（中断安全）
 * 
 * @param rb 环形缓冲区结构体指针
 * @param data 数据字节
 * @return int 0-成功，-1-失败（缓冲区满）
 */
int ring_buffer_write_byte(ring_buffer_t *rb, uint8_t data);

/**
 * @brief 单次读取一个字节（中断安全）
 * 
 * @param rb 环形缓冲区结构体指针
 * @param data 数据字节指针
 * @return int 0-成功，-1-失败（缓冲区空）
 */
int ring_buffer_read_byte(ring_buffer_t *rb, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* RING_BUFFER_H */
