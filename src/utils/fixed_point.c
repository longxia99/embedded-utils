/**
 * @file fixed_point.c
 * @brief 定点数运算库实现
 */

#include "fixed_point.h"

/*============================================================================
 * 基本转换
 *============================================================================*/

fixed_t float_to_fixed(float x)
{
    return (fixed_t)(x * (1 << FIXED_POINT_FRAC));
}

float fixed_to_float(fixed_t x)
{
    return (float)x / (1 << FIXED_POINT_FRAC);
}

fixed_t int_to_fixed(int x)
{
    return (fixed_t)(x << FIXED_POINT_FRAC);
}

int fixed_to_int(fixed_t x)
{
    // 四舍五入
    fixed_t half = FIXED_HALF;
    if (x >= 0) {
        return (int)((x + half) >> FIXED_POINT_FRAC);
    } else {
        return (int)((x - half) >> FIXED_POINT_FRAC);
    }
}

/*============================================================================
 * 基本运算
 *============================================================================*/

fixed_t fixed_mul(fixed_t a, fixed_t b)
{
    // 使用 64 位中间结果防止溢出
    fixed_long_t result = ((fixed_long_t)a * (fixed_long_t)b);
    return (fixed_t)(result >> FIXED_POINT_FRAC);
}

fixed_t fixed_div(fixed_t a, fixed_t b)
{
    if (b == 0) {
        return 0;  // 除零保护
    }
    
    // 使用 64 位中间结果
    fixed_long_t result = ((fixed_long_t)a << FIXED_POINT_FRAC);
    return (fixed_t)(result / b);
}

/*============================================================================
 * 数学函数
 *============================================================================*/

fixed_t fixed_sqrt(fixed_t x)
{
    if (x <= 0) {
        return 0;
    }
    
    // 牛顿迭代法求平方根
    fixed_t guess = x >> 1;  // 初始猜测
    if (guess == 0) {
        guess = 1;
    }
    
    // 迭代 10 次（通常 5-6 次就足够收敛）
    for (int i = 0; i < 10; i++) {
        fixed_t next = fixed_div(fixed_add(guess, fixed_div(x, guess)), FIXED_TWO);
        if (next == guess) {
            break;
        }
        guess = next;
    }
    
    return guess;
}

fixed_t fixed_mac(fixed_t acc, fixed_t a, fixed_t b)
{
    return fixed_add(acc, fixed_mul(a, b));
}

fixed_t fixed_clamp(fixed_t x, fixed_t min, fixed_t max)
{
    if (x < min) {
        return min;
    } else if (x > max) {
        return max;
    }
    return x;
}

fixed_t fixed_lerp(fixed_t a, fixed_t b, fixed_t t)
{
    // lerp = a + (b - a) * t
    fixed_t diff = fixed_sub(b, a);
    fixed_t scaled = fixed_mul(diff, t);
    return fixed_add(a, scaled);
}
