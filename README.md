# VSenv - VS Code 离线实例管理器

![Windows Platform](https://img.shields.io/badge/Platform-Windows-blue)
![License AGPL-3.0](https://img.shields.io/badge/License-AGPL--3.0-green)
[![GitHub Repo](https://img.shields.io/badge/GitHub-Repo-brightgreen)](https://github.com/dhjs0000/vsenv)

VSenv 是一个轻量级的命令行工具，用于创建和管理完全独立的 VS Code 实例。每个实例拥有自己独立的数据目录、扩展目录和用户设置，实现真正的环境隔离。

## 主要特性

- 🛠️ **创建独立实例**：为每个项目创建隔离的 VS Code 环境
- 📁 **数据隔离**：独立的数据目录和扩展目录
- 🚀 **一键启动**：快速启动特定实例
- ⏹️ **实例停止**：终止指定实例的所有进程
- 🗑️ **完全移除**：彻底删除实例及其所有数据
- 🌐 **多语言支持**：支持中文和英文界面
- 🎯 **轻量高效**：纯原生 Win32 API 实现，无额外依赖

## 使用场景

- 同时开发多个需要不同环境配置的项目
- 测试扩展或主题而不影响主配置
- 临时使用特定配置的 VS Code
- 保持开发环境的整洁与隔离

## 安装与使用

### 编译说明（使用 Visual Studio）
1. 克隆仓库：`git clone https://github.com/dhjs0000/vsenv.git`
2. 使用 Visual Studio 打开解决方案
3. 编译项目（Release x64 推荐）
4. 将生成的 `vsenv.exe` 添加到系统 PATH

### 基本使用
```bash
# 创建新实例
vsenv create my_project

# 启动实例
vsenv start my_project

# 停止实例
vsenv stop my_project

# 完全移除实例
vsenv remove my_project
```

### 使用中文界面
```bash
vsenv create my_project --lang cn
```

## 实例结构
每个实例包含以下目录：
```
~\.vsenv\my_project\
  ├── data\          # 用户数据目录
  ├── extensions\    # 扩展目录
  └── vscode\        # VS Code 离线包解压位置
```

## 首次使用指南
1. 创建实例：`vsenv create my_project`
2. 下载 VS Code 离线包（ZIP格式）
3. 将离线包解压到 `%USERPROFILE%\.vsenv\my_project\vscode\`
4. 启动实例：`vsenv start my_project`

## 许可证
本项目采用 GNU Affero General Public License v3.0 许可证开源。详细信息请查看 [LICENSE](LICENSE) 文件。

## 贡献
欢迎提交 Issue 和 Pull Request！请确保：
- 代码符合现有风格
- 包含必要的测试
- 更新相关文档

## 常见问题
**Q：为什么启动时报"未找到 Code.exe"？**  
A：请确保已将 VS Code 离线包解压到实例的 vscode 目录下。

**Q：如何完全重置实例？**  
A：使用 `vsenv remove 实例名` 后重新创建。

**Q：是否支持共享扩展？**  
A：每个实例完全独立，但可通过手动创建符号链接实现共享。

---

**追随马斯克的步伐，坚持免费开源**  
为开发者提供简单高效的工具，让环境管理不再烦恼！