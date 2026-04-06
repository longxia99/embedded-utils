/**
 * @file test_gpio_dev.c
 * @brief GPIO 设备框架单元测试（模拟平台层）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/drivers/gpio_dev.h"

/*============================================================================
 * 模拟 GPIO 平台层
 *============================================================================*/

static gpio_level_t simulated_gpio_state = GPIO_LEVEL_LOW;
static bool simulated_gpio_output = true;
static int simulated_irq_count = 0;

/**
 * @brief 模拟 GPIO 初始化
 */
static int mock_gpio_init(gpio_dev_t *dev)
{
    (void)dev;
    simulated_gpio_state = GPIO_LEVEL_LOW;
    return GPIO_OK;
}

/**
 * @brief 模拟 GPIO 设置方向
 */
static int mock_gpio_set_dir(gpio_dev_t *dev, gpio_dir_t dir)
{
    dev->dir = dir;
    simulated_gpio_output = (dir == GPIO_DIR_OUTPUT);
    return GPIO_OK;
}

/**
 * @brief 模拟 GPIO 读取
 */
static gpio_level_t mock_gpio_read(gpio_dev_t *dev)
{
    (void)dev;
    return simulated_gpio_state;
}

/**
 * @brief 模拟 GPIO 写入
 */
static void mock_gpio_write(gpio_dev_t *dev, gpio_level_t level)
{
    (void)dev;
    if (simulated_gpio_output) {
        simulated_gpio_state = level;
    }
}

/**
 * @brief 模拟 GPIO 切换
 */
static void mock_gpio_toggle(gpio_dev_t *dev)
{
    (void)dev;
    simulated_gpio_state = (simulated_gpio_state == GPIO_LEVEL_LOW) ? 
                           GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
}

/**
 * @brief 模拟 GPIO 中断配置
 */
static int mock_gpio_irq_config(gpio_dev_t *dev, gpio_edge_t edge, 
                                 gpio_irq_callback_t callback, void *user_data)
{
    (void)edge;
    dev->user_data = user_data;
    return GPIO_OK;
}

/**
 * @brief 模拟 GPIO 中断使能
 */
static void mock_gpio_irq_enable(gpio_dev_t *dev)
{
    (void)dev;
}

/**
 * @brief 模拟 GPIO 中断禁用
 */
static void mock_gpio_irq_disable(gpio_dev_t *dev)
{
    (void)dev;
}

// 平台操作接口
static const gpio_platform_ops_t mock_ops = {
    .init = mock_gpio_init,
    .deinit = NULL,
    .set_dir = mock_gpio_set_dir,
    .read = mock_gpio_read,
    .write = mock_gpio_write,
    .toggle = mock_gpio_toggle,
    .irq_config = mock_gpio_irq_config,
    .irq_enable = mock_gpio_irq_enable,
    .irq_disable = mock_gpio_irq_disable
};

/*============================================================================
 * 单元测试
 *============================================================================*/

static gpio_dev_t test_dev;

void test_init(void)
{
    printf("Test: gpio_dev_init... ");
    
    // 注册平台接口
    gpio_dev_register_platform(&mock_ops);
    
    // 初始化 GPIO（PB5, 推挽输出）
    int ret = gpio_dev_init(&test_dev, (void *)0x40010C00, 5, 
                            GPIO_MODE_OUTPUT_PP, GPIO_SPEED_MEDIUM);
    assert(ret == GPIO_OK);
    
    // 验证配置
    assert(test_dev.pin == 5);
    assert(test_dev.mode == GPIO_MODE_OUTPUT_PP);
    assert(test_dev.dir == GPIO_DIR_OUTPUT);
    
    // 错误参数
    ret = gpio_dev_init(NULL, (void *)0x40010C00, 5, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_MEDIUM);
    assert(ret == GPIO_ERR_PARAM);
    
    printf("PASSED\n");
}

void test_set_output_input(void)
{
    printf("Test: set output/input... ");
    
    // 设置为输出，初始低电平
    int ret = gpio_dev_set_output(&test_dev, GPIO_LEVEL_LOW);
    assert(ret == GPIO_OK);
    assert(test_dev.dir == GPIO_DIR_OUTPUT);
    
    // 设置为输入，上拉
    ret = gpio_dev_set_input(&test_dev, GPIO_MODE_INPUT_PULLUP);
    assert(ret == GPIO_OK);
    assert(test_dev.dir == GPIO_DIR_INPUT);
    assert(test_dev.mode == GPIO_MODE_INPUT_PULLUP);
    
    // 设置为开漏输出
    ret = gpio_dev_set_output_od(&test_dev, GPIO_MODE_INPUT_PULLUP);
    assert(ret == GPIO_OK);
    assert(test_dev.dir == GPIO_DIR_OUTPUT);
    assert(test_dev.mode == GPIO_MODE_OUTPUT_OD);
    
    printf("PASSED\n");
}

void test_write_read(void)
{
    printf("Test: write/read... ");
    
    // 设置为输出
    gpio_dev_set_output(&test_dev, GPIO_LEVEL_LOW);
    
    // 写入高电平
    gpio_dev_write(&test_dev, GPIO_LEVEL_HIGH);
    assert(simulated_gpio_state == GPIO_LEVEL_HIGH);
    
    // 写入低电平
    gpio_dev_write(&test_dev, GPIO_LEVEL_LOW);
    assert(simulated_gpio_state == GPIO_LEVEL_LOW);
    
    // 读取（输出模式下读回写值）
    gpio_level_t level = gpio_dev_read(&test_dev);
    assert(level == GPIO_LEVEL_LOW);
    
    printf("PASSED\n");
}

void test_set_clear_toggle(void)
{
    printf("Test: set/clear/toggle... ");
    
    gpio_dev_set_output(&test_dev, GPIO_LEVEL_LOW);
    
    // Set - 置高
    gpio_dev_set(&test_dev);
    assert(simulated_gpio_state == GPIO_LEVEL_HIGH);
    
    // Clear - 清零
    gpio_dev_clear(&test_dev);
    assert(simulated_gpio_state == GPIO_LEVEL_LOW);
    
    // Toggle - 切换
    gpio_dev_toggle(&test_dev);
    assert(simulated_gpio_state == GPIO_LEVEL_HIGH);
    
    gpio_dev_toggle(&test_dev);
    assert(simulated_gpio_state == GPIO_LEVEL_LOW);
    
    printf("PASSED\n");
}

void test_inverted_mode(void)
{
    printf("Test: inverted mode... ");
    
    gpio_dev_set_output(&test_dev, GPIO_LEVEL_LOW);
    
    // 正常模式
    gpio_dev_write(&test_dev, GPIO_LEVEL_HIGH);
    assert(simulated_gpio_state == GPIO_LEVEL_HIGH);
    
    // 反相模式
    gpio_dev_set_inverted(&test_dev, true);
    gpio_dev_write(&test_dev, GPIO_LEVEL_HIGH);
    assert(simulated_gpio_state == GPIO_LEVEL_LOW);  // 反相
    
    gpio_dev_write(&test_dev, GPIO_LEVEL_LOW);
    assert(simulated_gpio_state == GPIO_LEVEL_HIGH);  // 反相
    
    // 读取也反相
    simulated_gpio_state = GPIO_LEVEL_LOW;
    level = gpio_dev_read(&test_dev);
    assert(level == GPIO_LEVEL_HIGH);  // 反相
    
    printf("PASSED\n");
}

void test_irq_config(void)
{
    printf("Test: IRQ config... ");
    
    // 配置中断
    int ret = gpio_dev_irq_config(&test_dev, GPIO_EDGE_FALLING, NULL, (void *)0x1234);
    assert(ret == GPIO_OK);
    assert(test_dev.user_data == (void *)0x1234);
    
    // 使能中断
    gpio_dev_irq_enable(&test_dev);
    
    // 禁用中断
    gpio_dev_irq_disable(&test_dev);
    
    printf("PASSED\n");
}

void test_wait_level(void)
{
    printf("Test: wait level... ");
    
    gpio_dev_set_input(&test_dev, GPIO_MODE_INPUT_PULLUP);
    
    // 模拟电平变化
    simulated_gpio_state = GPIO_LEVEL_LOW;
    
    // 等待高电平（应该超时）
    int ret = gpio_dev_wait_level(&test_dev, GPIO_LEVEL_HIGH, 10);
    assert(ret == GPIO_ERR_TIMEOUT);
    
    // 等待低电平（应该成功）
    ret = gpio_dev_wait_level(&test_dev, GPIO_LEVEL_LOW, 10);
    assert(ret == GPIO_OK);
    
    printf("PASSED\n");
}

void test_pulse(void)
{
    printf("Test: pulse generation... ");
    
    gpio_dev_set_output(&test_dev, GPIO_LEVEL_LOW);
    
    // 产生 3 个脉冲
    gpio_dev_pulse(&test_dev, 3, 10);
    
    // 验证最终状态（应该是低电平）
    assert(simulated_gpio_state == GPIO_LEVEL_LOW);
    
    printf("PASSED\n");
}

void test_bit_operations(void)
{
    printf("Test: bit operations... ");
    
    gpio_dev_set_output(&test_dev, GPIO_LEVEL_LOW);
    
    // 原子置位
    gpio_dev_set_bit(&test_dev);
    assert(simulated_gpio_state == GPIO_LEVEL_HIGH);
    
    // 原子清零
    gpio_dev_clear_bit(&test_dev);
    assert(simulated_gpio_state == GPIO_LEVEL_LOW);
    
    // 原子切换
    gpio_dev_toggle_bit(&test_dev);
    assert(simulated_gpio_state == GPIO_LEVEL_HIGH);
    
    // 原子写入
    gpio_dev_write_bit(&test_dev, GPIO_LEVEL_LOW);
    assert(simulated_gpio_state == GPIO_LEVEL_LOW);
    
    printf("PASSED\n");
}

int main(void)
{
    printf("=== GPIO Device Framework Unit Tests ===\n\n");
    
    // 重置模拟状态
    simulated_gpio_state = GPIO_LEVEL_LOW;
    simulated_gpio_output = true;
    simulated_irq_count = 0;
    
    test_init();
    test_set_output_input();
    test_write_read();
    test_set_clear_toggle();
    test_inverted_mode();
    test_irq_config();
    test_wait_level();
    test_pulse();
    test_bit_operations();
    
    printf("\n=== All tests PASSED ===\n");
    
    return 0;
}
