# VSenv - VS Code 离线实例管理器

![License](https://img.shields.io/badge/License-AGPLv3.0-blue.svg)
![Version](https://img.shields.io/badge/Version-1.6.2-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![Status](https://img.shields.io/badge/Status-Only--fix-red.svg)

![Image](./images/vsenv.png)

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=dhjs0000/vsenv&type=Date)](https://www.star-history.com/#dhjs0000/vsenv&Date)

[English](./README_EN.md) | [中文](./README.md) | [日本語](./README_JP_F.md) |]

[自动化文档](./AutomationDocs.md)]

## 📖 目录
- [项目简介](#项目简介)
- [核心特性](#核心特性)
- [系统要求](#系统要求)
- [安装指南](#安装指南)
- [快速开始](#快速开始)
- [详细使用说明](#详细使用说明)
- [技术架构](#技术架构)
- [隐私与安全](#隐私与安全)
- [故障排除](#故障排除)
- [贡献指南](#贡献指南)
- [许可证](#许可证)
- [更新日志](#更新日志)

> ⚠️ **项目状态声明**: 该项目已进入 **Only-fix** 状态，仅接受bug修复，不再开发新功能。感谢所有用户的支持与贡献！

## 🚀 项目简介

VSenv 是一个专业的 VS Code 离线实例管理器，允许用户创建、管理和运行完全独立的 VS Code 实例。每个实例拥有独立的用户数据、扩展目录和配置环境，特别适合以下场景：

- 🔒 **隐私保护**：防止扩展和配置数据交叉污染
- 🏢 **企业环境**：为不同项目创建隔离的开发环境
- 🔬 **测试验证**：测试扩展和配置而不影响主环境
- 🌐 **网络隔离**：支持代理和网络隔离配置
- 📦 **离线部署**：完美支持离线环境下的扩展管理

## ✨ 核心特性

### 🛡️ 多层级隔离
- **文件系统隔离**：每个实例独立的 data 和 extensions 目录
- **注册表隔离**：独立的协议注册和硬件指纹配置
- **网络隔离**：支持代理配置和虚拟网卡模拟
- **进程隔离**：可选沙箱模式运行

### 🔧 高级功能
- **硬件指纹伪装**：随机化 CPUID、磁盘序列号和 MAC 地址
- **协议重定向**：自定义 vscode:// 协议处理
- **批量扩展管理**：支持扩展列表导入导出
- **实例迁移**：完整的实例导出导入功能
- **多语言支持**：中文和英文界面

## 💻 系统要求

### 最低要求
- **操作系统**: Windows 10 或更高版本
- **架构**: x64 (64位)
- **内存**: 4GB RAM
- **存储**: 至少 2GB 可用空间

### 推荐配置
- **操作系统**: Windows 11 22H2 或更高
- **内存**: 8GB RAM 或更多
- **存储**: SSD 硬盘，10GB 可用空间
- **权限**: 管理员权限（用于高级网络功能）

## 📥 安装指南

### 方法一：预编译版本（推荐）
1. 访问 [Releases 页面](https://github.com/dhjs0000/vsenv/releases)
2. 下载最新版本的 `vsenv-x.x.x-windows-amd64.zip`
3. 解压到任意目录（如 `C:\Tools\vsenv`）
4. 将目录添加到系统 PATH 环境变量

### 方法二：从源代码构建
```bash
# 克隆仓库
git clone https://github.com/dhjs0000/vsenv.git
cd vsenv

# 安装依赖
vcpkg install nlohmann-json

# 使用 Visual Studio 2019+ 编译
msbuild vsenv.sln /p:Configuration=Release
```

### 验证安装
打开命令提示符，运行：
```cmd
vsenv --version
```
应显示：`VSenv 1.6.2 by dhjs0000 (AGPLv3.0)`

## 🚀 快速开始

### 交互模式（推荐）
```cmd
# 启动交互模式选择实例
vsenv f
```

### 创建你的第一个实例
```cmd
# 创建名为 "myproject" 的实例
vsenv create myproject

# 将 VS Code 离线包解压到指定目录
# 默认路径: C:\Users\<用户名>\.vsenv\myproject\vscode
```

### 启动实例
```cmd
# 基本启动
vsenv start myproject

# 带隐私保护的启动
vsenv start myproject --fake-hw --sandbox
```

### 安装扩展
```cmd
# 安装中文语言包
vsenv extension myproject MS-CEINTL.vscode-language-pack-zh-hans

# 列出已安装扩展
vsenv extension list myproject
```

## 📚 详细使用说明

### 交互模式（推荐）
```cmd
# 启动交互模式
vsenv f

# 在交互模式中：
# ↑↓ 键选择实例
# 回车键启动选中实例
# : 键添加启动参数
# Esc 键退出
```

### 实例管理命令

#### 创建实例
```cmd
# 在默认位置创建实例
vsenv create project-alpha

# 在自定义位置创建实例
vsenv create project-beta "D:\Development\vscode-instances\beta"

# 创建实例并设置中文界面
vsenv create project-gamma --lang cn
```

#### 启动实例
```cmd
# 基本启动
vsenv start project-alpha

# 网络隔离模式
vsenv start project-alpha --host --mac --proxy http://127.0.0.1:8080

# 沙箱模式（增强安全）
vsenv start project-alpha --sandbox --fake-hw

# 中文界面启动
vsenv start project-alpha --lang cn
```

**启动选项说明：**
- `--host`: 随机化主机名（需要管理员权限）
- `--mac`: 创建虚拟网卡并随机化 MAC 地址
- `--proxy`: 设置 HTTP 代理服务器
- `--sandbox`: 在受限的 Logon-Session 中运行
- `--fake-hw`: 随机化硬件指纹

#### 停止和删除实例
```cmd
# 停止运行中的实例
vsenv stop project-alpha

# 完全删除实例（不可恢复）
vsenv remove project-alpha
```

### VSenv插件系统

#### 插件管理
```cmd
# 安装本地插件
vsenv plugin install -i <插件路径>

# 卸载插件
vsenv plugin remove <插件名>

# 列出已安装插件
vsenv plugin list

# 插件包管理器
vsenv pm
```

#### 扩展管理

#### 单个扩展操作
```cmd
# 安装扩展
vsenv extension project-alpha ms-python.python

# 安装中文语言包
vsenv extension project-alpha MS-CEINTL.vscode-language-pack-zh-hans
```

#### 批量扩展管理
```cmd
# 从文件批量安装扩展
vsenv extension import project-alpha extensions.txt

# 导出已安装扩展列表
vsenv extension list project-alpha > installed_extensions.txt
```

**扩展列表文件格式（extensions.txt）：**
```txt
# 注释以 # 开头
ms-python.python
ms-vscode.vscode-typescript-next
bradlc.vscode-tailwindcss
# 更多扩展...
```

### 协议管理

#### 重定向 vscode:// 协议
```cmd
# 将 vscode:// 链接重定向到指定实例
vsenv regist project-alpha

# 守护模式（防止其他程序修改协议注册）
vsenv regist-guard project-alpha

# 恢复默认的 vscode:// 处理
vsenv logoff

# 手动重建协议关联
vsenv rest "C:\Users\Username\AppData\Local\Programs\Microsoft VS Code\Code.exe"
```

### 实例迁移

#### 导出实例
```cmd
# 导出实例到压缩包
vsenv export project-alpha "D:\Backups\project-alpha.vsenv"

# 导出包含所有配置和扩展
```

#### 导入实例
```cmd
# 导入实例（保持原名称）
vsenv import "D:\Backups\project-alpha.vsenv"

# 导入并重命名实例
vsenv import "D:\Backups\project-alpha.vsenv" project-new
```

### 查看和管理
```cmd
# 列出所有实例
vsenv list

# 输出示例：
# Instance Name    Path
# -----------------------------------------
# project-alpha    C:\Users\user\.vsenv\project-alpha
# project-beta     D:\Development\vscode-instances\beta
```

## 🏗️ 技术架构

### 文件结构
```
.vsenv/
├── instance1/
│   ├── vscode/          # VS Code 程序文件
│   ├── data/            # 用户数据（配置、状态等）
│   ├── extensions/      # 扩展目录
│   └── logs/            # 日志文件
├── instance2/
│   └── ...             # 另一个实例
└── otherPath.json      # 自定义路径映射
```

### 注册表结构
```
HKEY_CURRENT_USER
└── Software
    ├── Classes
    │   ├── vsenv-{instancename}      # 自定义协议
    │   └── vscode                    # 协议重定向
    └── VSenv
        └── {instancename}
            └── Hardware              # 硬件指纹配置
```

### 隔离机制
1. **文件系统级隔离**: 每个实例独立的目录结构
2. **注册表级隔离**: 独立的协议注册和配置
3. **进程级隔离**: 可选沙箱执行环境
4. **网络级隔离**: 代理配置和虚拟网络设备

## 🔒 隐私与安全

### 硬件指纹保护
VSenv 可以随机化以下硬件标识符：
- **CPUID**: 处理器标识
- **Disk Serial**: 磁盘序列号  
- **MAC Address**: 网络适配器地址

### 网络隐私
- 支持 HTTP/HTTPS 代理
- 虚拟网卡 MAC 地址随机化
- 主机名随机化（管理员权限）

### 数据隔离
- 实例间完全数据隔离
- 独立的扩展生态系统
- 独立的用户配置和状态

## 🐛 故障排除

### 常见问题

### 调试和诊断工具

#### 医生模式（Doctor）
```cmd
# 运行系统自检
vsenv doctor

# 检查项目包括：
# - 主目录存在性
# - 插件目录状态
# - 注册表写入权限
# - PowerShell可用性
# - 插件加载状态
# - 实例扫描结果
```

#### 调试模式
```cmd
# 启用调试模式
vsenv -debug <子命令>

# 可用子命令：
# reg     - 打印vscode://协议当前值
# loaded  - 打印已加载插件列表
# env     - 打印环境变量和路径
# crash   - 故意触发崩溃（测试用）
# exception - 故意抛出异常（测试用）
# trace   - 打印最后一次Windows错误
# dll <插件名> - 打印DLL导出函数
# token   - 打印当前进程Token信息
# proc <实例名> - 打印实例进程信息
```

#### 2. 权限不足
**问题**: 网络相关功能需要管理员权限
**解决**: 以管理员身份运行命令提示符

#### 3. 协议重定向失败
**问题**: vscode:// 链接没有正确跳转
**解决**: 
```cmd
vsenv logoff        # 先恢复默认
vsenv regist <实例名> # 重新注册
```

#### 4. 扩展安装失败
**问题**: 扩展安装过程中出现错误
**解决**:
- 检查网络连接
- 验证扩展 ID 是否正确
- 尝试使用代理：`--proxy <代理地址>`

### 日志文件
实例日志位于：`实例目录\logs\vsenv.log`

### 调试模式
设置环境变量开启详细日志：
```cmd
set VSENV_DEBUG=1
vsenv start project-alpha
```

## 🤝 贡献指南

> ⚠️ **重要提醒**: 由于项目已进入 **Only-fix** 状态，目前**仅接受bug修复相关的贡献**。新功能开发将不再被合并。

对于bug修复的贡献，请阅读以下指南：

### 开发环境设置
1. Fork 本仓库
2. 克隆你的 fork:
   ```bash
   git clone https://github.com/your-username/vsenv.git
   ```
3. 创建功能分支:
   ```bash
   git checkout -b feature/amazing-feature
   ```
4. 提交更改:
   ```bash
   git commit -m 'Add some amazing feature'
   ```
5. 推送到分支:
   ```bash
   git push origin feature/amazing-feature
   ```
6. 创建 Pull Request

### 代码规范
- 遵循现有的代码风格
- 添加适当的注释
- 更新相关文档
- 测试你的更改

### 报告问题
请在 GitHub Issues 中报告 bug，包括：
- 详细的问题描述
- 复现步骤
- 系统环境信息
- 错误日志（如果有）

> ⚠️ **注意**: 目前**仅接受bug报告**，新功能请求将不会被处理。

## 📄 许可证

本项目采用 **AGPLv3.0** 许可证开源。

```
VSenv - VS Code 离线实例管理器
Copyright (C) 2025 dhjs0000

本程序是自由软件：你可以根据自由软件基金会发布的GNU Affero通用公共许可证
的条款，即许可证的第3版或（您选择的）任何后来的版本重新发布它和/或修改它。

本程序是希望它有用，但没有任何担保；没有甚至没有适销性或特定用途适用性的隐含担保。
更多细节请参见GNU Affero通用公共许可证。

你应该已经收到了一份GNU Affero通用公共许可证的副本
如果没有，请参阅<https://www.gnu.org/licenses/>。
```
---

## 🔗 相关链接

- 📖 **详细文档**: [https://dhjs0000.github.io/VSenv/](https://dhjs0000.github.io/VSenv/)
- 🐛 **问题反馈**: [GitHub Issues](https://github.com/dhjs0000/vsenv/issues)
- 💬 **讨论区**: [GitHub Discussions](https://github.com/dhjs0000/vsenv/discussions)
- ⭐ **给个星星**: 如果这个项目对你有帮助，请给个 ⭐

---

**温馨提示**: 本软件完全免费开源，如果你付费购买了这个软件，那么你被骗了！请通过官方渠道获取。

---
*最后更新: 2025年10月*  
*维护者: [dhjs0000](https://github.com/dhjs0000)*