/**
 * @file gpio_dev.h
 * @brief GPIO 通用抽象层
 * 
 * @author Your Name
 * @version 1.0.0
 * @date 2026-04-06
 * 
 * @copyright MIT License
 */

#ifndef GPIO_DEV_H
#define GPIO_DEV_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * 错误码定义
 *============================================================================*/

#define GPIO_OK             0       /**< 成功 */
#define GPIO_ERR_PARAM      -1      /**< 参数错误 */
#define GPIO_ERR_NOT_READY  -2      /**< 未就绪 */
#define GPIO_ERR_TIMEOUT    -3      /**< 超时 */

/*============================================================================
 * GPIO 模式定义
 *============================================================================*/

/**
 * @brief GPIO 方向
 */
typedef enum {
    GPIO_DIR_INPUT = 0,     /**< 输入模式 */
    GPIO_DIR_OUTPUT = 1     /**< 输出模式 */
} gpio_dir_t;

/**
 * @brief GPIO 驱动模式
 */
typedef enum {
    GPIO_MODE_INPUT         = 0,    /**< 浮空输入 */
    GPIO_MODE_INPUT_PULLUP  = 1,    /**< 上拉输入 */
    GPIO_MODE_INPUT_PULLDOWN = 2,   /**< 下拉输入 */
    GPIO_MODE_OUTPUT_PP     = 3,    /**< 推挽输出 */
    GPIO_MODE_OUTPUT_OD     = 4,    /**< 开漏输出 */
    GPIO_MODE_AF_PP         = 5,    /**< 复用推挽 */
    GPIO_MODE_AF_OD         = 6,    /**< 复用开漏 */
    GPIO_MODE_ANALOG        = 7     /**< 模拟模式 */
} gpio_mode_t;

/**
 * @brief GPIO 速度
 */
typedef enum {
    GPIO_SPEED_LOW      = 0,    /**< 低速 */
    GPIO_SPEED_MEDIUM   = 1,    /**< 中速 */
    GPIO_SPEED_HIGH     = 2,    /**< 高速 */
    GPIO_SPEED_VERY_HIGH = 3    /**< 超高速 */
} gpio_speed_t;

/**
 * @brief GPIO 电平
 */
typedef enum {
    GPIO_LEVEL_LOW  = 0,    /**< 低电平 */
    GPIO_LEVEL_HIGH = 1     /**< 高电平 */
} gpio_level_t;

/**
 * @brief GPIO 边沿
 */
typedef enum {
    GPIO_EDGE_NONE      = 0,    /**< 无中断 */
    GPIO_EDGE_RISING    = 1,    /**< 上升沿 */
    GPIO_EDGE_FALLING   = 2,    /**< 下降沿 */
    GPIO_EDGE_BOTH      = 3     /**< 双边沿 */
} gpio_edge_t;

/*============================================================================
 * GPIO 引脚定义
 *============================================================================*/

/**
 * @brief GPIO 引脚句柄
 */
typedef struct {
    void *port;             /**< 端口（由平台定义） */
    uint16_t pin;           /**< 引脚号（0-15） */
    gpio_dir_t dir;         /**< 方向 */
    gpio_mode_t mode;       /**< 模式 */
    gpio_speed_t speed;     /**< 速度 */
    bool inverted;          /**< 是否反相 */
    void *user_data;        /**< 用户数据 */
} gpio_dev_t;

/**
 * @brief GPIO 中断回调函数
 */
typedef void (*gpio_irq_callback_t)(gpio_dev_t *dev, void *user_data);

/**
 * @brief GPIO 平台操作接口（需要移植）
 */
typedef struct {
    /**
     * @brief GPIO 初始化
     * @param dev GPIO 设备句柄
     * @return GPIO_OK 成功，其他失败
     */
    int (*init)(gpio_dev_t *dev);
    
    /**
     * @brief GPIO 去初始化
     * @param dev GPIO 设备句柄
     */
    void (*deinit)(gpio_dev_t *dev);
    
    /**
     * @brief 设置 GPIO 方向
     * @param dev GPIO 设备句柄
     * @param dir 方向
     * @return GPIO_OK 成功
     */
    int (*set_dir)(gpio_dev_t *dev, gpio_dir_t dir);
    
    /**
     * @brief 读取 GPIO 电平
     * @param dev GPIO 设备句柄
     * @return GPIO_LEVEL_LOW/HIGH
     */
    gpio_level_t (*read)(gpio_dev_t *dev);
    
    /**
     * @brief 写入 GPIO 电平
     * @param dev GPIO 设备句柄
     * @param level 电平
     */
    void (*write)(gpio_dev_t *dev, gpio_level_t level);
    
    /**
     * @brief 切换 GPIO 电平
     * @param dev GPIO 设备句柄
     */
    void (*toggle)(gpio_dev_t *dev);
    
    /**
     * @brief 配置 GPIO 中断
     * @param dev GPIO 设备句柄
     * @param edge 触发边沿
     * @param callback 回调函数
     * @param user_data 用户数据
     * @return GPIO_OK 成功
     */
    int (*irq_config)(gpio_dev_t *dev, gpio_edge_t edge, 
                      gpio_irq_callback_t callback, void *user_data);
    
    /**
     * @brief 使能 GPIO 中断
     * @param dev GPIO 设备句柄
     */
    void (*irq_enable)(gpio_dev_t *dev);
    
    /**
     * @brief 禁用 GPIO 中断
     * @param dev GPIO 设备句柄
     */
    void (*irq_disable)(gpio_dev_t *dev);
} gpio_platform_ops_t;

/*============================================================================
 * 设备管理
 *============================================================================*/

/**
 * @brief 初始化 GPIO 引脚
 * 
 * @param dev GPIO 设备句柄
 * @param port 端口（由平台定义）
 * @param pin 引脚号（0-15）
 * @param mode 模式
 * @param speed 速度
 * @return GPIO_OK 成功，其他失败
 */
int gpio_dev_init(gpio_dev_t *dev, void *port, uint16_t pin, 
                  gpio_mode_t mode, gpio_speed_t speed);

/**
 * @brief 设置为输出模式
 * 
 * @param dev GPIO 设备句柄
 * @param initial_level 初始电平
 * @return GPIO_OK 成功
 */
int gpio_dev_set_output(gpio_dev_t *dev, gpio_level_t initial_level);

/**
 * @brief 设置为输入模式
 * 
 * @param dev GPIO 设备句柄
 * @param pull 上拉/下拉（GPIO_MODE_INPUT/PULLUP/PULLDOWN）
 * @return GPIO_OK 成功
 */
int gpio_dev_set_input(gpio_dev_t *dev, gpio_mode_t pull);

/**
 * @brief 设置为开漏输出
 * 
 * @param dev GPIO 设备句柄
 * @param pull 上拉/下拉
 * @return GPIO_OK 成功
 */
int gpio_dev_set_output_od(gpio_dev_t *dev, gpio_mode_t pull);

/*============================================================================
 * 基本操作
 *============================================================================*/

/**
 * @brief 写入高电平
 * 
 * @param dev GPIO 设备句柄
 */
static inline void gpio_dev_set(gpio_dev_t *dev)
{
    if (dev && dev->dir == GPIO_DIR_OUTPUT) {
        gpio_level_t level = dev->inverted ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH;
        if (g_gpio_ops.write) {
            g_gpio_ops.write(dev, level);
        }
    }
}

/**
 * @brief 写入低电平
 * 
 * @param dev GPIO 设备句柄
 */
static inline void gpio_dev_clear(gpio_dev_t *dev)
{
    if (dev && dev->dir == GPIO_DIR_OUTPUT) {
        gpio_level_t level = dev->inverted ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW;
        if (g_gpio_ops.write) {
            g_gpio_ops.write(dev, level);
        }
    }
}

/**
 * @brief 读取电平（返回逻辑电平）
 * 
 * @param dev GPIO 设备句柄
 * @return GPIO_LEVEL_LOW/HIGH
 */
gpio_level_t gpio_dev_read(gpio_dev_t *dev);

/**
 * @brief 写入电平
 * 
 * @param dev GPIO 设备句柄
 * @param level 电平
 */
void gpio_dev_write(gpio_dev_t *dev, gpio_level_t level);

/**
 * @brief 切换电平
 * 
 * @param dev GPIO 设备句柄
 */
void gpio_dev_toggle(gpio_dev_t *dev);

/**
 * @brief 读取原始电平（不反相）
 * 
 * @param dev GPIO 设备句柄
 * @return GPIO_LEVEL_LOW/HIGH
 */
gpio_level_t gpio_dev_read_raw(gpio_dev_t *dev);

/*============================================================================
 * 便捷操作（位带操作风格）
 *============================================================================*/

/**
 * @brief 原子置位
 * 
 * @param dev GPIO 设备句柄
 */
void gpio_dev_set_bit(gpio_dev_t *dev);

/**
 * @brief 原子清零
 * 
 * @param dev GPIO 设备句柄
 */
void gpio_dev_clear_bit(gpio_dev_t *dev);

/**
 * @brief 原子切换
 * 
 * @param dev GPIO 设备句柄
 */
void gpio_dev_toggle_bit(gpio_dev_t *dev);

/**
 * @brief 原子写入
 * 
 * @param dev GPIO 设备句柄
 * @param level 电平
 */
void gpio_dev_write_bit(gpio_dev_t *dev, gpio_level_t level);

/*============================================================================
 * 中断管理
 *============================================================================*/

/**
 * @brief 配置中断
 * 
 * @param dev GPIO 设备句柄
 * @param edge 触发边沿
 * @param callback 回调函数
 * @param user_data 用户数据
 * @return GPIO_OK 成功
 */
int gpio_dev_irq_config(gpio_dev_t *dev, gpio_edge_t edge, 
                        gpio_irq_callback_t callback, void *user_data);

/**
 * @brief 使能中断
 * 
 * @param dev GPIO 设备句柄
 */
void gpio_dev_irq_enable(gpio_dev_t *dev);

/**
 * @brief 禁用中断
 * 
 * @param dev GPIO 设备句柄
 */
void gpio_dev_irq_disable(gpio_dev_t *dev);

/*============================================================================
 * 高级功能
 *============================================================================*/

/**
 * @brief 设置反相模式
 * 
 * @param dev GPIO 设备句柄
 * @param inverted true-反相，false-正常
 */
void gpio_dev_set_inverted(gpio_dev_t *dev, bool inverted);

/**
 * @brief 等待电平变化（轮询）
 * 
 * @param dev GPIO 设备句柄
 * @param target_level 目标电平
 * @param timeout_ms 超时时间（ms）
 * @return GPIO_OK 成功，GPIO_ERR_TIMEOUT 超时
 */
int gpio_dev_wait_level(gpio_dev_t *dev, gpio_level_t target_level, uint32_t timeout_ms);

/**
 * @brief 产生脉冲
 * 
 * @param dev GPIO 设备句柄
 * @param count 脉冲数量
 * @param delay_us 半周期延时（微秒）
 */
void gpio_dev_pulse(gpio_dev_t *dev, uint32_t count, uint32_t delay_us);

/**
 * @brief 读取多个 GPIO（端口操作）
 * 
 * @param port 端口
 * @param mask 引脚掩码
 * @return 引脚状态
 */
uint16_t gpio_dev_read_port(void *port, uint16_t mask);

/**
 * @brief 写入多个 GPIO（端口操作）
 * 
 * @param port 端口
 * @param mask 引脚掩码
 * @param value 值
 */
void gpio_dev_write_port(void *port, uint16_t mask, uint16_t value);

/*============================================================================
 * 平台接口注册（移植用）
 *============================================================================*/

/**
 * @brief 注册 GPIO 平台操作接口
 * 
 * @param ops 平台操作接口
 */
void gpio_dev_register_platform(const gpio_platform_ops_t *ops);

/* 全局操作接口（内部使用） */
extern gpio_platform_ops_t g_gpio_ops;

#ifdef __cplusplus
}
#endif

#endif /* GPIO_DEV_H */
