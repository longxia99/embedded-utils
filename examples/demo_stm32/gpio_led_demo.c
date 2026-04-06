/**
 * @file gpio_led_demo.c
 * @brief STM32 平台 GPIO LED 控制示例
 * 
 * 硬件连接：
 * - LED1 -> PB5 (推挽输出，高电平点亮)
 * - LED2 -> PB6 (推挽输出，高电平点亮)
 * - KEY1 -> PA0 (上拉输入，按下接地)
 * - KEY2 -> PA1 (上拉输入，按下接地)
 */

#include "stm32f1xx_hal.h"
#include "../../include/drivers/gpio_dev.h"

// GPIO 设备句柄
static gpio_dev_t led1;
static gpio_dev_t led2;
static gpio_dev_t key1;
static gpio_dev_t key2;

// 状态标志
static volatile bool key1_pressed = false;
static volatile bool key2_pressed = false;

/*============================================================================
 * 平台层实现（STM32 HAL）
 *============================================================================*/

/**
 * @brief STM32 GPIO 初始化
 */
static int stm32_gpio_init(gpio_dev_t *dev)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_TypeDef *port = (GPIO_TypeDef *)dev->port;
    
    // 使能端口时钟
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    
    // 配置引脚
    GPIO_InitStruct.Pin = (1 << dev->pin);
    
    // 模式转换
    switch (dev->mode) {
        case GPIO_MODE_INPUT:
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            break;
        case GPIO_MODE_INPUT_PULLUP:
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            break;
        case GPIO_MODE_INPUT_PULLDOWN:
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
            break;
        case GPIO_MODE_OUTPUT_PP:
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            break;
        case GPIO_MODE_OUTPUT_OD:
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            break;
        case GPIO_MODE_ANALOG:
            GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            break;
        default:
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            break;
    }
    
    // 速度转换
    switch (dev->speed) {
        case GPIO_SPEED_LOW:
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            break;
        case GPIO_SPEED_MEDIUM:
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
            break;
        case GPIO_SPEED_HIGH:
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            break;
        case GPIO_SPEED_VERY_HIGH:
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            break;
    }
    
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    
    return GPIO_OK;
}

/**
 * @brief STM32 GPIO 设置方向
 */
static int stm32_gpio_set_dir(gpio_dev_t *dev, gpio_dir_t dir)
{
    dev->dir = dir;
    // 重新初始化
    return stm32_gpio_init(dev);
}

/**
 * @brief STM32 GPIO 读取
 */
static gpio_level_t stm32_gpio_read(gpio_dev_t *dev)
{
    GPIO_PinState state = HAL_GPIO_ReadPin((GPIO_TypeDef *)dev->port, dev->pin);
    return (state == GPIO_PIN_SET) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
}

/**
 * @brief STM32 GPIO 写入
 */
static void stm32_gpio_write(gpio_dev_t *dev, gpio_level_t level)
{
    GPIO_PinState state = (level == GPIO_LEVEL_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin((GPIO_TypeDef *)dev->port, dev->pin, state);
}

/**
 * @brief STM32 GPIO 切换
 */
static void stm32_gpio_toggle(gpio_dev_t *dev)
{
    HAL_GPIO_TogglePin((GPIO_TypeDef *)dev->port, dev->pin);
}

/**
 * @brief STM32 GPIO 中断配置
 */
static int stm32_gpio_irq_config(gpio_dev_t *dev, gpio_edge_t edge, 
                                  gpio_irq_callback_t callback, void *user_data)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_TypeDef *port = (GPIO_TypeDef *)dev->port;
    IRQn_Type irqn;
    
    // 确定中断线
    if (dev->pin < 5) {
        irqn = EXTI0_IRQn + dev->pin;
    } else if (dev->pin == 5) {
        irqn = EXTI9_5_IRQn;
    } else {
        irqn = EXTI15_10_IRQn;
    }
    
    // 边沿转换
    uint32_t trigger;
    switch (edge) {
        case GPIO_EDGE_RISING:
            trigger = GPIO_RISING;
            break;
        case GPIO_EDGE_FALLING:
            trigger = GPIO_FALLING;
            break;
        case GPIO_EDGE_BOTH:
            trigger = GPIO_RISING | GPIO_FALLING;
            break;
        default:
            return GPIO_ERR_PARAM;
    }
    
    // 配置 EXTI
    GPIO_InitStruct.Pin = (1 << dev->pin);
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  // 实际由 trigger 决定
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    
    // 设置中断优先级并使能
    HAL_NVIC_SetPriority(irqn, 1, 0);
    HAL_NVIC_EnableIRQ(irqn);
    
    dev->user_data = user_data;
    
    return GPIO_OK;
}

/**
 * @brief STM32 GPIO 中断使能
 */
static void stm32_gpio_irq_enable(gpio_dev_t *dev)
{
    // 由 HAL 管理，这里简化
}

/**
 * @brief STM32 GPIO 中断禁用
 */
static void stm32_gpio_irq_disable(gpio_dev_t *dev)
{
    // 由 HAL 管理，这里简化
}

/**
 * @brief GPIO 平台操作接口
 */
static const gpio_platform_ops_t stm32_gpio_ops = {
    .init = stm32_gpio_init,
    .deinit = NULL,
    .set_dir = stm32_gpio_set_dir,
    .read = stm32_gpio_read,
    .write = stm32_gpio_write,
    .toggle = stm32_gpio_toggle,
    .irq_config = stm32_gpio_irq_config,
    .irq_enable = stm32_gpio_irq_enable,
    .irq_disable = stm32_gpio_irq_disable
};

/*============================================================================
 * 中断服务函数
 *============================================================================*/

void EXTI0_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
        
        // 按键按下中断（下降沿）
        if (key1.user_data != NULL) {
            gpio_irq_callback_t callback = (gpio_irq_callback_t)key1.user_data;
            callback(&key1, NULL);
        }
    }
}

void EXTI1_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_1) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
        
        if (key2.user_data != NULL) {
            gpio_irq_callback_t callback = (gpio_irq_callback_t)key2.user_data;
            callback(&key2, NULL);
        }
    }
}

/*============================================================================
 * 应用层
 *============================================================================*/

/**
 * @brief 按键中断回调
 */
void key_irq_callback(gpio_dev_t *dev, void *user_data)
{
    (void)user_data;
    
    if (dev == &key1) {
        key1_pressed = true;
        gpio_dev_toggle(&led1);  // 切换 LED1
    } else if (dev == &key2) {
        key2_pressed = true;
        gpio_dev_toggle(&led2);  // 切换 LED2
    }
}

/**
 * @brief 初始化 GPIO
 */
int gpio_init(void)
{
    // 注册平台接口
    gpio_dev_register_platform(&stm32_gpio_ops);
    
    // 初始化 LED1 (PB5, 推挽输出)
    gpio_dev_init(&led1, GPIOB, 5, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_MEDIUM);
    gpio_dev_set_output(&led1, GPIO_LEVEL_LOW);  // 初始熄灭
    
    // 初始化 LED2 (PB6, 推挽输出)
    gpio_dev_init(&led2, GPIOB, 6, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_MEDIUM);
    gpio_dev_set_output(&led2, GPIO_LEVEL_LOW);
    
    // 初始化 KEY1 (PA0, 上拉输入)
    gpio_dev_init(&key1, GPIOA, 0, GPIO_MODE_INPUT_PULLUP, GPIO_SPEED_LOW);
    
    // 初始化 KEY2 (PA1, 上拉输入)
    gpio_dev_init(&key2, GPIOA, 1, GPIO_MODE_INPUT_PULLUP, GPIO_SPEED_LOW);
    
    // 配置按键中断（下降沿）
    gpio_dev_irq_config(&key1, GPIO_EDGE_FALLING, key_irq_callback, (void *)1);
    gpio_dev_irq_config(&key2, GPIO_EDGE_FALLING, key_irq_callback, (void *)2);
    
    return 0;
}

/**
 * @brief LED 闪烁测试
 */
void led_blink(gpio_dev_t *led, uint32_t count, uint32_t delay_ms)
{
    for (uint32_t i = 0; i < count; i++) {
        gpio_dev_toggle(led);
        
        // 简单延时
        for (volatile uint32_t j = 0; j < delay_ms * 1000; j++);
        
        gpio_dev_toggle(led);
        
        for (volatile uint32_t j = 0; j < delay_ms * 1000; j++);
    }
}

/**
 * @brief 按键扫描（轮询方式）
 */
void key_scan(void)
{
    static bool key1_last = true;
    static bool key2_last = true;
    
    bool key1_curr = gpio_dev_read(&key1);
    bool key2_curr = gpio_dev_read(&key2);
    
    // 检测按下（高 -> 低）
    if (key1_last && !key1_curr) {
        gpio_dev_toggle(&led1);
        HAL_Delay(50);  // 消抖
    }
    
    if (key2_last && !key2_curr) {
        gpio_dev_toggle(&led2);
        HAL_Delay(50);
    }
    
    key1_last = key1_curr;
    key2_last = key2_curr;
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
    
    // 初始化 GPIO
    gpio_init();
    
    // 上电自检：LED 闪烁 3 次
    led_blink(&led1, 3, 200);
    led_blink(&led2, 3, 200);
    
    while (1) {
        // 方式 1：中断方式（推荐）
        // 按键按下时自动触发中断，切换 LED
        
        // 方式 2：轮询方式
        key_scan();
        
        // 其他任务...
        HAL_Delay(10);
    }
}
