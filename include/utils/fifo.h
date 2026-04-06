/**
 * @file fifo.h
 * @brief 通用 FIFO 队列 - 支持多生产者多消费者
 * 
 * @author Your Name
 * @version 1.0.0
 * @date 2026-04-06
 * 
 * @copyright MIT License
 */

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief FIFO 结构体
 * 
 * 基于环形缓冲区实现，支持多生产者多消费者
 * 需要外部同步机制（mutex/关中断）保证线程安全
 */
typedef struct {
    uint8_t *buffer;      /**< 数据缓冲区 */
    size_t size;          /**< 缓冲区总大小 */
    size_t head;          /**< 写指针 */
    size_t tail;          /**< 读指针 */
    size_t count;         /**< 当前数据量 */
    size_t item_size;     /**< 单个元素大小（字节） */
    size_t max_items;     /**< 最大元素数量 */
    bool overflow;        /**< 溢出标志 */
} fifo_t;

/**
 * @brief 初始化 FIFO
 * 
 * @param fifo FIFO 结构体指针
 * @param buffer 数据缓冲区指针
 * @param item_size 单个元素大小（字节）
 * @param item_count 最大元素数量
 * @return int 0-成功，-1-失败
 */
int fifo_init(fifo_t *fifo, uint8_t *buffer, size_t item_size, size_t item_count);

/**
 * @brief 向 FIFO 写入一个元素
 * 
 * @param fifo FIFO 结构体指针
 * @param data 数据指针
 * @return int 0-成功，-1-失败（FIFO 满）
 */
int fifo_push(fifo_t *fifo, const void *data);

/**
 * @brief 从 FIFO 读出一个元素
 * 
 * @param fifo FIFO 结构体指针
 * @param data 数据接收指针
 * @return int 0-成功，-1-失败（FIFO 空）
 */
int fifo_pop(fifo_t *fifo, void *data);

/**
 * @brief 查看队首元素（不取出）
 * 
 * @param fifo FIFO 结构体指针
 * @param data 数据接收指针
 * @return int 0-成功，-1-失败（FIFO 空）
 */
int fifo_peek(fifo_t *fifo, void *data);

/**
 * @brief 查询 FIFO 中元素数量
 * 
 * @param fifo FIFO 结构体指针
 * @return size_t 元素数量
 */
size_t fifo_count(fifo_t *fifo);

/**
 * @brief 查询 FIFO 剩余空间
 * 
 * @param fifo FIFO 结构体指针
 * @return size_t 可写入元素数量
 */
size_t fifo_space(fifo_t *fifo);

/**
 * @brief 检查 FIFO 是否为空
 * 
 * @param fifo FIFO 结构体指针
 * @return true 空
 * @return false 非空
 */
bool fifo_is_empty(fifo_t *fifo);

/**
 * @brief 检查 FIFO 是否已满
 * 
 * @param fifo FIFO 结构体指针
 * @return true 满
 * @return false 未满
 */
bool fifo_is_full(fifo_t *fifo);

/**
 * @brief 清空 FIFO
 * 
 * @param fifo FIFO 结构体指针
 */
void fifo_clear(fifo_t *fifo);

/**
 * @brief 批量写入元素
 * 
 * @param fifo FIFO 结构体指针
 * @param data 数据数组指针
 * @param count 元素数量
 * @return size_t 实际写入的元素数量
 */
size_t fifo_push_many(fifo_t *fifo, const void *data, size_t count);

/**
 * @brief 批量读出元素
 * 
 * @param fifo FIFO 结构体指针
 * @param data 数据数组指针
 * @param count 读取数量
 * @return size_t 实际读出的元素数量
 */
size_t fifo_pop_many(fifo_t *fifo, void *data, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* FIFO_H */
