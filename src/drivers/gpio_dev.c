/**
 * @file gpio_dev.c
 * @brief GPIO 通用抽象层实现
 */

#include "gpio_dev.h"
#include <string.h>

/*============================================================================
 * 全局变量
 *============================================================================*/

gpio_platform_ops_t g_gpio_ops = {0};

/*============================================================================
 * 设备管理
 *============================================================================*/

int gpio_dev_init(gpio_dev_t *dev, void *port, uint16_t pin, 
                  gpio_mode_t mode, gpio_speed_t speed)
{
    if (dev == NULL || port == NULL) {
        return GPIO_ERR_PARAM;
    }
    
    dev->port = port;
    dev->pin = pin & 0x0F;  // 确保 0-15
    dev->mode = mode;
    dev->speed = speed;
    dev->inverted = false;
    dev->user_data = NULL;
    
    // 根据模式设置方向
    if (mode == GPIO_MODE_INPUT || mode == GPIO_MODE_INPUT_PULLUP || 
        mode == GPIO_MODE_INPUT_PULLDOWN || mode == GPIO_MODE_ANALOG) {
        dev->dir = GPIO_DIR_INPUT;
    } else {
        dev->dir = GPIO_DIR_OUTPUT;
    }
    
    // 调用平台初始化
    if (g_gpio_ops.init != NULL) {
        return g_gpio_ops.init(dev);
    }
    
    return GPIO_OK;
}

int gpio_dev_set_output(gpio_dev_t *dev, gpio_level_t initial_level)
{
    if (dev == NULL) {
        return GPIO_ERR_PARAM;
    }
    
    dev->dir = GPIO_DIR_OUTPUT;
    dev->mode = GPIO_MODE_OUTPUT_PP;
    
    if (g_gpio_ops.set_dir != NULL) {
        g_gpio_ops.set_dir(dev, GPIO_DIR_OUTPUT);
    }
    
    // 设置初始电平
    if (g_gpio_ops.write != NULL) {
        g_gpio_ops.write(dev, initial_level);
    }
    
    return GPIO_OK;
}

int gpio_dev_set_input(gpio_dev_t *dev, gpio_mode_t pull)
{
    if (dev == NULL) {
        return GPIO_ERR_PARAM;
    }
    
    dev->dir = GPIO_DIR_INPUT;
    dev->mode = pull;
    
    if (g_gpio_ops.set_dir != NULL) {
        g_gpio_ops.set_dir(dev, GPIO_DIR_INPUT);
    }
    
    return GPIO_OK;
}

int gpio_dev_set_output_od(gpio_dev_t *dev, gpio_mode_t pull)
{
    if (dev == NULL) {
        return GPIO_ERR_PARAM;
    }
    
    dev->dir = GPIO_DIR_OUTPUT;
    dev->mode = GPIO_MODE_OUTPUT_OD;
    
    if (g_gpio_ops.set_dir != NULL) {
        g_gpio_ops.set_dir(dev, GPIO_DIR_OUTPUT);
    }
    
    return GPIO_OK;
}

/*============================================================================
 * 基本操作
 *============================================================================*/

gpio_level_t gpio_dev_read(gpio_dev_t *dev)
{
    if (dev == NULL || g_gpio_ops.read == NULL) {
        return GPIO_LEVEL_LOW;
    }
    
    gpio_level_t level = g_gpio_ops.read(dev);
    
    // 反相处理
    return dev->inverted ? (level == GPIO_LEVEL_LOW ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW) : level;
}

void gpio_dev_write(gpio_dev_t *dev, gpio_level_t level)
{
    if (dev == NULL || g_gpio_ops.write == NULL) {
        return;
    }
    
    // 反相处理
    gpio_level_t actual_level = dev->inverted ? 
                                (level == GPIO_LEVEL_LOW ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW) : 
                                level;
    
    g_gpio_ops.write(dev, actual_level);
}

void gpio_dev_toggle(gpio_dev_t *dev)
{
    if (dev == NULL || g_gpio_ops.toggle == NULL) {
        return;
    }
    
    g_gpio_ops.toggle(dev);
}

gpio_level_t gpio_dev_read_raw(gpio_dev_t *dev)
{
    if (dev == NULL || g_gpio_ops.read == NULL) {
        return GPIO_LEVEL_LOW;
    }
    
    return g_gpio_ops.read(dev);  // 不反相
}

/*============================================================================
 * 便捷操作
 *============================================================================*/

void gpio_dev_set_bit(gpio_dev_t *dev)
{
    gpio_dev_set(dev);
}

void gpio_dev_clear_bit(gpio_dev_t *dev)
{
    gpio_dev_clear(dev);
}

void gpio_dev_toggle_bit(gpio_dev_t *dev)
{
    gpio_dev_toggle(dev);
}

void gpio_dev_write_bit(gpio_dev_t *dev, gpio_level_t level)
{
    gpio_dev_write(dev, level);
}

/*============================================================================
 * 中断管理
 *============================================================================*/

int gpio_dev_irq_config(gpio_dev_t *dev, gpio_edge_t edge, 
                        gpio_irq_callback_t callback, void *user_data)
{
    if (dev == NULL || g_gpio_ops.irq_config == NULL) {
        return GPIO_ERR_PARAM;
    }
    
    dev->user_data = user_data;
    
    return g_gpio_ops.irq_config(dev, edge, callback, user_data);
}

void gpio_dev_irq_enable(gpio_dev_t *dev)
{
    if (dev == NULL || g_gpio_ops.irq_enable == NULL) {
        return;
    }
    
    g_gpio_ops.irq_enable(dev);
}

void gpio_dev_irq_disable(gpio_dev_t *dev)
{
    if (dev == NULL || g_gpio_ops.irq_disable == NULL) {
        return;
    }
    
    g_gpio_ops.irq_disable(dev);
}

/*============================================================================
 * 高级功能
 *============================================================================*/

void gpio_dev_set_inverted(gpio_dev_t *dev, bool inverted)
{
    if (dev == NULL) {
        return;
    }
    
    dev->inverted = inverted;
}

int gpio_dev_wait_level(gpio_dev_t *dev, gpio_level_t target_level, uint32_t timeout_ms)
{
    if (dev == NULL || g_gpio_ops.read == NULL) {
        return GPIO_ERR_PARAM;
    }
    
    // 简单轮询（实际使用需要平台提供精确计时）
    uint32_t count = timeout_ms / 10;  // 假设 10ms 一次
    
    while (count > 0) {
        if (gpio_dev_read(dev) == target_level) {
            return GPIO_OK;
        }
        
        // delay_ms(10);
        count--;
    }
    
    return GPIO_ERR_TIMEOUT;
}

void gpio_dev_pulse(gpio_dev_t *dev, uint32_t count, uint32_t delay_us)
{
    if (dev == NULL) {
        return;
    }
    
    for (uint32_t i = 0; i < count; i++) {
        gpio_dev_set(dev);
        
        // delay_us(delay_us);
        for (volatile uint32_t j = 0; j < delay_us; j++);
        
        gpio_dev_clear(dev);
        
        // delay_us(delay_us);
        for (volatile uint32_t j = 0; j < delay_us; j++);
    }
}

uint16_t gpio_dev_read_port(void *port, uint16_t mask)
{
    // 这个函数需要平台具体实现
    // 这里提供一个通用框架
    (void)port;
    (void)mask;
    return 0;
}

void gpio_dev_write_port(void *port, uint16_t mask, uint16_t value)
{
    // 这个函数需要平台具体实现
    (void)port;
    (void)mask;
    (void)value;
}

/*============================================================================
 * 平台接口注册
 *============================================================================*/

void gpio_dev_register_platform(const gpio_platform_ops_t *ops)
{
    if (ops != NULL) {
        memcpy(&g_gpio_ops, ops, sizeof(gpio_platform_ops_t));
    }
}
