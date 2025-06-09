# CakemuOoO
CakeMu-OoO，乱序处理器模拟器。

## 概述

CakemuOoO 是一个基于 SystemC 的超标量乱序执行 RISC-V 处理器模拟器。它实现了具有以下特性的现代处理器流水线：

- 取指、译码、执行和回写阶段
- 用于 ALU、内存和分支操作的预约站
- 用于按程序顺序提交指令的重排序缓冲区
- 寄存器重命名以消除假依赖
- 具有指令和数据访问的内存系统
- 先进的分支预测策略
- 详细的性能分析工具
- 全面的测试套件

## 系统要求

- 支持 C++17 的 C++ 编译器
- SystemC 库（版本 2.3.3 或更新）
- CMake（版本 3.10 或更新）
- RISC-V 工具链（用于测试程序编译）

## 安装

### 安装 SystemC

### 安装 RISC-V 工具链

### 构建 CakemuOoO

要构建模拟器：

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## 使用方法

运行模拟器：

```bash
./cakemu_ooo -f program.bin -t 1000
```

使用 RISC-V 二进制文件运行模拟器：

```bash
./build/cakemu_ooo -f path/to/program.bin -t 10000
```

### 命令行选项

- `-f <file>`: 程序二进制文件（默认：program.bin）
- `-t <time>`: 模拟时间（纳秒，默认：1000）
- `-p <type>`: 分支预测器类型（默认：two_bit）
  - 支持的类型：always_not_taken, always_taken, static_btfn, one_bit, two_bit, gshare, tournament
- `-r`: 生成详细性能报告
- `-o <file>`: 性能报告输出文件（默认：performance_report.txt）
- `-c <file>`: 导出性能数据到 CSV（默认：performance_data.csv）

### 运行测试套件

```bash
# 使测试脚本可执行
chmod +x run_tests.sh

# 运行所有测试
./run_tests.sh
```

## 二进制格式

模拟器需要包含 RISC-V 指令的原始二进制文件。您可以使用 RISC-V 编译器和 objcopy 创建此文件：

```bash
riscv64-unknown-elf-gcc -march=rv64i -mabi=lp64 -static -nostdlib -o program program.c
riscv64-unknown-elf-objcopy -O binary program program.bin
```

## 处理器架构

模拟器实现了一个超标量乱序处理器，具有以下组件：

- **取指单元**：从内存中获取指令
- **译码单元**：解码指令并提取操作数
- **执行单元**：包含预约站和算术逻辑单元
- **重排序缓冲区**：确保按程序顺序提交指令
- **寄存器文件**：包含架构寄存器
- **内存系统**：提供指令和数据访问

处理器使用 Tomasulo 算法和寄存器重命名来处理数据依赖并实现乱序执行。

## 许可证

本项目是开源的，使用 [MIT 许可证](LICENSE)。
