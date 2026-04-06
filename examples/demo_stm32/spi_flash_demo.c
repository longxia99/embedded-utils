/**
 * @file spi_flash_demo.c
 * @brief STM32 平台 SPI Flash 使用示例（W25Q64）
 * 
 * 硬件连接：
 * - SPI1_SCK -> PA5
 * - SPI1_MISO -> PA6
 * - SPI1_MOSI -> PA7
 * - CS -> PB12
 */

#include "stm32f1xx_hal.h"
#include "../../include/drivers/spi_dev.h"

// Flash 设备句柄
static spi_dev_t flash_dev;

// SPI 句柄（由 HAL 提供）
extern SPI_HandleTypeDef hspi1;

// 片选引脚（用 GPIO 引脚号表示）
#define FLASH_CS_PIN    GPIOB, GPIO_PIN_12

/*============================================================================
 * W25Q64 命令定义
 *============================================================================*/

#define CMD_WRITE_ENABLE        0x06
#define CMD_WRITE_DISABLE       0x04
#define CMD_READ_STATUS         0x05
#define CMD_PAGE_PROGRAM        0x02
#define CMD_SEQUENTIAL_PROGRAM  0xAD
#define CMD_BLOCK_ERASE_4K      0x20
#define CMD_BLOCK_ERASE_32K     0x52
#define CMD_BLOCK_ERASE_64K     0xD8
#define CMD_CHIP_ERASE          0xC7
#define CMD_READ_DATA           0x03
#define CMD_FAST_READ           0x0B
#define CMD_POWER_DOWN          0xB9
#define CMD_RELEASE_POWER_DOWN  0xAB
#define CMD_JEDEC_ID            0x9F

/*============================================================================
 * 平台层实现（STM32 HAL）
 *============================================================================*/

/**
 * @brief STM32 SPI 配置
 */
static int stm32_spi_configure(void *handle, uint32_t freq_hz, uint8_t mode, uint8_t data_bits)
{
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)handle;
    
    // 根据频率设置预分频器（需要根据实际时钟计算）
    // 这里简化处理
    
    // 设置 SPI 模式
    switch (mode) {
        case SPI_MODE_0:
            hspi->Init.CLKPolarity = GPIO_LOW;
            hspi->Init.CLKPhase = GPIO_PHASE_1EDGE;
            break;
        case SPI_MODE_1:
            hspi->Init.CLKPolarity = GPIO_LOW;
            hspi->Init.CLKPhase = GPIO_PHASE_2EDGE;
            break;
        case SPI_MODE_2:
            hspi->Init.CLKPolarity = GPIO_HIGH;
            hspi->Init.CLKPhase = GPIO_PHASE_1EDGE;
            break;
        case SPI_MODE_3:
            hspi->Init.CLKPolarity = GPIO_HIGH;
            hspi->Init.CLKPhase = GPIO_PHASE_2EDGE;
            break;
    }
    
    // 设置数据位数
    hspi->Init.DataSize = (data_bits == 16) ? SPI_DATASIZE_16BIT : SPI_DATASIZE_8BIT;
    
    HAL_SPI_Init(hspi);
    
    return SPI_OK;
}

/**
 * @brief STM32 SPI 片选有效
 */
static void stm32_spi_cs_select(void *cs_pin)
{
    // 拉低片选
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

/**
 * @brief STM32 SPI 片选无效
 */
static void stm32_spi_cs_deselect(void *cs_pin)
{
    // 拉高片选
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

/**
 * @brief STM32 SPI 写操作
 */
static int stm32_spi_write(void *handle, const uint8_t *data, size_t len, uint32_t timeout)
{
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)handle;
    
    HAL_StatusTypeDef status = HAL_SPI_Transmit(hspi, (uint8_t *)data, len, timeout);
    
    if (status == HAL_OK) {
        return SPI_OK;
    } else if (status == HAL_TIMEOUT) {
        return SPI_ERR_TIMEOUT;
    }
    return SPI_ERR_BUS;
}

/**
 * @brief STM32 SPI 读操作
 */
static int stm32_spi_read(void *handle, uint8_t *data, size_t len, uint32_t timeout)
{
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)handle;
    
    HAL_StatusTypeDef status = HAL_SPI_Receive(hspi, data, len, timeout);
    
    if (status == HAL_OK) {
        return SPI_OK;
    } else if (status == HAL_TIMEOUT) {
        return SPI_ERR_TIMEOUT;
    }
    return SPI_ERR_BUS;
}

/**
 * @brief STM32 SPI 全双工传输
 */
static int stm32_spi_transfer(void *handle, const uint8_t *write_data, uint8_t *read_data, 
                              size_t len, uint32_t timeout)
{
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)handle;
    
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, 
                                                        (uint8_t *)write_data, 
                                                        read_data, 
                                                        len, 
                                                        timeout);
    
    if (status == HAL_OK) {
        return SPI_OK;
    } else if (status == HAL_TIMEOUT) {
        return SPI_ERR_TIMEOUT;
    }
    return SPI_ERR_BUS;
}

/**
 * @brief SPI 平台操作接口
 */
static const spi_platform_ops_t stm32_spi_ops = {
    .configure = stm32_spi_configure,
    .cs_select = stm32_spi_cs_select,
    .cs_deselect = stm32_spi_cs_deselect,
    .write = stm32_spi_write,
    .read = stm32_spi_read,
    .transfer = stm32_spi_transfer
};

/*============================================================================
 * Flash 驱动层
 *============================================================================*/

/**
 * @brief 初始化 Flash
 */
int flash_init(void)
{
    // 注册平台接口
    spi_dev_register_platform(&stm32_spi_ops);
    
    // 初始化设备（40MHz, MODE_0）
    return spi_dev_init(&flash_dev, &hspi1, (void *)GPIO_PIN_12, 40000000, SPI_MODE_0, 100);
}

/**
 * @brief 读取 Flash ID
 */
int flash_read_id(uint8_t *id, size_t id_len)
{
    return spi_dev_read_id(&flash_dev, CMD_JEDEC_ID, id, id_len);
}

/**
 * @brief 读取状态寄存器
 */
int flash_read_status(uint8_t *status)
{
    return spi_dev_read_status(&flash_dev, CMD_READ_STATUS, status);
}

/**
 * @brief 等待 Flash 就绪
 */
int flash_wait_ready(void)
{
    return spi_dev_wait_ready(&flash_dev, CMD_READ_STATUS, 0x01, 0x00, 1000);
}

/**
 * @brief 写使能
 */
int flash_write_enable(void)
{
    return spi_dev_write(&flash_dev, (const uint8_t[]){CMD_WRITE_ENABLE}, 1);
}

/**
 * @brief 页写入（256 字节）
 */
int flash_page_write(uint32_t addr, const uint8_t *data, size_t len)
{
    if (len > 256) {
        len = 256;  // 页大小限制
    }
    
    // 等待就绪
    int ret = flash_wait_ready();
    if (ret != SPI_OK) {
        return ret;
    }
    
    // 写使能
    ret = flash_write_enable();
    if (ret != SPI_OK) {
        return ret;
    }
    
    // 发送地址（24 位）
    uint8_t addr_buf[3];
    addr_buf[0] = (addr >> 16) & 0xFF;
    addr_buf[1] = (addr >> 8) & 0xFF;
    addr_buf[2] = addr & 0xFF;
    
    // 页编程命令
    uint8_t cmd = CMD_PAGE_PROGRAM;
    ret = spi_dev_cmd(&flash_dev, cmd, addr_buf, 3, data, len);
    
    // 等待写入完成
    flash_wait_ready();
    
    return ret;
}

/**
 * @brief 读取数据
 */
int flash_read_data(uint32_t addr, uint8_t *data, size_t len)
{
    // 等待就绪
    int ret = flash_wait_ready();
    if (ret != SPI_OK) {
        return ret;
    }
    
    // 发送地址（24 位）
    uint8_t addr_buf[3];
    addr_buf[0] = (addr >> 16) & 0xFF;
    addr_buf[1] = (addr >> 8) & 0xFF;
    addr_buf[2] = addr & 0xFF;
    
    // 读数据命令
    uint8_t cmd = CMD_READ_DATA;
    return spi_dev_cmd(&flash_dev, cmd, addr_buf, 3, data, len);
}

/**
 * @brief 擦除 4K 块
 */
int flash_erase_4k(uint32_t addr)
{
    // 等待就绪
    int ret = flash_wait_ready();
    if (ret != SPI_OK) {
        return ret;
    }
    
    // 写使能
    ret = flash_write_enable();
    if (ret != SPI_OK) {
        return ret;
    }
    
    // 发送地址（24 位）
    uint8_t addr_buf[3];
    addr_buf[0] = (addr >> 16) & 0xFF;
    addr_buf[1] = (addr >> 8) & 0xFF;
    addr_buf[2] = addr & 0xFF;
    
    // 4K 块擦除命令
    ret = spi_dev_write(&flash_dev, &cmd, 1);
    if (ret != SPI_OK) {
        return ret;
    }
    ret = spi_dev_write(&flash_dev, addr_buf, 3);
    
    // 等待擦除完成
    flash_wait_ready();
    
    return ret;
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
    MX_SPI1_Init();
    
    // 初始化 Flash
    if (flash_init() != SPI_OK) {
        // 错误处理
        while (1);
    }
    
    // 读取 Flash ID
    uint8_t id[3];
    if (flash_read_id(id, sizeof(id)) == SPI_OK) {
        // W25Q64 的 ID 应该是：0xEF, 0x40, 0x17
        // 验证 ID...
    }
    
    // 示例 1：页写入
    uint8_t write_data[] = "Hello, SPI Flash!";
    flash_page_write(0x000000, write_data, sizeof(write_data));
    
    // 示例 2：读取数据
    uint8_t read_data[32];
    flash_read_data(0x000000, read_data, sizeof(write_data));
    // read_data 应该等于 "Hello, SPI Flash!"
    
    // 示例 3：擦除 4K 块
    flash_erase_4k(0x000000);
    
    while (1) {
        // 主循环
    }
}
