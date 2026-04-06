/**
 * @file fixed_point.h
 * @brief 定点数运算库 - 替代浮点运算（适合无 FPU 的 MCU）
 * 
 * @author Your Name
 * @version 1.0.0
 * @date 2026-04-06
 * 
 * @copyright MIT License
 */

#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * 配置
 *============================================================================*/

/**
 * @brief 定点数精度配置
 * 
 * Q 格式：Qm.n，其中 m 为整数位数，n 为小数位数
 * 本库默认使用 Q16.16 格式（32 位）
 */
#define FIXED_POINT_BITS    32          /**< 总位数 */
#define FIXED_POINT_FRAC    16          /**< 小数位数 */
#define FIXED_POINT_INT     15          /**< 整数位数（含符号） */

/**
 * @brief 定点数类型定义
 */
typedef int32_t fixed_t;                /**< 32 位定点数 */
typedef int64_t fixed_long_t;           /**< 64 位定点数（用于中间计算）

/*============================================================================
 * 基本转换
 *============================================================================*/

/**
 * @brief 浮点数转定点数
 * 
 * @param x 浮点数
 * @return fixed_t 定点数
 */
fixed_t float_to_fixed(float x);

/**
 * @brief 定点数转浮点数
 * 
 * @param x 定点数
 * @return float 浮点数
 */
float fixed_to_float(fixed_t x);

/**
 * @brief 整数转定点数
 * 
 * @param x 整数
 * @return fixed_t 定点数
 */
fixed_t int_to_fixed(int x);

/**
 * @brief 定点数转整数（四舍五入）
 * 
 * @param x 定点数
 * @return int 整数
 */
int fixed_to_int(fixed_t x);

/**
 * @brief 定点数常量宏（编译时计算）
 * 
 * 使用示例：FIXED(3.14) → 将 3.14 转为定点数
 */
#define FIXED(x)  ((fixed_t)((x) * (1 << FIXED_POINT_FRAC)))

/*============================================================================
 * 基本运算
 *============================================================================*/

/**
 * @brief 定点数加法
 * 
 * @param a 操作数 1
 * @param b 操作数 2
 * @return fixed_t a + b
 */
static inline fixed_t fixed_add(fixed_t a, fixed_t b)
{
    return a + b;
}

/**
 * @brief 定点数减法
 * 
 * @param a 操作数 1
 * @param b 操作数 2
 * @return fixed_t a - b
 */
static inline fixed_t fixed_sub(fixed_t a, fixed_t b)
{
    return a - b;
}

/**
 * @brief 定点数乘法
 * 
 * @param a 操作数 1
 * @param b 操作数 2
 * @return fixed_t a * b
 */
fixed_t fixed_mul(fixed_t a, fixed_t b);

/**
 * @brief 定点数除法
 * 
 * @param a 被除数
 * @param b 除数
 * @return fixed_t a / b
 */
fixed_t fixed_div(fixed_t a, fixed_t b);

/**
 * @brief 定点数取反
 * 
 * @param a 操作数
 * @return fixed_t -a
 */
static inline fixed_t fixed_neg(fixed_t a)
{
    return -a;
}

/**
 * @brief 定点数绝对值
 * 
 * @param a 操作数
 * @return fixed_t |a|
 */
static inline fixed_t fixed_abs(fixed_t a)
{
    return (a < 0) ? -a : a;
}

/*============================================================================
 * 比较运算
 *============================================================================*/

/**
 * @brief 定点数比较（等于）
 * 
 * @param a 操作数 1
 * @param b 操作数 2
 * @return true a == b
 * @return false a != b
 */
static inline bool fixed_eq(fixed_t a, fixed_t b)
{
    return a == b;
}

/**
 * @brief 定点数比较（大于）
 * 
 * @param a 操作数 1
 * @param b 操作数 2
 * @return true a > b
 * @return false a <= b
 */
static inline bool fixed_gt(fixed_t a, fixed_t b)
{
    return a > b;
}

/**
 * @brief 定点数比较（小于）
 * 
 * @param a 操作数 1
 * @param b 操作数 2
 * @return true a < b
 * @return false a >= b
 */
static inline bool fixed_lt(fixed_t a, fixed_t b)
{
    return a < b;
}

/**
 * @brief 定点数比较（大于等于）
 * 
 * @param a 操作数 1
 * @param b 操作数 2
 * @return true a >= b
 * @return false a < b
 */
static inline bool fixed_ge(fixed_t a, fixed_t b)
{
    return a >= b;
}

/**
 * @brief 定点数比较（小于等于）
 * 
 * @param a 操作数 1
 * @param b 操作数 2
 * @return true a <= b
 * @return false a > b
 */
static inline bool fixed_le(fixed_t a, fixed_t b)
{
    return a <= b;
}

/*============================================================================
 * 数学函数
 *============================================================================*/

/**
 * @brief 定点数平方根
 * 
 * @param x 被开方数（必须 >= 0）
 * @return fixed_t sqrt(x)
 */
fixed_t fixed_sqrt(fixed_t x);

/**
 * @brief 定点数乘法累加（MAC 操作）
 * 
 * @param acc 累加器
 * @param a 操作数 1
 * @param b 操作数 2
 * @return fixed_t acc + a * b
 */
fixed_t fixed_mac(fixed_t acc, fixed_t a, fixed_t b);

/**
 * @brief 定点数限幅
 * 
 * @param x 输入值
 * @param min 最小值
 * @param max 最大值
 * @return fixed_t 限幅后的值
 */
fixed_t fixed_clamp(fixed_t x, fixed_t min, fixed_t max);

/**
 * @brief 定点数线性插值
 * 
 * @param a 起点值
 * @param b 终点值
 * @param t 插值因子（0.0-1.0，定点数）
 * @return fixed_t a + (b - a) * t
 */
fixed_t fixed_lerp(fixed_t a, fixed_t b, fixed_t t);

/*============================================================================
 * 常用常数
 *============================================================================*/

/**
 * @brief 常用定点数常数
 */
#define FIXED_ZERO      FIXED(0.0)
#define FIXED_ONE       FIXED(1.0)
#define FIXED_TWO       FIXED(2.0)
#define FIXED_HALF      FIXED(0.5)
#define FIXED_PI        FIXED(3.14159265358979)
#define FIXED_E         FIXED(2.71828182845904)
#define FIXED_SQRT2     FIXED(1.41421356237309)

#ifdef __cplusplus
}
#endif

#endif /* FIXED_POINT_H */
