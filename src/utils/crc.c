/**
 * @file crc.c
 * @brief CRC 校验计算实现 - CRC8/CRC16/CRC32
 */

#include "crc.h"
#include <stdbool.h>

/*============================================================================
 * 辅助函数
 *============================================================================*/

/**
 * @brief 反转字节
 */
static uint8_t reflect8(uint8_t data)
{
    uint8_t result = 0;
    for (int i = 0; i < 8; i++) {
        if (data & (1 << i)) {
            result |= (1 << (7 - i));
        }
    }
    return result;
}

/**
 * @brief 反转 16 位
 */
static uint16_t reflect16(uint16_t data)
{
    uint16_t result = 0;
    for (int i = 0; i < 16; i++) {
        if (data & (1 << i)) {
            result |= (1 << (15 - i));
        }
    }
    return result;
}

/**
 * @brief 反转 32 位
 */
static uint32_t reflect32(uint32_t data)
{
    uint32_t result = 0;
    for (int i = 0; i < 32; i++) {
        if (data & (1U << i)) {
            result |= (1U << (31 - i));
        }
    }
    return result;
}

/*============================================================================
 * CRC8 实现
 *============================================================================*/

const crc8_config_t crc8_ccitt = {
    .polynomial = 0x07,
    .init = 0x00,
    .xorout = 0x00,
    .reflect_in = false,
    .reflect_out = false
};

const crc8_config_t crc8_maxim = {
    .polynomial = 0x31,
    .init = 0x00,
    .xorout = 0x00,
    .reflect_in = false,
    .reflect_out = false
};

const crc8_config_t crc8_dallas = {
    .polynomial = 0x31,
    .init = 0x00,
    .xorout = 0x00,
    .reflect_in = false,
    .reflect_out = false
};

uint8_t crc8_calc(const uint8_t *data, size_t length, const crc8_config_t *config)
{
    if (data == NULL || length == 0) {
        return 0;
    }
    
    if (config == NULL) {
        config = &crc8_ccitt;
    }
    
    uint8_t crc = config->init;
    
    for (size_t i = 0; i < length; i++) {
        uint8_t byte = config->reflect_in ? reflect8(data[i]) : data[i];
        crc ^= byte;
        
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ config->polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    
    if (config->reflect_out) {
        crc = reflect8(crc);
    }
    
    return crc ^ config->xorout;
}

uint8_t crc8_ccitt_calc(const uint8_t *data, size_t length)
{
    return crc8_calc(data, length, &crc8_ccitt);
}

uint8_t crc8_maxim_calc(const uint8_t *data, size_t length)
{
    return crc8_calc(data, length, &crc8_maxim);
}

/*============================================================================
 * CRC16 实现
 *============================================================================*/

const crc16_config_t crc16_ibm = {
    .polynomial = 0x8005,
    .init = 0x0000,
    .xorout = 0x0000,
    .reflect_in = true,
    .reflect_out = true
};

const crc16_config_t crc16_ccitt = {
    .polynomial = 0x1021,
    .init = 0xFFFF,
    .xorout = 0x0000,
    .reflect_in = false,
    .reflect_out = false
};

const crc16_config_t crc16_modbus = {
    .polynomial = 0x8005,
    .init = 0xFFFF,
    .xorout = 0x0000,
    .reflect_in = true,
    .reflect_out = true
};

const crc16_config_t crc16_usb = {
    .polynomial = 0x8005,
    .init = 0xFFFF,
    .xorout = 0xFFFF,
    .reflect_in = true,
    .reflect_out = true
};

uint16_t crc16_calc(const uint8_t *data, size_t length, const crc16_config_t *config)
{
    if (data == NULL || length == 0) {
        return 0;
    }
    
    if (config == NULL) {
        config = &crc16_ccitt;
    }
    
    uint16_t crc = config->init;
    
    for (size_t i = 0; i < length; i++) {
        uint16_t byte = config->reflect_in ? reflect8(data[i]) : data[i];
        crc ^= (byte << 8);
        
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ config->polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    
    if (config->reflect_out) {
        crc = reflect16(crc);
    }
    
    return crc ^ config->xorout;
}

uint16_t crc16_ccitt_calc(const uint8_t *data, size_t length)
{
    return crc16_calc(data, length, &crc16_ccitt);
}

uint16_t crc16_modbus_calc(const uint8_t *data, size_t length)
{
    return crc16_calc(data, length, &crc16_modbus);
}

/*============================================================================
 * CRC32 实现
 *============================================================================*/

const crc32_config_t crc32_ethernet = {
    .polynomial = 0x04C11DB7,
    .init = 0xFFFFFFFF,
    .xorout = 0xFFFFFFFF,
    .reflect_in = true,
    .reflect_out = true
};

const crc32_config_t crc32_iso = {
    .polynomial = 0x04C11DB7,
    .init = 0xFFFFFFFF,
    .xorout = 0xFFFFFFFF,
    .reflect_in = true,
    .reflect_out = true
};

/**
 * @brief CRC32 查表（256 项）
 */
static uint32_t crc32_table[256];
static bool crc32_table_initialized = false;

void crc32_init_table(void)
{
    if (crc32_table_initialized) {
        return;
    }
    
    const uint32_t polynomial = 0xEDB88320;  // 反转后的多项式
    
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
        crc32_table[i] = crc;
    }
    
    crc32_table_initialized = true;
}

uint32_t crc32_calc(const uint8_t *data, size_t length, const crc32_config_t *config)
{
    if (data == NULL || length == 0) {
        return 0;
    }
    
    if (config == NULL) {
        config = &crc32_iso;
    }
    
    // 使用查表法（仅适用于标准 CRC32）
    if (config == &crc32_iso || config == &crc32_ethernet) {
        crc32_init_table();
        
        uint32_t crc = config->init;
        
        for (size_t i = 0; i < length; i++) {
            crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
        }
        
        return crc ^ config->xorout;
    }
    
    // 通用计算法
    uint32_t crc = config->init;
    
    for (size_t i = 0; i < length; i++) {
        uint32_t byte = config->reflect_in ? reflect8(data[i]) : data[i];
        crc ^= (byte << 24);
        
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ config->polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    
    if (config->reflect_out) {
        crc = reflect32(crc);
    }
    
    return crc ^ config->xorout;
}

uint32_t crc32_iso_calc(const uint8_t *data, size_t length)
{
    return crc32_calc(data, length, &crc32_iso);
}
