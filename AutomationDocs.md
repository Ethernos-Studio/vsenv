# 📄 VSenv 1.6.2 CI/CD 命令文档（带 `--quiet`）

> 所有命令均假设在 Windows 环境下执行，且 `vsenv.exe` 位于当前路径或系统 PATH 中。  
> 所有命令结尾均附加 `--quiet`，以确保输出简洁、适合自动化脚本使用。

---

## 🔧 基础命令

### 1. 显示版本
```bash
vsenv --version --quiet
```
**输出格式（标准错误）**：
```
VSenv 1.6.2
```
> 注：`--quiet` 下仍输出到 `stderr`，无换行。

---

### 2. 列出所有实例
```bash
vsenv list --quiet
```
**输出格式（标准输出）**：
```
Instance Name	Path
-----------------------------------------
<name1>	<full_path_1>
<name2>	<full_path_2> (无效实例)
...
```
- 若实例路径下无 `vscode\Code.exe`，标记为 `(无效实例)`。
- 路径来自默认目录（`%USERPROFILE%\.vsenv`）或 `otherPath.json`。

---

### 3. 创建实例
```bash
vsenv create <实例名> [<自定义路径>] --quiet
```
**示例**：
```bash
vsenv create dev --quiet
vsenv create prod "D:\vsenv\prod" --quiet
```
**输出格式（标准输出）**：
```
Instance '<实例名>' created at <路径>
Please extract the offline package to <路径>\vscode\
```

---

### 4. 启动实例
```bash
vsenv start <实例名> [--host] [--mac] [--proxy <url>] [--sandbox] [--fake-hw] --quiet
```
**示例**：
```bash
vsenv start dev --quiet
vsenv start dev --sandbox --fake-hw --quiet
vsenv start dev --proxy http://127.0.0.1:8080 --quiet
```
**输出格式（标准输出）**：
```
Started <实例名>
```
> 若 `Code.exe` 不存在，输出（标准错误）：
```
Code.exe not found, please check the path.
```

---

### 5. 停止实例（过时，仅杀进程）
```bash
vsenv stop <实例名> --quiet
```
**输出格式（标准输出）**：
```
Stopped <实例名>
```

---

### 6. 删除实例
```bash
vsenv remove <实例名> --quiet
```
**输出格式（标准输出）**：
```
Removed <实例名>
```
> 同时清理注册表协议、硬件指纹、`otherPath.json` 记录。

---

## 🔗 协议管理

### 7. 注册 vscode:// 到实例
```bash
vsenv regist <实例名> --quiet
```
**输出格式（标准输出）**：
```
vscode:// now redirects to instance '<实例名>'
```

---

### 8. 守护协议（需管理员，持续运行）
```bash
vsenv regist-guard <实例名> --quiet
```
**输出格式（标准输出）**：
```
开始守护 vscode:// 协议，按 Ctrl+C 停止...
[INFO] 检测到协议被修改，正在修复...
[OK] 已修复
```
> 此命令为**阻塞式守护进程**，不适合 CI/CD 自动化，仅用于测试。

---

### 9. 恢复默认 vscode:// 协议
```bash
vsenv logoff --quiet
```
**输出格式（标准输出）**：
```
vscode:// restored to original VS Code
```

---

### 10. 手动重建 vscode:// 协议
```bash
vsenv rest "<Code.exe完整路径>" --quiet
```
**示例**：
```bash
vsenv rest "C:\Users\user\AppData\Local\Programs\Microsoft VS Code\Code.exe" --quiet
```
**输出格式（成功）**：
```
✅ vscode:// 协议已成功恢复至: <路径>
```
**失败（标准错误）**：
```
恢复失败，请检查路径和权限
```

---

## 🧩 扩展管理

### 11. 安装单个扩展
```bash
vsenv extension <实例名> <扩展ID> --quiet
```
**示例**：
```bash
vsenv extension dev ms-python.python --quiet
```
**输出格式（成功）**：
```
Extension '<扩展ID>' installed.
```
**失败（标准错误）**：
```
VS Code CLI not found, cannot install extension.
```

---

### 12. 批量安装扩展
```bash
vsenv extension <实例名> import <列表文件> --quiet
```
**列表文件格式**（每行一个扩展 ID，支持 `#` 注释）：
```txt
ms-python.python
ms-vscode.cpptools
# Chinese lang pack
MS-CEINTL.vscode-language-pack-zh-hans
```
**输出**：每成功安装一个扩展，输出一行：
```
Extension '<ID>' installed.
```

---

### 13. 列出已安装扩展
```bash
vsenv extension <实例名> list --quiet
```
**输出格式**：直接输出 `code --list-extensions` 的结果（标准输出）：
```
ms-python.python
ms-vscode.cpptools
...
```

---

## 📦 导入/导出

### 14. 导出实例（生成 `.zip` 包）
```bash
vsenv export <实例名> <导出路径.vsenv> --quiet
```
**示例**：
```bash
vsenv export dev D:\backup\dev.vsenv --quiet
```
**输出格式（标准输出）**：
```
实例已成功导出到: <导出路径.vsenv>
文件大小: <整数> MB
```

---

### 15. 导入实例
```bash
vsenv import <.vsenv文件> [<新实例名>] --quiet
```
**示例**：
```bash
vsenv import D:\backup\dev.vsenv dev2 --quiet
```
**输出格式（标准输出）**：
```
实例已成功从 '<文件>' 导入到 '<实例名>'
位置: <目标路径>
```
> 若含硬件指纹，额外输出：
```
Hardware fingerprints randomized: CPUID=..., DiskSN=..., MAC=...
```

---

## 🧪 调试与诊断

### 16. 自检工具
```bash
vsenv doctor --quiet
```
**输出格式（标准输出）**：
```
VSenv Doctor —— 一键自检工具
[+] 主目录存在: ...
[+] 插件目录存在: ...
[+] 注册表可写
[+] PowerShell 可用
[*]  当前未加载任何插件
[*]  未找到任何实例
Doctor 完成：发现 <N> 个问题。
你的 VSenv 非常健康，放心食用！
```
> 若有问题，对应项为 `[-]`，末尾提示修复建议。

---

### 17. 调试子命令（开发用）
```bash
vsenv -debug <子命令> --quiet
```
常用子命令：
- `reg`：打印当前 `vscode://` 注册表值
- `loaded`：列出已加载插件
- `env`：打印环境路径
- `proc <实例名>`：打印进程信息

> 输出为纯文本，适合日志采集。

---

## 🧩 插件管理

### 18. 列出已安装插件
```bash
vsenv plugin list --quiet
```
**输出格式**：
```
VSenv Plugin System 1.6.2
已安装插件：
  - <插件名>  v<版本>  (<作者>)
```

---

### 19. 安装本地插件
```bash
vsenv plugin install -i <插件.zip或目录> --quiet
```
> 安装过程含交互确认（**不适合 CI/CD**）。  
> 若需完全静默，需修改源码跳过 `getline`。

---

### 20. 卸载插件
```bash
vsenv plugin remove <插件名> --quiet
```
> 同样含交互确认（**不适合 CI/CD**）。

---

## ⚠️ 注意事项（CI/CD 场景）

1. **`--quiet` 仅抑制彩蛋和 Banner，不影响错误/关键信息输出**。
2. **`regist-guard`、`plugin install/remove` 含交互，不适合自动化**。
3. 所有路径建议使用**绝对路径**，避免相对路径歧义。
4. 若需完全无交互 CI/CD，建议：
   - 预先准备好实例目录结构；
   - 使用 `export`/`import` 实现实例复用；
   - 避免使用 `--host`/`--mac` 等需管理员权限的功能。