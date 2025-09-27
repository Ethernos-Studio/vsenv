# VSenv - VS Code Offline Instance Manager

![License](https://img.shields.io/badge/License-AGPLv3.0-blue.svg)
![Version](https://img.shields.io/badge/Version-1.0.0-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)

## 📖 Table of Contents
- [Project Introduction](#project-introduction)
- [Core Features](#core-features)
- [System Requirements](#system-requirements)
- [Installation Guide](#installation-guide)
- [Quick Start](#quick-start)
- [Detailed Usage](#detailed-usage)
- [Technical Architecture](#technical-architecture)
- [Privacy & Security](#privacy--security)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)
- [Changelog](#changelog)

## 🚀 Project Introduction

VSenv is a professional VS Code offline instance manager that allows users to create, manage, and run completely independent VS Code instances. Each instance has separate user data, extension directories, and configuration environments, making it ideal for:

- 🔒 **Privacy Protection**: Prevent extension and configuration data cross-contamination
- 🏢 **Enterprise Environments**: Create isolated development environments for different projects
- 🔬 **Testing & Validation**: Test extensions and configurations without affecting main environment
- 🌐 **Network Isolation**: Support for proxy and network isolation configurations
- 📦 **Offline Deployment**: Perfect support for extension management in offline environments

## ✨ Core Features

### 🛡️ Multi-Level Isolation
- **Filesystem Isolation**: Separate data and extensions directories for each instance
- **Registry Isolation**: Independent protocol registration and hardware fingerprint configuration
- **Network Isolation**: Proxy configuration and virtual network adapter simulation
- **Process Isolation**: Optional sandbox mode execution

### 🔧 Advanced Features
- **Hardware Fingerprint Spoofing**: Randomize CPUID, disk serial numbers, and MAC addresses
- **Protocol Redirection**: Custom vscode:// protocol handling
- **Batch Extension Management**: Support for extension list import/export
- **Instance Migration**: Complete instance export/import functionality
- **Multi-language Support**: Chinese and English interfaces

## 💻 System Requirements

### Minimum Requirements
- **OS**: Windows 10 or higher
- **Architecture**: x64 (64-bit)
- **Memory**: 4GB RAM
- **Storage**: At least 2GB free space

### Recommended Configuration
- **OS**: Windows 11 22H2 or higher
- **Memory**: 8GB RAM or more
- **Storage**: SSD with 10GB free space
- **Permissions**: Administrator rights (for advanced network features)

## 📥 Installation Guide

### Method 1: Pre-compiled Version (Recommended)
1. Visit [Releases page](https://github.com/dhjs0000/vsenv/releases)
2. Download latest `vsenv-x.x.x-windows-amd64.zip`
3. Extract to any directory (e.g., `C:\Tools\vsenv`)
4. Add directory to system PATH environment variable

### Method 2: Build from Source
```bash
# Clone repository
git clone https://github.com/dhjs0000/vsenv.git
cd vsenv

# Install dependencies
vcpkg install nlohmann-json

# Compile with Visual Studio 2019+
msbuild vsenv.sln /p:Configuration=Release
```

### Verify Installation
Open Command Prompt and run:
```cmd
vsenv --version
```
Should display: `VSenv 1.0.0 by dhjs0000 (AGPLv3.0)`

## 🚀 Quick Start

### Create Your First Instance
```cmd
# Create instance named "myproject"
vsenv create myproject

# Extract VS Code offline package to specified directory
# Default path: C:\Users\<Username>\.vsenv\myproject\vscode\
```

### Start Instance
```cmd
# Basic startup
vsenv start myproject

# Start with privacy protection
vsenv start myproject --fake-hw --sandbox
```

### Install Extensions
```cmd
# Install Chinese language pack
vsenv extension myproject MS-CEINTL.vscode-language-pack-zh-hans

# List installed extensions
vsenv extension list myproject
```

## 📚 Detailed Usage

### Instance Management Commands

#### Create Instance
```cmd
# Create instance in default location
vsenv create project-alpha

# Create instance in custom location
vsenv create project-beta "D:\Development\vscode-instances\beta"

# Create instance with Chinese interface
vsenv create project-gamma --lang cn
```

#### Start Instance
```cmd
# Basic startup
vsenv start project-alpha

# Network isolation mode
vsenv start project-alpha --host --mac --proxy http://127.0.0.1:8080

# Sandbox mode (enhanced security)
vsenv start project-alpha --sandbox --fake-hw

# Start with Chinese interface
vsenv start project-alpha --lang cn
```

**Start Options:**
- `--host`: Randomize hostname (requires admin rights)
- `--mac`: Create virtual network adapter with random MAC address
- `--proxy`: Set HTTP proxy server
- `--sandbox`: Run in restricted Logon-Session
- `--fake-hw`: Randomize hardware fingerprints

#### Stop and Remove Instances
```cmd
# Stop running instance
vsenv stop project-alpha

# Completely remove instance (irreversible)
vsenv remove project-alpha
```

### Extension Management

#### Single Extension Operations
```cmd
# Install extension
vsenv extension project-alpha ms-python.python

# Install Chinese language pack
vsenv extension project-alpha MS-CEINTL.vscode-language-pack-zh-hans
```

#### Batch Extension Management
```cmd
# Install extensions from file
vsenv extension import project-alpha extensions.txt

# Export installed extensions list
vsenv extension list project-alpha > installed_extensions.txt
```

**Extension List File Format (extensions.txt):**
```txt
# Comments start with #
ms-python.python
ms-vscode.vscode-typescript-next
bradlc.vscode-tailwindcss
# More extensions...
```

### Protocol Management

#### Redirect vscode:// Protocol
```cmd
# Redirect vscode:// links to specified instance
vsenv regist project-alpha

# Guard mode (prevent other programs from modifying protocol registration)
vsenv regist-guard project-alpha

# Restore default vscode:// handler
vsenv logoff

# Manually rebuild protocol association
vsenv rest "C:\Users\Username\AppData\Local\Programs\Microsoft VS Code\Code.exe"
```

### Instance Migration

#### Export Instance
```cmd
# Export instance to compressed package
vsenv export project-alpha "D:\Backups\project-alpha.vsenv"

# Export includes all configurations and extensions
```

#### Import Instance
```cmd
# Import instance (keep original name)
vsenv import "D:\Backups\project-alpha.vsenv"

# Import and rename instance
vsenv import "D:\Backups\project-alpha.vsenv" project-new
```

### View and Management
```cmd
# List all instances
vsenv list

# Output example:
# Instance Name    Path
# -----------------------------------------
# project-alpha    C:\Users\user\.vsenv\project-alpha
# project-beta     D:\Development\vscode-instances\beta
```

## 🏗️ Technical Architecture

### File Structure
```
.vsenv/
├── instance1/
│   ├── vscode/          # VS Code program files
│   ├── data/            # User data (configurations, state, etc.)
│   ├── extensions/      # Extensions directory
│   └── logs/            # Log files
├── instance2/
│   └── ...             # Another instance
└── otherPath.json      # Custom path mapping
```

### Registry Structure
```
HKEY_CURRENT_USER
└── Software
    ├── Classes
    │   ├── vsenv-{instancename}      # Custom protocol
    │   └── vscode                    # Protocol redirection
    └── VSenv
        └── {instancename}
            └── Hardware              # Hardware fingerprint configuration
```

### Isolation Mechanisms
1. **Filesystem-level Isolation**: Separate directory structure per instance
2. **Registry-level Isolation**: Independent protocol registration and configuration
3. **Process-level Isolation**: Optional sandbox execution environment
4. **Network-level Isolation**: Proxy configuration and virtual network devices

## 🔒 Privacy & Security

### Hardware Fingerprint Protection
VSenv can randomize the following hardware identifiers:
- **CPUID**: Processor identification
- **Disk Serial**: Disk serial numbers
- **MAC Address**: Network adapter addresses

### Network Privacy
- Support for HTTP/HTTPS proxies
- Virtual network adapter MAC address randomization
- Hostname randomization (admin rights required)

### Data Isolation
- Complete data isolation between instances
- Independent extension ecosystems
- Separate user configurations and state

## 🐛 Troubleshooting

### Common Issues

#### 1. VS Code Not Found
**Problem**: `Code.exe not found, please check the path.`
**Solution**: 
- Ensure VS Code offline package is extracted to correct directory
- Check if path exists: `instance directory\vscode\Code.exe`

#### 2. Insufficient Permissions
**Problem**: Network-related features require administrator privileges
**Solution**: Run Command Prompt as Administrator

#### 3. Protocol Redirection Failed
**Problem**: vscode:// links not redirecting correctly
**Solution**: 
```cmd
vsenv logoff        # Restore default first
vsenv regist <instance-name> # Re-register
```

#### 4. Extension Installation Failed
**Problem**: Errors during extension installation
**Solution**:
- Check network connection
- Verify extension ID is correct
- Try using proxy: `--proxy <proxy-address>`

### Log Files
Instance logs located at: `instance directory\logs\vsenv.log`

### Debug Mode
Set environment variable for verbose logging:
```cmd
set VSENV_DEBUG=1
vsenv start project-alpha
```

## 🤝 Contributing

We welcome community contributions! Please read the following guidelines:

### Development Environment Setup
1. Fork this repository
2. Clone your fork:
   ```bash
   git clone https://github.com/your-username/vsenv.git
   ```
3. Create feature branch:
   ```bash
   git checkout -b feature/amazing-feature
   ```
4. Commit changes:
   ```bash
   git commit -m 'Add some amazing feature'
   ```
5. Push to branch:
   ```bash
   git push origin feature/amazing-feature
   ```
6. Create Pull Request

### Code Standards
- Follow existing code style
- Add appropriate comments
- Update relevant documentation
- Test your changes

### Reporting Issues
Please report bugs or feature requests in GitHub Issues, including:
- Detailed problem description
- Reproduction steps
- System environment information
- Error logs (if available)

## 📄 License

This project is open source under the **AGPLv3.0** license.

```
VSenv - VS Code Offline Instance Manager
Copyright (C) 2025 dhjs0000

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

## 📋 Changelog

### v1.0.0 (2025-09-27)
- ✅ Instance creation, startup, stop, removal functionality
- ✅ Multi-level isolation mechanisms (file, registry, network)
- ✅ Hardware fingerprint spoofing
- ✅ Protocol redirection and guarding
- ✅ Batch extension management
- ✅ Instance import/export
- ✅ Multi-language support (Chinese/English)
- ✅ Sandbox execution environment
- ✅ Network isolation and proxy support

---

## 🔗 Related Links

- 📖 **Detailed Documentation**: [https://dhjs0000.github.io/VSenv/](https://dhjs0000.github.io/VSenv/)
- 🐛 **Issue Tracking**: [GitHub Issues](https://github.com/dhjs0000/vsenv/issues)
- 💬 **Discussion Forum**: [GitHub Discussions](https://github.com/dhjs0000/vsenv/discussions)
- ⭐ **Give a Star**: If this project helps you, please give it a ⭐

---

**Important Notice**: This software is completely free and open source. If you paid for this software, you have been scammed! Please obtain it through official channels.

---
*Last Updated: 9-2025*  
*Maintainer: [dhjs0000](https://github.com/dhjs0000)*