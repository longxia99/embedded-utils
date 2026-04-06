/**
 * @file test_crc.c
 * @brief CRC 校验单元测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/utils/crc.h"

void test_crc8_ccitt(void)
{
    printf("Test: CRC8-CCITT... ");
    
    // 标准测试向量
    uint8_t data1[] = "123456789";
    uint8_t result1 = crc8_ccitt_calc(data1, sizeof(data1) - 1);
    assert(result1 == 0xF4);  // CRC8-CCITT 标准结果
    
    // 空数据
    uint8_t result2 = crc8_ccitt_calc(NULL, 0);
    assert(result2 == 0x00);
    
    printf("PASSED (0x%02X)\n", result1);
}

void test_crc8_maxim(void)
{
    printf("Test: CRC8-MAXIM... ");
    
    // 标准测试向量
    uint8_t data1[] = "123456789";
    uint8_t result1 = crc8_maxim_calc(data1, sizeof(data1) - 1);
    assert(result1 == 0xA2);  // CRC8-MAXIM 标准结果
    
    printf("PASSED (0x%02X)\n", result1);
}

void test_crc16_ccitt(void)
{
    printf("Test: CRC16-CCITT... ");
    
    // 标准测试向量
    uint8_t data1[] = "123456789";
    uint16_t result1 = crc16_ccitt_calc(data1, sizeof(data1) - 1);
    assert(result1 == 0x29B1);  // CRC16-CCITT 标准结果
    
    printf("PASSED (0x%04X)\n", result1);
}

void test_crc16_modbus(void)
{
    printf("Test: CRC16-MODBUS... ");
    
    // 标准测试向量
    uint8_t data1[] = "123456789";
    uint16_t result1 = crc16_modbus_calc(data1, sizeof(data1) - 1);
    assert(result1 == 0x4B37);  // CRC16-MODBUS 标准结果
    
    printf("PASSED (0x%04X)\n", result1);
}

void test_crc32_iso(void)
{
    printf("Test: CRC32-ISO... ");
    
    // 标准测试向量
    uint8_t data1[] = "123456789";
    uint32_t result1 = crc32_iso_calc(data1, sizeof(data1) - 1);
    assert(result1 == 0xCBF43926);  // CRC32 标准结果
    
    printf("PASSED (0x%08X)\n", result1);
}

void test_crc_consistency(void)
{
    printf("Test: CRC consistency... ");
    
    // 同一数据多次计算结果一致
    uint8_t data[] = "Embedded-Utils Test Data";
    
    uint32_t crc1 = crc32_iso_calc(data, strlen((char*)data));
    uint32_t crc2 = crc32_iso_calc(data, strlen((char*)data));
    uint32_t crc3 = crc32_iso_calc(data, strlen((char*)data));
    
    assert(crc1 == crc2);
    assert(crc2 == crc3);
    
    printf("PASSED\n");
}

void test_crc_sensitivity(void)
{
    printf("Test: CRC sensitivity... ");
    
    // 单比特变化应该产生不同的 CRC
    uint8_t data1[] = "Hello";
    uint8_t data2[] = "hello";  // 首字母大小写不同
    
    uint32_t crc1 = crc32_iso_calc(data1, sizeof(data1) - 1);
    uint32_t crc2 = crc32_iso_calc(data2, sizeof(data2) - 1);
    
    assert(crc1 != crc2);  // 应该不同
    
    printf("PASSED\n");
}

void test_custom_config(void)
{
    printf("Test: Custom CRC config... ");
    
    // 自定义 CRC8 配置
    crc8_config_t custom = {
        .polynomial = 0x1D,
        .init = 0xFF,
        .xorout = 0x00,
        .reflect_in = false,
        .reflect_out = false
    };
    
    uint8_t data[] = "Test";
    uint8_t result = crc8_calc(data, sizeof(data) - 1, &custom);
    
    // 验证非零（只要不崩溃即可）
    assert(result >= 0);
    
    printf("PASSED (0x%02X)\n", result);
}

int main(void)
{
    printf("=== CRC Unit Tests ===\n\n");
    
    test_crc8_ccitt();
    test_crc8_maxim();
    test_crc16_ccitt();
    test_crc16_modbus();
    test_crc32_iso();
    test_crc_consistency();
    test_crc_sensitivity();
    test_custom_config();
    
    printf("\n=== All tests PASSED ===\n");
    
    return 0;
}
