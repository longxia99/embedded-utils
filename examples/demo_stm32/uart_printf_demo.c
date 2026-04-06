/**
 * @file uart_printf_demo.c
 * @brief STM32 平台 UART 重定向 printf 示例
 * 
 * 硬件连接：
 * - USART1_TX -> PA9
 * - USART1_RX -> PA10
 */

#include "stm32f1xx_hal.h"
#include <stdio.h>
#include "../../include/drivers/uart_dev.h"

// UART 设备句柄
static uart_dev_t uart_dev;

// UART 句柄（由 HAL 提供）
extern UART_HandleTypeDef huart1;

// 接收/发送缓冲区
#define RX_BUFFER_SIZE  256
#define TX_BUFFER_SIZE  256
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t tx_buffer[TX_BUFFER_SIZE];

/*============================================================================
 * 平台层实现（STM32 HAL）
 *============================================================================*/

/**
 * @brief STM32 UART 初始化
 */
static int stm32_uart_init(void *handle, uint32_t baudrate, uart_data_bits_t data_bits,
                           uart_stop_bits_t stop_bits, uart_parity_t parity)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)handle;
    
    // 配置波特率
    huart->Init.BaudRate = baudrate;
    
    // 配置数据位
    switch (data_bits) {
        case UART_DATA_7:
            huart->Init.WordLength = UART_WORDLENGTH_7B;
            break;
        case UART_DATA_9:
            huart->Init.WordLength = UART_WORDLENGTH_9B;
            break;
        case UART_DATA_8:
        default:
            huart->Init.WordLength = UART_WORDLENGTH_8B;
            break;
    }
    
    // 配置停止位
    switch (stop_bits) {
        case UART_STOP_2:
            huart->Init.StopBits = UART_STOPBITS_2;
            break;
        case UART_STOP_1_5:
            huart->Init.StopBits = UART_STOPBITS_1_5;
            break;
        case UART_STOP_1:
        default:
            huart->Init.StopBits = UART_STOPBITS_1;
            break;
    }
    
    // 配置校验位
    switch (parity) {
        case UART_PARITY_EVEN:
            huart->Init.Parity = UART_PARITY_EVEN;
            break;
        case UART_PARITY_ODD:
            huart->Init.Parity = UART_PARITY_ODD;
            break;
        case UART_PARITY_NONE:
        default:
            huart->Init.Parity = UART_PARITY_NONE;
            break;
    }
    
    if (HAL_UART_Init(huart) == HAL_OK) {
        return UART_OK;
    }
    return UART_ERR_PARAM;
}

/**
 * @brief STM32 UART 发送一个字节
 */
static int stm32_uart_send_byte(void *handle, uint8_t data, uint32_t timeout)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)handle;
    
    if (HAL_UART_Transmit(huart, &data, 1, timeout) == HAL_OK) {
        return UART_OK;
    }
    return UART_ERR_TIMEOUT;
}

/**
 * @brief STM32 UART 接收一个字节
 */
static int stm32_uart_recv_byte(void *handle, uint8_t *data, uint32_t timeout)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)handle;
    
    if (HAL_UART_Receive(huart, data, 1, timeout) == HAL_OK) {
        return UART_OK;
    }
    return UART_ERR_TIMEOUT;
}

/**
 * @brief STM32 UART 中断处理
 */
void UART1_IRQHandler(void)
{
    uint8_t data;
    
    // 接收中断
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) {
        data = huart1.Instance->DR;
        uart_dev_isr_rx(&uart_dev, data);
    }
    
    // 发送中断
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) != RESET) {
        uint8_t tx_data;
        size_t count = uart_dev_isr_tx(&uart_dev, &tx_data, 1);
        
        if (count > 0) {
            huart1.Instance->DR = tx_data;
        } else {
            // 发送完成，关闭 TXE 中断
            __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
            uart_dev.tx_busy = false;
        }
    }
    
    // 错误中断
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE) != RESET ||
        __HAL_UART_GET_FLAG(&huart1, UART_FLAG_NE) != RESET ||
        __HAL_UART_GET_FLAG(&huart1, UART_FLAG_FE) != RESET) {
        uart_dev_isr_error(&uart_dev, UART_ERR_FRAME);
        __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_ORE | UART_FLAG_NE | UART_FLAG_FE);
    }
}

/**
 * @brief UART 平台操作接口
 */
static const uart_platform_ops_t stm32_uart_ops = {
    .init = stm32_uart_init,
    .deinit = NULL,
    .send_byte = stm32_uart_send_byte,
    .recv_byte = stm32_uart_recv_byte,
    .dma_recv_start = NULL,
    .dma_send_start = NULL,
    .dma_get_count = NULL,
    .irq_handler = NULL
};

/*============================================================================
 * printf 重定向
 *============================================================================*/

/**
 * @brief 重定向 fputc 到 UART
 */
int fputc(int ch, FILE *f)
{
    (void)f;
    uart_dev_send_byte(&uart_dev, (uint8_t)ch, 100);
    return ch;
}

/**
 * @brief 重定向 fgetc 从 UART
 */
int fgetc(int f)
{
    (void)f;
    uint8_t ch;
    
    while (uart_dev_recv_byte(&uart_dev, &ch, 1000) != UART_OK) {
        // 等待字符
    }
    
    return ch;
}

/*============================================================================
 * 应用层示例
 *============================================================================*/

/**
 * @brief 初始化 UART
 */
int uart_init(void)
{
    // 注册平台接口
    uart_dev_register_platform(&stm32_uart_ops);
    
    // 初始化设备（115200）
    return uart_dev_init(&uart_dev, &huart1, 
                         rx_buffer, sizeof(rx_buffer),
                         tx_buffer, sizeof(tx_buffer),
                         115200);
}

/**
 * @brief 命令行交互示例
 */
void uart_cli_task(void)
{
    static char cmd_buffer[64];
    static size_t cmd_len = 0;
    uint8_t ch;
    
    // 非阻塞读取
    while (uart_dev_read(&uart_dev, &ch, 1) > 0) {
        if (ch == '\r' || ch == '\n') {
            if (cmd_len > 0) {
                cmd_buffer[cmd_len] = '\0';
                
                // 处理命令
                printf("\r\nReceived command: %s\r\n", cmd_buffer);
                
                // 简单命令解析
                if (strcmp(cmd_buffer, "help") == 0) {
                    printf("Available commands:\r\n");
                    printf("  help    - Show this help\r\n");
                    printf("  info    - Show system info\r\n");
                    printf("  clear   - Clear screen\r\n");
                } else if (strcmp(cmd_buffer, "info") == 0) {
                    printf("System Info:\r\n");
                    printf("  CPU: STM32F103\r\n");
                    printf("  SysTick: %lu\r\n", HAL_GetTick());
                } else if (strcmp(cmd_buffer, "clear") == 0) {
                    printf("\033[2J\033[H");  // ANSI 清屏
                } else {
                    printf("Unknown command: %s\r\n", cmd_buffer);
                    printf("Type 'help' for available commands\r\n");
                }
                
                cmd_len = 0;
                printf("> ");
            }
        } else if (ch == '\b' || ch == 127) {
            // 退格
            if (cmd_len > 0) {
                cmd_len--;
                printf("\b \b");
            }
        } else if (cmd_len < sizeof(cmd_buffer) - 1) {
            // 正常字符
            cmd_buffer[cmd_len++] = ch;
            putchar(ch);
        }
    }
}

/*============================================================================
 * 主函数示例
 *============================================================================*/

int main(void)
{
    // HAL 初始化
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    
    // 初始化 UART
    if (uart_init() != UART_OK) {
        // 错误处理
        while (1);
    }
    
    // 开启 UART 中断
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
    
    // 欢迎信息
    printf("\r\n=== UART Demo ===\r\n");
    printf("STM32 UART with ring buffer\r\n");
    printf("Type 'help' for commands\r\n\r\n");
    printf("> ");
    
    while (1) {
        // 处理命令行
        uart_cli_task();
        
        // 其他任务...
        HAL_Delay(10);
    }
}
