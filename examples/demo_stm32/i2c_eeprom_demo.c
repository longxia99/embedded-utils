/**
 * @file i2c_eeprom_demo.c
 * @brief STM32 平台 I2C EEPROM 使用示例（AT24C02）
 * 
 * 硬件连接：
 * - I2C1_SCL -> PB6
 * - I2C1_SDA -> PB7
 * - EEPROM A0/A1/A2 -> GND (地址 0x50)
 */

#include "stm32f1xx_hal.h"
#include "../../include/drivers/i2c_dev.h"

// EEPROM 设备句柄
static i2c_dev_t eeprom_dev;

// I2C 句柄（由 HAL 提供）
extern I2C_HandleTypeDef hi2c1;

/*============================================================================
 * 平台层实现（STM32 HAL）
 *============================================================================*/

/**
 * @brief STM32 I2C 写操作
 */
static int stm32_i2c_write(void *handle, uint16_t dev_addr, const uint8_t *data, size_t len, uint32_t timeout)
{
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
    
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, 
                                                        dev_addr << 1,  // 7 位转 8 位
                                                        (uint8_t *)data, 
                                                        len, 
                                                        timeout);
    
    if (status == HAL_OK) {
        return I2C_OK;
    } else if (status == HAL_ERROR) {
        return I2C_ERR_NACK;
    } else if (status == HAL_TIMEOUT) {
        return I2C_ERR_TIMEOUT;
    }
    return I2C_ERR_BUS;
}

/**
 * @brief STM32 I2C 读操作
 */
static int stm32_i2c_read(void *handle, uint16_t dev_addr, uint8_t *data, size_t len, uint32_t timeout)
{
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
    
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(hi2c, 
                                                       dev_addr << 1, 
                                                       data, 
                                                       len, 
                                                       timeout);
    
    if (status == HAL_OK) {
        return I2C_OK;
    } else if (status == HAL_ERROR) {
        return I2C_ERR_NACK;
    } else if (status == HAL_TIMEOUT) {
        return I2C_ERR_TIMEOUT;
    }
    return I2C_ERR_BUS;
}

/**
 * @brief STM32 I2C 写 + 读操作（重复 START）
 */
static int stm32_i2c_write_read(void *handle, uint16_t dev_addr, 
                                const uint8_t *write_data, size_t write_len,
                                uint8_t *read_data, size_t read_len, 
                                uint32_t timeout)
{
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)handle;
    
    // 先写入寄存器地址
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, 
                                                        dev_addr << 1, 
                                                        (uint8_t *)write_data, 
                                                        write_len, 
                                                        timeout);
    if (status != HAL_OK) {
        return I2C_ERR_NACK;
    }
    
    // 重复 START，读取数据
    status = HAL_I2C_Master_Receive(hi2c, 
                                    dev_addr << 1, 
                                    read_data, 
                                    read_len, 
                                    timeout);
    
    if (status == HAL_OK) {
        return I2C_OK;
    } else if (status == HAL_ERROR) {
        return I2C_ERR_NACK;
    } else if (status == HAL_TIMEOUT) {
        return I2C_ERR_TIMEOUT;
    }
    return I2C_ERR_BUS;
}

/**
 * @brief I2C 平台操作接口
 */
static const i2c_platform_ops_t stm32_i2c_ops = {
    .write = stm32_i2c_write,
    .read = stm32_i2c_read,
    .write_read = stm32_i2c_write_read
};

/*============================================================================
 * 应用层示例
 *============================================================================*/

/**
 * @brief 初始化 EEPROM
 */
int eeprom_init(void)
{
    // 注册平台接口
    i2c_dev_register_platform(&stm32_i2c_ops);
    
    // 初始化设备（AT24C02 地址 0x50）
    return i2c_dev_init(&eeprom_dev, &hi2c1, 0x50, 100);
}

/**
 * @brief 检测 EEPROM 是否存在
 */
int eeprom_detect(void)
{
    return i2c_dev_scan(&eeprom_dev);
}

/**
 * @brief 写入一个字节
 * 
 * @param addr EEPROM 地址（0-255）
 * @param data 数据
 * @return I2C_OK 成功
 */
int eeprom_write_byte(uint8_t addr, uint8_t data)
{
    return i2c_dev_write_reg8(&eeprom_dev, addr, data);
}

/**
 * @brief 读取一个字节
 * 
 * @param addr EEPROM 地址
 * @param data 读取的数据
 * @return I2C_OK 成功
 */
int eeprom_read_byte(uint8_t addr, uint8_t *data)
{
    return i2c_dev_read_reg8(&eeprom_dev, addr, data);
}

/**
 * @brief 写入多字节（页写入，最多 8 字节）
 * 
 * @param addr 起始地址
 * @param data 数据指针
 * @param len 长度（<=8）
 * @return I2C_OK 成功
 */
int eeprom_write_page(uint8_t addr, const uint8_t *data, size_t len)
{
    if (len > 8) {
        len = 8;  // AT24C02 页大小 8 字节
    }
    return i2c_dev_write_reg(&eeprom_dev, addr, data, len);
}

/**
 * @brief 读取多字节
 * 
 * @param addr 起始地址
 * @param data 数据指针
 * @param len 长度
 * @return I2C_OK 成功
 */
int eeprom_read_bytes(uint8_t addr, uint8_t *data, size_t len)
{
    return i2c_dev_read_reg(&eeprom_dev, addr, data, len);
}

/*============================================================================
 * 使用示例
 *============================================================================*/

int main(void)
{
    // HAL 初始化
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    
    // 初始化 EEPROM
    if (eeprom_init() != I2C_OK) {
        // 错误处理
        while (1);
    }
    
    // 检测 EEPROM
    if (eeprom_detect() != I2C_OK) {
        // 未找到设备
        while (1);
    }
    
    // 示例 1：写入一个字节
    uint8_t write_data = 0x5A;
    eeprom_write_byte(0x00, write_data);
    HAL_Delay(10);  // 等待写入完成（EEPROM 需要 5ms）
    
    // 示例 2：读取一个字节
    uint8_t read_data;
    eeprom_read_byte(0x00, &read_data);
    // read_data 应该等于 0x5A
    
    // 示例 3：写入多字节
    uint8_t buffer_write[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    eeprom_write_page(0x10, buffer_write, 5);
    HAL_Delay(10);
    
    // 示例 4：读取多字节
    uint8_t buffer_read[5];
    eeprom_read_bytes(0x10, buffer_read, 5);
    // buffer_read 应该等于 {0x01, 0x02, 0x03, 0x04, 0x05}
    
    // 示例 5：修改寄存器特定位
    // 读取当前值，修改 bit0-3，保留 bit4-7
    i2c_dev_modify_reg(&eeprom_dev, 0x00, 0x0F, 0x0A);
    
    while (1) {
        // 主循环
    }
}
