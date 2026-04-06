/**
 * @file fifo.c
 * @brief 通用 FIFO 队列实现
 */

#include "fifo.h"
#include <string.h>

/**
 * @brief 初始化 FIFO
 */
int fifo_init(fifo_t *fifo, uint8_t *buffer, size_t item_size, size_t item_count)
{
    if (fifo == NULL || buffer == NULL || item_size == 0 || item_count == 0) {
        return -1;
    }
    
    fifo->buffer = buffer;
    fifo->size = item_size * item_count;
    fifo->item_size = item_size;
    fifo->max_items = item_count;
    fifo->head = 0;
    fifo->tail = 0;
    fifo->count = 0;
    fifo->overflow = false;
    
    return 0;
}

/**
 * @brief 向 FIFO 写入一个元素
 */
int fifo_push(fifo_t *fifo, const void *data)
{
    if (fifo == NULL || data == NULL) {
        return -1;
    }
    
    // FIFO 满
    if (fifo->count >= fifo->max_items) {
        fifo->overflow = true;
        return -1;
    }
    
    // 计算写入位置
    uint8_t *write_pos = fifo->buffer + (fifo->head * fifo->item_size);
    
    // 复制数据
    memcpy(write_pos, data, fifo->item_size);
    
    // 更新写指针
    fifo->head = (fifo->head + 1) % fifo->max_items;
    fifo->count++;
    
    return 0;
}

/**
 * @brief 从 FIFO 读出一个元素
 */
int fifo_pop(fifo_t *fifo, void *data)
{
    if (fifo == NULL || data == NULL) {
        return -1;
    }
    
    // FIFO 空
    if (fifo->count == 0) {
        return -1;
    }
    
    // 计算读取位置
    uint8_t *read_pos = fifo->buffer + (fifo->tail * fifo->item_size);
    
    // 复制数据
    memcpy(data, read_pos, fifo->item_size);
    
    // 更新读指针
    fifo->tail = (fifo->tail + 1) % fifo->max_items;
    fifo->count--;
    
    fifo->overflow = false;  // 清除溢出标志
    
    return 0;
}

/**
 * @brief 查看队首元素（不取出）
 */
int fifo_peek(fifo_t *fifo, void *data)
{
    if (fifo == NULL || data == NULL) {
        return -1;
    }
    
    // FIFO 空
    if (fifo->count == 0) {
        return -1;
    }
    
    // 复制数据（不更新指针）
    uint8_t *read_pos = fifo->buffer + (fifo->tail * fifo->item_size);
    memcpy(data, read_pos, fifo->item_size);
    
    return 0;
}

/**
 * @brief 查询 FIFO 中元素数量
 */
size_t fifo_count(fifo_t *fifo)
{
    if (fifo == NULL) {
        return 0;
    }
    
    return fifo->count;
}

/**
 * @brief 查询 FIFO 剩余空间
 */
size_t fifo_space(fifo_t *fifo)
{
    if (fifo == NULL) {
        return 0;
    }
    
    return fifo->max_items - fifo->count;
}

/**
 * @brief 检查 FIFO 是否为空
 */
bool fifo_is_empty(fifo_t *fifo)
{
    if (fifo == NULL) {
        return true;
    }
    
    return fifo->count == 0;
}

/**
 * @brief 检查 FIFO 是否已满
 */
bool fifo_is_full(fifo_t *fifo)
{
    if (fifo == NULL) {
        return false;
    }
    
    return fifo->count >= fifo->max_items;
}

/**
 * @brief 清空 FIFO
 */
void fifo_clear(fifo_t *fifo)
{
    if (fifo == NULL) {
        return;
    }
    
    fifo->head = 0;
    fifo->tail = 0;
    fifo->count = 0;
    fifo->overflow = false;
}

/**
 * @brief 批量写入元素
 */
size_t fifo_push_many(fifo_t *fifo, const void *data, size_t count)
{
    if (fifo == NULL || data == NULL) {
        return 0;
    }
    
    size_t pushed = 0;
    const uint8_t *src = (const uint8_t *)data;
    
    while (pushed < count) {
        if (fifo_push(fifo, src) != 0) {
            break;  // FIFO 满
        }
        src += fifo->item_size;
        pushed++;
    }
    
    return pushed;
}

/**
 * @brief 批量读出元素
 */
size_t fifo_pop_many(fifo_t *fifo, void *data, size_t count)
{
    if (fifo == NULL || data == NULL) {
        return 0;
    }
    
    size_t popped = 0;
    uint8_t *dst = (uint8_t *)data;
    
    while (popped < count) {
        if (fifo_pop(fifo, dst) != 0) {
            break;  // FIFO 空
        }
        dst += fifo->item_size;
        popped++;
    }
    
    return popped;
}
