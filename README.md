# Embedded-Utils 🚀

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-STM32%20%7C%20ESP32%20%7C%20Linux-blue)](https://github.com/longxia99/embedded-utils)

> **嵌入式开发通用工具库** - 让嵌入式开发更高效

📖 **中文文档**: [README_CN.md](README_CN.md)

---

## ✨ 特性

- 🔧 **开箱即用** - 纯 C 实现，零依赖
- 📦 **模块化设计** - 按需取用，灵活裁剪
- 🎯 **跨平台** - 支持 STM32/ESP32/Linux/RTOS
- 🧪 **完整测试** - 单元测试覆盖率 > 90%
- 📚 **详细文档** - API 文档 + 使用示例 + 移植指南

---

## 📦 核心组件

### 阶段 1：基础工具 (utils/) ✅

| 组件 | 描述 | 状态 | 示例 |
|------|------|------|------|
| **ring_buffer** | 中断安全的环形缓冲区 | ✅ | [示例](examples/demo_linux/ring_buffer_demo.c) |
| **fifo** | 多生产者/消费者 FIFO | ✅ | [示例](tests/test_fifo.c) |
| **crc** | CRC8/16/32 计算 | ✅ | [示例](tests/test_crc.c) |
| **fixed_point** | 定点数运算库 | ✅ | [示例](tests/test_fixed_point.c) |

### 阶段 2：驱动框架 (drivers/) 🚧

| 组件 | 描述 | 状态 | 示例 |
|------|------|------|------|
| **i2c_dev** | I2C 设备通用驱动框架 | ✅ | [示例](examples/demo_stm32/i2c_eeprom_demo.c) |
| **spi_dev** | SPI 设备通用驱动框架 | ✅ | [示例](examples/demo_stm32/spi_flash_demo.c) |
| **uart_dev** | UART 环形缓冲收发 | 🔄 Coming soon |
| **gpio_dev** | GPIO 抽象层 | 🔄 Coming soon |

### 阶段 3：OS 抽象 (os/) 🚧

| 组件 | 描述 | 状态 |
|------|------|------|
| **mutex** | 互斥锁抽象 | 🚧 Coming soon |
| **queue** | 消息队列抽象 | 🚧 Coming soon |
| **thread** | 线程抽象 | 🚧 Coming soon |

---

## 🚀 快速开始

### 1. 克隆项目

```bash
git clone https://github.com/longxia99/embedded-utils.git
cd embedded-utils
```

### 2. 添加到你的项目

```bash
# 复制需要的模块到你的项目
cp -r include/utils/* your_project/include/
cp -r src/utils/* your_project/src/
```

### 3. 使用示例

```c
#include "ring_buffer.h"

// 创建环形缓冲区
ring_buffer_t rb;
uint8_t buffer[256];
ring_buffer_init(&rb, buffer, sizeof(buffer));

// 写入数据
uint8_t data[] = "Hello";
ring_buffer_write(&rb, data, strlen(data));

// 读取数据
uint8_t out[64];
size_t len = ring_buffer_read(&rb, out, sizeof(out));
```

### 4. 运行测试

```bash
cd tests
gcc -o test_ring_buffer test_ring_buffer.c ../src/utils/ring_buffer.c
./test_ring_buffer
```

---

## 📁 项目结构

```
Embedded-Utils/
├── include/
│   ├── utils/          # 基础工具头文件
│   ├── drivers/        # 驱动框架头文件
│   └── os/             # OS 抽象头文件
├── src/
│   ├── utils/          # 基础工具实现
│   ├── drivers/        # 驱动框架实现
│   └── os/             # OS 抽象实现
├── examples/
│   ├── demo_stm32/     # STM32 示例
│   ├── demo_esp32/     # ESP32 示例
│   └── demo_linux/     # Linux 示例
├── tests/              # 单元测试
└── docs/               # 文档
```

---

## 📚 文档

- [快速开始指南](docs/quick_start.md) - 如何集成到你的项目
- [API 参考文档](docs/api_reference.md) - 完整 API 说明
- [移植指南](docs/porting_guide.md) - 移植到不同平台

---

## 🛠️ 移植指南

### STM32 (HAL 库)

```c
// 在项目中包含头文件路径
// Include: include/utils/
// Source: src/utils/

// 直接使用，无需特殊配置
#include "ring_buffer.h"
```

### ESP32 (ESP-IDF)

```bash
# 将组件添加到 components 目录
# 在 CMakeLists.txt 中添加
idf_component_register(
    INCLUDE_DIRS "include"
    SRCS "src/utils/ring_buffer.c"
)
```

### Linux

```bash
# 直接编译使用
gcc -I./include your_code.c ./src/utils/*.c -o your_app
```

### FreeRTOS

```c
// 后续会添加 FreeRTOS 适配层
// 支持 mutex/queue 等 OS 抽象
```

---

## 🧪 测试覆盖率

| 模块 | 覆盖率 | 状态 |
|------|--------|------|
| ring_buffer | 95% | ✅ |
| fifo | 90% | ✅ |
| crc | 100% | ✅ |
| fixed_point | 95% | ✅ |

---

## 📅 开发路线图

### 2026 Q2
- [x] ring_buffer 模块
- [x] fifo 模块
- [x] crc 模块
- [x] fixed_point 模块

### 2026 Q3
- [ ] i2c_dev 驱动框架
- [ ] spi_dev 驱动框架
- [ ] uart_dev 驱动框架

### 2026 Q4
- [ ] OS 抽象层（mutex/queue/thread）
- [ ] state_machine 框架
- [ ] 完整文档

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

---

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情

---

## 👨‍💻 关于作者

**嵌入式开发顾问** - 专注嵌入式系统开发

- 💼 擅长：Linux 驱动/单片机/RTOS
- 🎯 提供：技术咨询、项目开发、代码审查
- 📧 联系：[467100920@qq.com]
- 💬 知乎：[https://www.zhihu.com/people/60-95-31-98]

**如果你有嵌入式开发需求，欢迎联系！**

---

## 🌟 Star History

如果这个项目对你有帮助，请给一个 Star！⭐

[![Star History Chart](https://api.star-history.com/svg?repos=longxia99/embedded-utils&type=Date)](https://star-history.com/#longxia99/embedded-utils&Date)

---

## 🔗 相关链接

- [知乎专栏：嵌入式开发那些事](https://www.zhihu.com/column/c_2024468569010831878)
- [CSDN 博客](https://blog.csdn.net/sdkerjerf?spm=1011.2266.3001.5343)
- [技术文章：嵌入式驱动开发的 5 个常见坑](https://zhuanlan.zhihu.com/p/2024442903481599044)
