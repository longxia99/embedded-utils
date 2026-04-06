/**
 * @file ring_buffer_demo.c
 * @brief Linux 平台环形缓冲区使用示例
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/utils/ring_buffer.h"

#define BUFFER_SIZE  256

int main(void)
{
    printf("=== Ring Buffer Demo on Linux ===\n\n");
    
    // 分配缓冲区
    uint8_t *buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        printf("Failed to allocate buffer\n");
        return -1;
    }
    
    ring_buffer_t rb;
    
    // 初始化
    if (ring_buffer_init(&rb, buffer, BUFFER_SIZE) != 0) {
        printf("Failed to initialize ring buffer\n");
        free(buffer);
        return -1;
    }
    
    printf("Buffer initialized: %zu bytes\n\n", BUFFER_SIZE);
    
    // 示例 1：批量写入和读取
    printf("Demo 1: Batch write/read\n");
    uint8_t write_data[] = "Hello, Embedded-Utils!";
    size_t write_len = strlen((char*)write_data);
    
    size_t written = ring_buffer_write(&rb, write_data, write_len);
    printf("Written: %zu bytes\n", written);
    printf("Available: %zu bytes\n", ring_buffer_available(&rb));
    
    uint8_t read_data[64];
    size_t read_len = ring_buffer_read(&rb, read_data, write_len);
    read_data[read_len] = '\0';
    printf("Read: %zu bytes - \"%s\"\n\n", read_len, (char*)read_data);
    
    // 示例 2：单字节操作
    printf("Demo 2: Byte operations\n");
    for (int i = 0; i < 10; i++) {
        ring_buffer_write_byte(&rb, 'A' + i);
    }
    printf("Wrote 10 bytes (A-J)\n");
    
    printf("Reading byte by byte: ");
    for (int i = 0; i < 10; i++) {
        uint8_t byte;
        if (ring_buffer_read_byte(&rb, &byte) == 0) {
            printf("%c ", byte);
        }
    }
    printf("\n\n");
    
    // 示例 3：回绕测试
    printf("Demo 3: Wrap around\n");
    ring_buffer_clear(&rb);
    
    // 写入接近缓冲区大小的数据
    uint8_t large_data[200];
    memset(large_data, 'X', sizeof(large_data));
    
    written = ring_buffer_write(&rb, large_data, sizeof(large_data));
    printf("Written: %zu bytes\n", written);
    
    // 读取一半
    uint8_t half_data[100];
    ring_buffer_read(&rb, half_data, 100);
    printf("Read 100 bytes, available: %zu\n", ring_buffer_available(&rb));
    
    // 再写入，触发回绕
    written = ring_buffer_write(&rb, large_data, 100);
    printf("Written again: %zu bytes (wrap around)\n", written);
    printf("Final available: %zu bytes\n", ring_buffer_available(&rb));
    
    // 示例 4：溢出处理
    printf("\nDemo 4: Overflow handling\n");
    ring_buffer_clear(&rb);
    
    // 写满缓冲区
    uint8_t fill_data[255];
    memset(fill_data, 0, sizeof(fill_data));
    ring_buffer_write(&rb, fill_data, sizeof(fill_data));
    
    printf("Buffer full: %s\n", ring_buffer_is_full(&rb) ? "YES" : "NO");
    printf("Space remaining: %zu bytes\n", ring_buffer_space(&rb));
    
    // 尝试写入更多数据
    uint8_t extra = 0xFF;
    size_t extra_written = ring_buffer_write(&rb, &extra, 1);
    printf("Extra write attempt: %zu bytes (overflow: %s)\n", 
           extra_written, rb.overflow ? "YES" : "NO");
    
    // 清理
    free(buffer);
    
    printf("\n=== Demo completed ===\n");
    
    return 0;
}
