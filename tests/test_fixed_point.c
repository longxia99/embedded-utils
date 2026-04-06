/**
 * @file test_fixed_point.c
 * @brief 定点数运算单元测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "../include/utils/fixed_point.h"

#define EPSILON  0.01f  /**< 浮点比较误差容限 */

/**
 * @brief 比较浮点数和定点数（转换后）
 */
static void assert_fixed_near(fixed_t fixed_val, float expected)
{
    float actual = fixed_to_float(fixed_val);
    float diff = fabsf(actual - expected);
    assert(diff < EPSILON);
}

void test_float_conversion(void)
{
    printf("Test: float conversion... ");
    
    // 浮点数转换
    float test_values[] = {0.0f, 1.0f, -1.0f, 3.14f, -2.71f, 0.5f, 100.123f};
    size_t count = sizeof(test_values) / sizeof(test_values[0]);
    
    for (size_t i = 0; i < count; i++) {
        fixed_t fixed = float_to_fixed(test_values[i]);
        float back = fixed_to_float(fixed);
        assert(fabsf(back - test_values[i]) < EPSILON);
    }
    
    // 整数转换
    assert(int_to_fixed(5) == FIXED(5.0));
    assert(fixed_to_int(FIXED(10.0)) == 10);
    assert(fixed_to_int(FIXED(3.7)) == 4);  // 四舍五入
    assert(fixed_to_int(FIXED(3.2)) == 3);
    assert(fixed_to_int(FIXED(-3.7)) == -4);
    
    printf("PASSED\n");
}

void test_basic_arithmetic(void)
{
    printf("Test: basic arithmetic... ");
    
    fixed_t a = FIXED(10.5);
    fixed_t b = FIXED(3.25);
    
    // 加法
    fixed_t sum = fixed_add(a, b);
    assert_fixed_near(sum, 13.75f);
    
    // 减法
    fixed_t diff = fixed_sub(a, b);
    assert_fixed_near(diff, 7.25f);
    
    // 乘法
    fixed_t prod = fixed_mul(a, b);
    assert_fixed_near(prod, 34.125f);
    
    // 除法
    fixed_t quot = fixed_div(a, b);
    assert_fixed_near(quot, 3.230769f);
    
    // 取反
    fixed_t neg = fixed_neg(a);
    assert_fixed_near(neg, -10.5f);
    
    // 绝对值
    fixed_t abs_val = fixed_abs(FIXED(-5.5));
    assert_fixed_near(abs_val, 5.5f);
    
    printf("PASSED\n");
}

void test_comparison(void)
{
    printf("Test: comparison... ");
    
    fixed_t a = FIXED(5.0);
    fixed_t b = FIXED(3.0);
    fixed_t c = FIXED(5.0);
    
    assert(fixed_eq(a, c) == true);
    assert(fixed_eq(a, b) == false);
    
    assert(fixed_gt(a, b) == true);
    assert(fixed_gt(b, a) == false);
    
    assert(fixed_lt(b, a) == true);
    assert(fixed_lt(a, b) == false);
    
    assert(fixed_ge(a, c) == true);
    assert(fixed_ge(a, b) == true);
    
    assert(fixed_le(b, a) == true);
    assert(fixed_le(a, c) == true);
    
    printf("PASSED\n");
}

void test_sqrt(void)
{
    printf("Test: sqrt... ");
    
    // 测试完全平方数
    fixed_t sqrt4 = fixed_sqrt(FIXED(4.0));
    assert_fixed_near(sqrt4, 2.0f);
    
    fixed_t sqrt9 = fixed_sqrt(FIXED(9.0));
    assert_fixed_near(sqrt9, 3.0f);
    
    fixed_t sqrt16 = fixed_sqrt(FIXED(16.0));
    assert_fixed_near(sqrt16, 4.0f);
    
    // 测试非完全平方数
    fixed_t sqrt2 = fixed_sqrt(FIXED(2.0));
    assert_fixed_near(sqrt2, 1.414f);
    
    fixed_t sqrt10 = fixed_sqrt(FIXED(10.0));
    assert_fixed_near(sqrt10, 3.162f);
    
    // 边界情况
    assert_fixed_near(fixed_sqrt(FIXED(0.0)), 0.0f);
    assert_fixed_near(fixed_sqrt(FIXED(1.0)), 1.0f);
    
    printf("PASSED\n");
}

void test_mac(void)
{
    printf("Test: MAC operation... ");
    
    // MAC: acc + a * b
    fixed_t acc = FIXED(10.0);
    fixed_t a = FIXED(3.0);
    fixed_t b = FIXED(4.0);
    
    fixed_t result = fixed_mac(acc, a, b);
    assert_fixed_near(result, 22.0f);  // 10 + 3*4 = 22
    
    // 多次 MAC（模拟 FIR 滤波）
    acc = FIXED_ZERO;
    fixed_t x[] = {FIXED(1.0), FIXED(2.0), FIXED(3.0)};
    fixed_t h[] = {FIXED(0.5), FIXED(0.3), FIXED(0.2)};
    
    for (int i = 0; i < 3; i++) {
        acc = fixed_mac(acc, x[i], h[i]);
    }
    assert_fixed_near(acc, 1.7f);  // 1*0.5 + 2*0.3 + 3*0.2 = 1.7
    
    printf("PASSED\n");
}

void test_clamp(void)
{
    printf("Test: clamp... ");
    
    fixed_t min = FIXED(0.0);
    fixed_t max = FIXED(10.0);
    
    // 正常范围
    assert_fixed_near(fixed_clamp(FIXED(5.0), min, max), 5.0f);
    
    // 低于最小值
    assert_fixed_near(fixed_clamp(FIXED(-5.0), min, max), 0.0f);
    
    // 高于最大值
    assert_fixed_near(fixed_clamp(FIXED(15.0), min, max), 10.0f);
    
    printf("PASSED\n");
}

void test_lerp(void)
{
    printf("Test: lerp... ");
    
    fixed_t a = FIXED(10.0);
    fixed_t b = FIXED(20.0);
    
    // t = 0 → 返回 a
    fixed_t t0 = FIXED(0.0);
    assert_fixed_near(fixed_lerp(a, b, t0), 10.0f);
    
    // t = 0.5 → 返回中点
    fixed_t t05 = FIXED(0.5);
    assert_fixed_near(fixed_lerp(a, b, t05), 15.0f);
    
    // t = 1 → 返回 b
    fixed_t t1 = FIXED(1.0);
    assert_fixed_near(fixed_lerp(a, b, t1), 20.0f);
    
    printf("PASSED\n");
}

void test_constants(void)
{
    printf("Test: constants... ");
    
    assert_fixed_near(FIXED_ZERO, 0.0f);
    assert_fixed_near(FIXED_ONE, 1.0f);
    assert_fixed_near(FIXED_TWO, 2.0f);
    assert_fixed_near(FIXED_HALF, 0.5f);
    assert_fixed_near(FIXED_PI, 3.14159f);
    assert_fixed_near(FIXED_E, 2.71828f);
    assert_fixed_near(FIXED_SQRT2, 1.41421f);
    
    printf("PASSED\n");
}

void test_macro(void)
{
    printf("Test: FIXED macro... ");
    
    // 编译时常量宏
    fixed_t pi = FIXED(3.14159);
    assert_fixed_near(pi, 3.14159f);
    
    fixed_t e = FIXED(2.71828);
    assert_fixed_near(e, 2.71828f);
    
    printf("PASSED\n");
}

int main(void)
{
    printf("=== Fixed Point Unit Tests ===\n\n");
    
    test_float_conversion();
    test_basic_arithmetic();
    test_comparison();
    test_sqrt();
    test_mac();
    test_clamp();
    test_lerp();
    test_constants();
    test_macro();
    
    printf("\n=== All tests PASSED ===\n");
    
    return 0;
}
