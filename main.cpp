/*
    VSenv --VS Code 离线实例管理器
    以AGPLv3.0开源 dhjs0000版权所有
    该程序允许用户创建、启动、停止和删除独立的 VS Code 实例，
    每个实例拥有独立的用户数据和扩展目录。

    版本：1.2.0
*/

// 常量定义

#define VSENV_VERSION "1.2.0"
#define VSENV_AUTHOR "dhjs0000"
#define VSENV_LICENSE "AGPLv3.0"


#define _WIN32_WINNT 0x0A00  // Windows 10

#include <sdkddkver.h>
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#include <iostream>
#include <string>
#include <userenv.h>
#include <errno.h>  // 添加errno头文件
#include <fstream>
#include <shlwapi.h>   // 为了 他妈的PathAppendA
#include <random>      // 添加随机数生成
#include <iomanip>     // 添加流格式化
#include <vector>

#include <sstream>
#include <unordered_map>
#include <fstream>
#include <nlohmann/json.hpp> // 需要添加JSON库支持
#include <tlhelp32.h>
#include <psapi.h>


using json = nlohmann::json;
static std::unordered_map<std::string, std::string> g_otherPath;

static void loadOtherPath();
static void saveOtherPathEntry(const std::string& name, const std::string& real);

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "userenv.lib")

using std::string;
using std::cout;
using std::cerr;

/* =========== 语言包 =========== */
struct L10N {
    string title = "VSenv - Stand-alone VS Code instance manager";
    string created = "Instance '%s' created at %s";
    string copyHint = "Please extract the offline package to %s\\vscode\\";
    string notFound = "Code.exe not found, please check the path.";
    string started = "Started %s";
    string stopped = "Stopped %s";
    string removed = "Removed %s";
    string registOK = "vscode:// now redirects to instance '%s'";
    string logoffOK = "vscode:// restored to original VS Code";
    string restHint = "Cannot redirect to VS Code? Use --rest to restore/rebuild vscode:// protocol";
    string fakeHW = "Hardware fingerprints randomized: CPUID=%s, DiskSN=%s, MAC=%s";
    string extNotFound = "VS Code CLI not found, cannot install extension.";
    string extOK = "Extension '%s' installed.";
    string checkingCN = "Checking Chinese language pack...";
    string alreadyCN = "Chinese language pack already installed.";
};

static const L10N EN; // 默认英文
static L10N CN = { "VSenv - 独立 VS Code 实例管理器",
                   "实例 '%s' 已创建：%s",
                   "请把离线包解压到 %s\\vscode\\",
                   "未找到 Code.exe，请检查路径。",
                   "已启动 %s",
                   "已停止 %s",
                   "已删除 %s",
                   "vscode:// 现在会跳转到实例'%s'",
                   "vscode:// 现在会跳转到原来的VSCode",
                   "无法跳转至 VS Code？使用 --rest 以恢复/重建 vscode:// 协议",
                   "硬件指纹已随机化: CPUID=%s, 磁盘序列号=%s, MAC地址=%s"
                   "VS Code CLI 未找到，无法安装扩展。",
                   "扩展 '%s' 已安装。"
                   "正在检查中文语言包...",
                   "中文语言包已安装。"
};

namespace con
{
    // 传统颜色代码
    enum LegacyColor
    {
        BLACK = 0,
        DARK_BLUE = FOREGROUND_BLUE,
        DARK_GREEN = FOREGROUND_GREEN,
        DARK_CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
        DARK_RED = FOREGROUND_RED,
        DARK_MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
        DARK_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
        GRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        DARK_GRAY = GRAY | FOREGROUND_INTENSITY,
        BLUE = DARK_BLUE | FOREGROUND_INTENSITY,
        GREEN = DARK_GREEN | FOREGROUND_INTENSITY,
        CYAN = DARK_CYAN | FOREGROUND_INTENSITY,
        RED = DARK_RED | FOREGROUND_INTENSITY,
        MAGENTA = DARK_MAGENTA | FOREGROUND_INTENSITY,
        YELLOW = DARK_YELLOW | FOREGROUND_INTENSITY,
        WHITE = GRAY | FOREGROUND_INTENSITY
    };

    static bool vtEnabled = false;

    // 初始化：优先 VT，失败则用传统 API
    void init()
    {

        HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
        if (hOut == INVALID_HANDLE_VALUE) return;

        // 尝试开启 VT 处理
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode))
        {
            if (SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
                vtEnabled = true;
            else
                SetConsoleMode(hOut, mode); // 还原
        }
    }

    // 设置颜色：VT 转义 或 SetConsoleTextAttribute
    void setColor(LegacyColor c)
    {
        HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
        if (vtEnabled)
        {
            // 粗略映射到 ANSI
            WORD w = static_cast<WORD>(c);
            if (w & FOREGROUND_INTENSITY) std::cerr << "\033[1m";
            if (w & FOREGROUND_RED)       std::cerr << "\033[31m";
            if (w & FOREGROUND_GREEN)     std::cerr << "\033[32m";
            if (w & FOREGROUND_BLUE)      std::cerr << "\033[34m";
            if ((w & (FOREGROUND_RED | FOREGROUND_GREEN)) == (FOREGROUND_RED | FOREGROUND_GREEN))
                std::cerr << "\033[33m";
            if ((w & (FOREGROUND_GREEN | FOREGROUND_BLUE)) == (FOREGROUND_GREEN | FOREGROUND_BLUE))
                std::cerr << "\033[36m";
            if ((w & (FOREGROUND_RED | FOREGROUND_BLUE)) == (FOREGROUND_RED | FOREGROUND_BLUE))
                std::cerr << "\033[35m";
            if (w == WHITE) std::cerr << "\033[37m";
        }
        else
        {
            SetConsoleTextAttribute(hOut, static_cast<WORD>(c));
        }
    }

    void reset()
    {
        if (vtEnabled)
            std::cerr << "\033[0m";
        else
            setColor(GRAY);
    }
}


// --------------- 用法字符串 ---------------
void showUsage()
{
    using namespace con;

    auto title = []() { setColor(CYAN); };
    auto cmd = []() { setColor(GREEN); };
    auto opt = []() { setColor(DARK_GRAY); };
    auto url = []() { setColor(BLUE); SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY); };
    auto rst = []() { reset(); };

    std::cerr << "\n";
    title(); std::cerr << "用法：\n"; rst();
    cmd();   std::cerr << "  vsenv --version"; rst(); std::cerr << "\t\t\t\t显示版本信息\n";
    cmd();   std::cerr << "  vsenv create"; rst(); std::cerr << " <实例名> [路径] \n";
    cmd();   std::cerr << "  vsenv start";  rst(); std::cerr << "  <实例名> ";
    opt();   std::cerr << "[--host] [--mac] [--proxy <url>] [--sandbox] [--fake-hw]"; rst(); std::cerr << "\n";
    std::cerr << "  "; title(); std::cerr << "[推荐]"; rst(); cmd(); std::cerr << "vsenv f"; rst(); std::cerr << "\t\t\t\t\t交互模式启动实例\n";
    cmd();   std::cerr << "  vsenv stop";   rst(); std::cerr << "   <实例名> \n";
    cmd();   std::cerr << "  vsenv remove"; rst(); std::cerr << " <实例名> \n";
    cmd();   std::cerr << "  vsenv regist"; rst(); std::cerr << " <实例名>\t\t\t\t将 vscode:// 协议重定向到此实例\n";
    cmd();   std::cerr << "  vsenv regist-guard"; rst(); std::cerr << " <实例名>\t\t\t守护 vscode:// 协议不被篡改（需管理员权限）\n";
    cmd();   std::cerr << "  vsenv logoff"; rst(); std::cerr << "\t\t\t\t\t恢复默认的 vscode:// 协议处理程序\n";
    cmd();   std::cerr << "  vsenv rest"; rst(); std::cerr << " <路径>\t\t\t\t手动重建 vscode:// 协议（支持拖拽带双引号的路径）\n";
    cmd();   std::cerr << "  vsenv extension"; rst(); std::cerr << " <实例名> <扩展ID>\t\t安装单个扩展\n";
    cmd();   std::cerr << "  vsenv extension "; rst(); std::cerr << "<实例名> "; cmd(); std::cerr << "import"; rst(); std::cerr << " <列表文件>\t批量安装\n";
    cmd();   std::cerr << "  vsenv extension "; rst(); std::cerr << "<实例名> "; cmd(); std::cerr << "list";   rst(); std::cerr << "\t\t列出已装扩展\n";
    cmd();   std::cerr << "  vsenv list"; rst(); std::cerr << "\t\t\t\t\t列出全部实例\n";
    cmd();   std::cerr << "  vsenv export"; rst(); std::cerr << " <实例名> <导出路径>\t\t导出实例\n";
    cmd();   std::cerr << "  vsenv import"; rst(); std::cerr << " <配置文件> [新实例名]\t\t导入实例\n";
    std::cerr << "\n";
    title(); std::cerr << "全局选项：\n"; rst();
    opt();   std::cerr << "  --lang <en|cn>"; rst() ; std::cerr << "   设置界面语言，默认为 \"en\"。\n"; rst();
    std::cerr << "\n";
    title(); std::cerr << "vsenv start 选项：\n"; rst();
    opt();   std::cerr << "  --host"; rst(); std::cerr << "              在当前 Windows 会话内随机化主机名（需管理员权限）。\n"; rst();
    std::cerr << "                      可用于隐藏真实计算机名。\n";
    opt();   std::cerr << "  --mac"; rst(); std::cerr << "               生成随机 MAC 地址，并创建名为 VSenv-<实例名> 的临时虚拟网卡（需管理员权限）。\n"; rst();
    opt();   std::cerr << "  --proxy <url>"; rst(); std::cerr << "       强制所有 WinHTTP 流量通过指定的 HTTP(S) 代理。\n"; rst();
    std::cerr << "                      示例：";
    title(); std::cerr << "--proxy http://127.0.0.1:8080"; rst();
    std::cerr << " \n";
    opt();   std::cerr << "  --sandbox"; rst(); std::cerr << "           在受限的 Logon-Session 沙箱内启动 VS Code。\n"; rst();
    opt();   std::cerr << "  --fake-hw"; rst(); std::cerr << "           随机化硬件指纹（CPUID、磁盘序列号、MAC）以增强隐私和隔离。\n"; rst();
    std::cerr << "\n";
    url();   std::cerr << "更多帮助：https://dhjs0000.github.io/VSenv/helps.html\n"; rst();
}
/* =========== 工具函数 =========== */
bool fileExists(const std::string& path);
string rootDir(const string& name);
string homeDir();
void create(const string& name, const string& customPath, const L10N& L);

long long getFileSize(const string& filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

string vsCodeCliPath(const string& name) {
    return rootDir(name) + "\\vscode\\bin\\code.cmd";
}
// 工具：读取当前注册表值
static bool readRegValue(const string& keyPath, const string& valueName,
    char* buf, DWORD bufSize)
{
    return RegGetValueA(HKEY_CURRENT_USER,
        keyPath.c_str(),
        valueName.empty() ? nullptr : valueName.c_str(),
        RRF_RT_REG_SZ,
        nullptr,
        buf,
        &bufSize) == ERROR_SUCCESS;
}

// 工具：写入注册表值
static bool writeRegValue(const string& keyPath, const string& valueName,
    const string& data)
{
    HKEY hKey;
    LONG ret = RegCreateKeyExA(HKEY_CURRENT_USER,
        keyPath.c_str(),
        0, nullptr, 0, KEY_SET_VALUE,
        nullptr, &hKey, nullptr);
    if (ret != ERROR_SUCCESS) return false;
    ret = RegSetValueExA(hKey,
        valueName.empty() ? "" : valueName.c_str(),
        0, REG_SZ,
        reinterpret_cast<const BYTE*>(data.c_str()),
        static_cast<DWORD>(data.size() + 1));
    RegCloseKey(hKey);
    return ret == ERROR_SUCCESS;
}

// 生成我们希望的命令行
static string makeGuardCommand(const string& instanceName)
{
    char userProfile[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, userProfile);

    char exePath[MAX_PATH];
    PathCombineA(exePath, userProfile,
        (".vsenv\\" + instanceName + "\\vscode\\Code.exe").c_str());
    char dataDir[MAX_PATH];
    PathCombineA(dataDir, userProfile,
        (".vsenv\\" + instanceName + "\\data").c_str());
    char extDir[MAX_PATH];
    PathCombineA(extDir, userProfile,
        (".vsenv\\" + instanceName + "\\extensions").c_str());

    char cmd[4096];
    snprintf(cmd, sizeof(cmd),
        "\"%s\" --user-data-dir=\"%s\" --extensions-dir=\"%s\" --open-url -- %%1",
        exePath, dataDir, extDir);
    return string(cmd);
}

// 守护循环
static void guardRegist(const string& instanceName, const L10N& L)
{
    const string key = "Software\\Classes\\vscode\\shell\\open\\command";
    const string ourCmd = makeGuardCommand(instanceName);

    cout << "开始守护 vscode:// 协议，按 Ctrl+C 停止...\n";

    while (true)
    {
        char current[4096] = { 0 };
        DWORD len = sizeof(current);
        if (!readRegValue(key, "", current, len))
        {
            // 读不到就视为被删了
            cout << "[WARN] 注册表读取失败，尝试重新写入...\n";
        }

        if (string(current) != ourCmd)
        {
            cout << "[INFO] 检测到协议被修改，正在修复...\n";
            if (writeRegValue(key, "", ourCmd))
                cout << "[OK] 已修复\n";
            else
                cout << "[ERR] 修复失败\n";
        }
        Sleep(1000);   // 每 1 秒检查一次
    }
}

bool restoreCodeProtocol(const std::string& codePath)
{
    if (!fileExists(codePath)) return false;

    std::string cmd = "\"" + codePath + "\" --open-url -- \"%1\"";

    // 1. 重建命令 - 修正为 vscode 协议
    HKEY hKey;
    std::string key = "Software\\Classes\\vscode\\shell\\open\\command";
    RegCreateKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    RegSetValueExA(hKey, "", 0, REG_SZ, (LPBYTE)cmd.c_str(), (DWORD)cmd.size() + 1);
    RegCloseKey(hKey);

    // 2. 加 URL Protocol
    key = "Software\\Classes\\vscode";
    RegCreateKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    RegSetValueExA(hKey, "URL Protocol", 0, REG_SZ, (LPBYTE)"", 1);
    RegCloseKey(hKey);

    return true;
}

string homeDir() {
    char buf[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, buf);
    return buf;
}

string rootDir(const string& name)
{
    string def = homeDir() + "\\.vsenv\\" + name;
    if (fileExists(def)) return def;

    loadOtherPath();
    auto it = g_otherPath.find(name);
    if (it != g_otherPath.end() && fileExists(it->second))
        return it->second;

    return def; // 实在没有就返回默认，留给调用者报错
}

bool fileExists(const string& path) {
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool backupOriginalVSCodeHandler()
{
    char buf[1024];
    DWORD len = sizeof(buf);
    // 检查 vscode 协议是否存在
    if (RegGetValueA(HKEY_CURRENT_USER,
        "Software\\Classes\\vscode\\shell\\open\\command",
        "", RRF_RT_REG_SZ, nullptr, buf, &len) == ERROR_SUCCESS)
    {
        // 备份到特定位置
        RegSetKeyValueA(HKEY_CURRENT_USER,
            "Software\\Classes",
            "vsenv_backup_vscode_cmd",
            REG_SZ,
            buf,
            len);
        return true;
    }
    return false;
}

bool restoreOriginalVSCodeHandler()
{
    char buf[1024];
    DWORD len = sizeof(buf);
    if (RegGetValueA(HKEY_CURRENT_USER,
        "Software\\Classes",
        "vsenv_backup_vscode_cmd",
        RRF_RT_REG_SZ, nullptr, buf, &len) == ERROR_SUCCESS)
    {
        // 重建 vscode:// 协议
        HKEY hKey;
        RegCreateKeyA(HKEY_CURRENT_USER,
            "Software\\Classes\\vscode\\shell\\open\\command",
            &hKey);
        RegSetValueExA(hKey, "", 0, REG_SZ, (LPBYTE)buf, len);
        RegCloseKey(hKey);

        // 设置 URL Protocol
        RegCreateKeyA(HKEY_CURRENT_USER,
            "Software\\Classes\\vscode",
            &hKey);
        RegSetValueExA(hKey, "URL Protocol", 0, REG_SZ, (LPBYTE)"", 1);
        RegCloseKey(hKey);

        // 删除备份
        RegDeleteKeyValueA(HKEY_CURRENT_USER,
            "Software\\Classes",
            "vsenv_backup_vscode_cmd");
        return true;
    }
    return false;
}

string officialCodeExe() {
    char pf[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, pf);
    string path = string(pf) + "\\Programs\\Microsoft VS Code\\Code.exe";
    return fileExists(path) ? path : "";
}

bool registerCustomProtocol(const string& instanceName)
{
    // 1. 拼绝对路径
    char userProfile[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, userProfile);

    char exePath[MAX_PATH];
    PathCombineA(exePath, userProfile, (".vsenv\\" + instanceName + "\\vscode\\Code.exe").c_str());

    char dataDir[MAX_PATH];
    PathCombineA(dataDir, userProfile, (".vsenv\\" + instanceName + "\\data").c_str());

    char extDir[MAX_PATH];
    PathCombineA(extDir, userProfile, (".vsenv\\" + instanceName + "\\extensions").c_str());

    // 2. 修复命令行参数：添加 --open-url 和 --
    string cmdLine;
    cmdLine += "\"";
    cmdLine += exePath;
    cmdLine += "\" --open-url --user-data-dir=\"";
    cmdLine += dataDir;
    cmdLine += "\" --extensions-dir=\"";
    cmdLine += extDir;
    cmdLine += "\" -- \"%1\"";

    // 3. 写注册表
    HKEY hKey;
    string protocol = "Software\\Classes\\vsenv-" + instanceName;

    // 根键
    RegCreateKeyExA(HKEY_CURRENT_USER, protocol.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    RegSetValueExA(hKey, "", 0, REG_SZ, (LPBYTE)("URL:vsenv-" + instanceName).c_str(), (DWORD)("URL:vsenv-" + instanceName).size() + 1);
    RegSetValueExA(hKey, "URL Protocol", 0, REG_SZ, (LPBYTE)"", 1);
    RegCloseKey(hKey);

    // shell/open/command
    protocol += "\\shell\\open\\command";
    RegCreateKeyExA(HKEY_CURRENT_USER, protocol.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr);
    RegSetValueExA(hKey, "", 0, REG_SZ, (LPBYTE)cmdLine.c_str(), (DWORD)cmdLine.size() + 1);
    RegCloseKey(hKey);

    return true;
}

bool registVSCodeProtocol(const std::string& instanceName)
{
    char userProfile[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, userProfile);

    char exePath[MAX_PATH];
    PathCombineA(exePath, userProfile,
        (".vsenv\\" + instanceName + "\\vscode\\Code.exe").c_str());
    if (!fileExists(exePath))
        return false;

    char dataDir[MAX_PATH];
    PathCombineA(dataDir, userProfile,
        (".vsenv\\" + instanceName + "\\data").c_str());

    char extDir[MAX_PATH];
    PathCombineA(extDir, userProfile,
        (".vsenv\\" + instanceName + "\\extensions").c_str());

    // 直接注册一条最简单的命令行
    char cmdLine[4096];
    snprintf(cmdLine, sizeof(cmdLine),
        "\"%s\" --user-data-dir=\"%s\" --extensions-dir=\"%s\" --open-url -- %%1",
        exePath, dataDir, extDir);

    // 写入 HKEY_CURRENT_USER\Software\Classes\vscode\shell\open\command
    HKEY hKey;
    std::string key = "Software\\Classes\\vscode\\shell\\open\\command";
    if (RegCreateKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, nullptr, 0,
        KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return false;

    RegSetValueExA(hKey, "", 0, REG_SZ,
        reinterpret_cast<const BYTE*>(cmdLine),
        static_cast<DWORD>(strlen(cmdLine) + 1));
    RegCloseKey(hKey);

    // 写入 URL Protocol 空值
    key = "Software\\Classes\\vscode";
    if (RegCreateKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, nullptr, 0,
        KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return false;

    RegSetValueExA(hKey, "URL Protocol", 0, REG_SZ,
        reinterpret_cast<const BYTE*>(""), 1);
    RegCloseKey(hKey);

    return true;
}
 
/* =========== 硬件指纹伪造 =========== */
struct FakeHardwareID {
    string cpuID;
    string diskSN;
    string macAddress;
};

FakeHardwareID generateFakeHardwareID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    // 生成随机的CPU ID (16字节十六进制)
    char cpuBuf[17];
    for (int i = 0; i < 16; i += 4) {
        sprintf_s(cpuBuf + i, 5, "%04X", dis(gen) << 8 | dis(gen));
    }
    cpuBuf[16] = '\0';

    // 生成随机的磁盘序列号 (8字节十六进制)
    char diskBuf[9];
    for (int i = 0; i < 8; i += 2) {
        sprintf_s(diskBuf + i, 3, "%02X", dis(gen));
    }
    diskBuf[8] = '\0';

    // 生成随机的MAC地址
    char macBuf[18];
    sprintf_s(macBuf, sizeof(macBuf), "%02X:%02X:%02X:%02X:%02X:%02X",
        dis(gen), dis(gen), dis(gen), dis(gen), dis(gen), dis(gen));

    return { cpuBuf, diskBuf, macBuf };
}

void applyFakeHardwareProfile(const string& instanceName, const FakeHardwareID& fakeID) {
    string regPath = "Software\\VSenv\\" + instanceName + "\\Hardware";
    HKEY hKey;
    RegCreateKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, nullptr,
        REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);

    RegSetValueExA(hKey, "CPUID", 0, REG_SZ,
        (const BYTE*)fakeID.cpuID.c_str(), fakeID.cpuID.size() + 1);
    RegSetValueExA(hKey, "DiskSerial", 0, REG_SZ,
        (const BYTE*)fakeID.diskSN.c_str(), fakeID.diskSN.size() + 1);
    RegSetValueExA(hKey, "MACAddress", 0, REG_SZ,
        (const BYTE*)fakeID.macAddress.c_str(), fakeID.macAddress.size() + 1);

    RegCloseKey(hKey);
}

static void loadOtherPath()
{
    g_otherPath.clear();
    std::string jsonFile = homeDir() + "\\.vsenv\\otherPath.json";

    // 目录不存在就拉倒，不报错
    if (!fileExists(jsonFile))
        return;

    std::ifstream f(jsonFile);
    if (!f) return;

    std::string line;
    while (std::getline(f, line)) {
        auto pos = line.find('\t');
        if (pos == std::string::npos) continue;
        g_otherPath[line.substr(0, pos)] = line.substr(pos + 1);
    }
}

static void saveOtherPathEntry(const std::string& name, const std::string& real)
{
    loadOtherPath();                       // 先读，防止覆盖已有条目
    g_otherPath[name] = real;

    std::string vsenvDir = homeDir() + "\\.vsenv";
    std::string jsonFile = vsenvDir + "\\otherPath.json";

    // 1. 确保目录存在
    if (!fileExists(vsenvDir))
        SHCreateDirectoryExA(nullptr, vsenvDir.c_str(), nullptr);

    // 2. 写文件
    std::ofstream f(jsonFile, std::ios::trunc);
    if (!f) {
        cerr << "警告：无法写入 otherPath.json\n";
        return;
    }
    for (const auto& kv : g_otherPath)
        f << kv.first << '\t' << kv.second << '\n';
}

/* ===========  =========== */
void printBanner() {
    cout << "=======================================\n";
    cout << "  本软件完全免费开源，买到就是被骗啦！\n";
    cout << "  https://github.com/dhjs0000/vsenv\n";
    cout << "=======================================\n\n";
    cout << " VSenv " << VSENV_VERSION << " by " << VSENV_AUTHOR << " (" << VSENV_LICENSE << ")\n\n";
    cout << " VSenv (C) 2025 by dhjs0000，为爱发电中～\n\n";
}

/* =========== 隔离方案实现 =========== */
bool startInSandbox(const string& name, const L10N& L) {
    string dir = rootDir(name);
    string exe = dir + "\\vscode\\Code.exe";
    if (!fileExists(exe)) { cerr << L.notFound << "\n"; return false; }

    HANDLE hToken;
    if (!LogonUserA("VSenv", ".", "dummyPwd", LOGON32_LOGON_INTERACTIVE,
        LOGON32_PROVIDER_DEFAULT, &hToken)) return false;

    HANDLE hRestricted;
    if (!CreateRestrictedToken(hToken, 0, 0, nullptr, 0, nullptr, 0, nullptr, &hRestricted)) {
        CloseHandle(hToken); return false;
    }

    STARTUPINFOA si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    string args = "--user-data-dir=\"" + dir + "\\data\" --extensions-dir=\"" + dir + "\\extensions\"";
    if (CreateProcessAsUserA(hRestricted, exe.c_str(), const_cast<char*>(args.c_str()),
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        printf(L.started.c_str(), name.c_str());
    }
    CloseHandle(hRestricted);
    CloseHandle(hToken);
    return true;
}

bool startInAppContainer(const string& name, const L10N& L) {
    string dir = rootDir(name);
    string exe = dir + "\\vscode\\Code.exe";
    if (!fileExists(exe)) {
        cerr << L.notFound << "\n";
        return false;
    }

    // 1. 创建 AppContainer 配置文件
    PSID appContainerSid = nullptr;

    // 添加 UI 访问能力
    WELL_KNOWN_SID_TYPE capabilities[] = {
        WinCapabilityInternetClientSid,
        WinCapabilityPrivateNetworkClientServerSid,
        WinCapabilityPicturesLibrarySid,
        WinCapabilityVideosLibrarySid,
        WinCapabilityMusicLibrarySid,
        WinCapabilityDocumentsLibrarySid,
        WinCapabilityEnterpriseAuthenticationSid,
        WinCapabilitySharedUserCertificatesSid
    };

    SID_AND_ATTRIBUTES caps[ARRAYSIZE(capabilities)];
    for (int i = 0; i < ARRAYSIZE(capabilities); i++) {
        caps[i].Attributes = SE_GROUP_ENABLED;
        caps[i].Sid = (PSID)LocalAlloc(LPTR, SECURITY_MAX_SID_SIZE);
        DWORD sidSize = SECURITY_MAX_SID_SIZE;
        CreateWellKnownSid(capabilities[i], NULL, caps[i].Sid, &sidSize);
    }

    HRESULT hr = CreateAppContainerProfile(
        L"VSenvAppContainer",
        L"VSenv AppContainer",
        L"VS Code AppContainer Profile",
        caps,   // pCapabilities
        ARRAYSIZE(caps), // capabilityCount
        &appContainerSid
    );

    // 清理能力数组
    for (int i = 0; i < ARRAYSIZE(capabilities); i++) {
        LocalFree(caps[i].Sid);
    }

    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)) {
        cerr << "创建 AppContainer 失败: 0x" << std::hex << hr << "\n";
        return false;
    }

    // 2. 准备进程属性列表
    STARTUPINFOEXA siex = { sizeof(siex) };
    PROCESS_INFORMATION pi = {};

    SIZE_T size = 0;
    InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
    siex.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(
        GetProcessHeap(), 0, size);

    if (!siex.lpAttributeList ||
        !InitializeProcThreadAttributeList(siex.lpAttributeList, 1, 0, &size))
    {
        cerr << "初始化属性列表失败\n";
        if (siex.lpAttributeList) HeapFree(GetProcessHeap(), 0, siex.lpAttributeList);
        return false;
    }

    // 3. 设置 AppContainer 属性
    SECURITY_CAPABILITIES sc = {};
    sc.AppContainerSid = appContainerSid;
    sc.Capabilities = caps;
    sc.CapabilityCount = ARRAYSIZE(caps);

    if (!UpdateProcThreadAttribute(siex.lpAttributeList,
        0,
        PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES,
        &sc,
        sizeof(sc),
        nullptr,
        nullptr))
    {
        cerr << "更新线程属性失败: " << GetLastError() << "\n";
        DeleteProcThreadAttributeList(siex.lpAttributeList);
        HeapFree(GetProcessHeap(), 0, siex.lpAttributeList);
        return false;
    }

    // 4. 准备命令行参数
    string args = "--user-data-dir=\"" + dir + "\\data\" --extensions-dir=\"" + dir + "\\extensions\"";
    string cmdLine = "\"" + exe + "\" " + args;

    // 5. 创建进程
    BOOL success = CreateProcessA(
        exe.c_str(),
        const_cast<char*>(cmdLine.c_str()),
        nullptr,
        nullptr,
        FALSE,
        EXTENDED_STARTUPINFO_PRESENT | CREATE_NEW_CONSOLE, // 添加 CREATE_NEW_CONSOLE
        nullptr,
        nullptr,
        &siex.StartupInfo,
        &pi
    );

    // 6. 清理资源
    DeleteProcThreadAttributeList(siex.lpAttributeList);
    HeapFree(GetProcessHeap(), 0, siex.lpAttributeList);

    if (success) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        printf(L.started.c_str(), name.c_str());
        return true;
    }
    else {
        cerr << "CreateProcess 失败: " << GetLastError() << "\n";
        return false;
    }
}
// 战略保留 AppCntainer
void startInWindowsSandbox(const string& name, const L10N& L) {
    string dir = rootDir(name);
    string exe = dir + "\\vscode\\Code.exe";
    if (!fileExists(exe)) { cerr << L.notFound << "\n"; return; }

    string cmd = R"(powershell -Command "$wsb = @'
<Configuration>
  <MappedFolders>
    <MappedFolder>
      <HostFolder>)" + dir + R"(</HostFolder>
      <ReadOnly>true</ReadOnly>
    </MappedFolder>
  </MappedFolders>
  <LogonCommand>
    <Command>)" + exe + R"( --user-data-dir C:\vsenv\)" + name + R"(\data --extensions-dir C:\vsenv\)" + name + R"(\extensions</Command>
  </LogonCommand>
</Configuration>
'@ | Out-File $env:TEMP\vsenv.wsb -Encoding UTF8; Start-Process $env:TEMP\vsenv.wsb")";
    system(cmd.c_str());
    printf(L.started.c_str(), name.c_str());
}
// 战略保留WSB
void startWithNetworkIsolation(const string& name, const L10N& L, bool randomHost, bool randomMac, const string& proxy) {
    string dir = rootDir(name);
    string exe = dir + "\\vscode\\Code.exe";
    if (!fileExists(exe)) { cerr << L.notFound << "\n"; return; }

    // 简化并修复 PowerShell 脚本
    string psScript = "powershell -Command \"\n";
    psScript += "    $InstanceName = '" + name + "'\n";

    // 设置布尔变量
    if (randomHost) psScript += "    $RandomHost = $true\n";
    else psScript += "    $RandomHost = $false\n";

    if (randomMac) psScript += "    $RandomMac = $true\n";
    else psScript += "    $RandomMac = $false\n";

    if (!proxy.empty()) psScript += "    $Proxy = '" + proxy + "'\n";
    else psScript += "    $Proxy = $null\n";

    psScript +=
        "    # 检查管理员权限\n"
        "    $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)\n"
        "    if (-not $isAdmin) {\n"
        "        Write-Warning '需要管理员权限执行网络隔离操作'\n"
        "    }\n"
        "    \n"
        "    # 随机主机名(已删除)\n"
        "    \n"
        "    # 随机MAC地址\n"
        "    if ($RandomMac -and $isAdmin) {\n"
        "        try {\n"
        "            $MacBytes = 1..5 | ForEach-Object { Get-Random -Minimum 0 -Maximum 255 }\n"
        "            $Mac = '02:{0:X2}:{1:X2}:{2:X2}:{3:X2}:{4:X2}' -f $MacBytes\n"
        "            New-NetAdapter -Name (\"VSenv-$InstanceName\") -MacAddress $Mac -Confirm:$false\n"
        "        } catch {\n"
        "            Write-Warning '无法创建虚拟网卡，请以管理员身份运行'\n"
        "        }\n"
        "    }\n"
        "    \n"
        "    # 代理设置\n"
        "    if ($Proxy -and $Proxy -ne '') {\n"
        "        netsh winhttp set proxy $Proxy\n"
        "        Write-Host \"已启用代理 $Proxy\"\n"
        "    } else {\n"
        "        netsh winhttp reset proxy\n"
        "        Write-Host '已恢复直连'\n"
        "    }\n"
        "\"";

    // 输出脚本到文件用于调试
    std::ofstream scriptFile("debug_script.ps1");
    if (scriptFile.is_open()) {
        scriptFile << psScript;
        scriptFile.close();
    }
    else {
        cerr << "无法创建调试脚本文件\n";
    }

    // 执行 PowerShell 脚本
    system(psScript.c_str());

    // 启动 VS Code
    string args = "\"" + exe + "\" --user-data-dir=\"" + dir + "\\data\" --extensions-dir=\"" + dir + "\\extensions\"";
    STARTUPINFOA si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    if (CreateProcessA(nullptr, const_cast<char*>(args.c_str()),
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        printf(L.started.c_str(), name.c_str());
    }
    else {
        cerr << "启动失败，错误代码: " << GetLastError() << "\n";
    }
}

/* =========== 业务逻辑 =========== */

// 只负责打印，不再自己扫磁盘
void printInstanceTable(const std::vector<std::pair<std::string, std::string>>& instances)
{
    cout << "Instance Name\tPath\n";
    cout << "-----------------------------------------\n";
    for (const auto& [n, p] : instances)
    {
        cout << n << "\t" << p;
        if (!fileExists(p + "\\vscode\\Code.exe"))
            cout << " (无效实例)";
        cout << "\n";
    }
}

// 真正扫描磁盘，返回 vector
std::vector<std::pair<std::string, std::string>> enumerateInstances()
{
    std::vector<std::pair<std::string, std::string>> res;

    /* 1. 默认目录 */
    string base = homeDir() + "\\.vsenv";
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA((base + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                string name = fd.cFileName;
                if (name == "." || name == "..") continue;
                string path = base + "\\" + name;
                res.emplace_back(name, path);
            }
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }

    /* 2. otherPath.json */
    loadOtherPath();
    for (const auto& [n, p] : g_otherPath)
        res.emplace_back(n, p);

    return res;
}

void create(const string& name, const string& customPath, const L10N& L)
{
    string dir;
    if (!customPath.empty())
    {
        dir = customPath;
        // 去掉首尾双引号（用户拖拽路径时常带）
        if (dir.size() >= 2 && dir.front() == '"' && dir.back() == '"')
            dir = dir.substr(1, dir.size() - 2);
        saveOtherPathEntry(name, dir);
    }
    else
    {
        dir = rootDir(name);
    }
    if (_mkdir(homeDir().c_str()) == -1 && errno != EEXIST)
    {
        cerr << "Failed to create directory: " << dir << "\n";
        return;
    }
    if (_mkdir(dir.c_str()) == -1 && errno != EEXIST)
    {
        cerr << "Failed to create directory: " << dir << "\n";
        return;
    }

    if (_mkdir((dir + "\\data").c_str()) == -1 && errno != EEXIST) {
		cerr << "Failed to create data directory\n";
		return;
    }
    if (_mkdir((dir + "\\extensions").c_str()) == -1 && errno != EEXIST) {
        cerr << "Failed to create extensions directory\n";
        return;
    }

    if (!registerCustomProtocol(name))
        cerr << "Failed to register custom protocol\n";

    printf(L.created.c_str(), name.c_str(), dir.c_str());
    printf(("\n" + L.copyHint).c_str(), dir.c_str());
}

void start(const string& name, const L10N& L,
    bool randomHost, bool randomMac, const string& proxy,
	bool useSandbox, bool useAppContainer, bool useWSB, bool fakeHW) { // 由于前面程序使用了useAppContainer和useWSB参数，这里保留以防止编译错误

    if (fakeHW) {
        FakeHardwareID fakeID = generateFakeHardwareID();
        applyFakeHardwareProfile(name, fakeID);
        printf(L.fakeHW.c_str(), fakeID.cpuID.c_str(), fakeID.diskSN.c_str(), fakeID.macAddress.c_str());
        cout << "\n";
    }

    if (useSandbox) {
        if (startInSandbox(name, L)) return;
    }

    string dir = rootDir(name);
    string exe = dir + "\\vscode\\Code.exe";
    if (!fileExists(exe)) { cerr << L.notFound << "\n"; return; }

    if (randomHost || randomMac || !proxy.empty()) {
        startWithNetworkIsolation(name, L, randomHost, randomMac, proxy);
        return;
    }

    string args = "\"" + exe + "\" --user-data-dir=\"" + dir + "\\data\" --extensions-dir=\"" + dir + "\\extensions\"";
    STARTUPINFOA si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    if (CreateProcessA(exe.c_str(), const_cast<char*>(args.c_str()),
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
        printf(L.started.c_str(), name.c_str());
    }
    else {
        cerr << "Start failed\n";
    }
}

void stop(const string& name, const L10N& L, char* argv0) {
    //杀进程
    system(("taskkill /FI \"WINDOWTITLE eq " + name + "*\" /T /F >nul 2>&1").c_str());

    printf(L.stopped.c_str(), name.c_str());
}

void remove(const string& name, const L10N& L, char* argv0) {

    // 1. 如果当前劫持的是本实例，先还原 vscode://
    restoreOriginalVSCodeHandler();

    // 2. 删除实例私有协议
    SHDeleteKeyA(HKEY_CURRENT_USER, ("Software\\Classes\\vsenv-" + name).c_str());

    // 3. 删除硬件配置文件
    SHDeleteKeyA(HKEY_CURRENT_USER, ("Software\\VSenv\\" + name).c_str());

    // 4. 从 otherPath.json 中移除路径记录（如果存在）
    loadOtherPath();
    auto it = g_otherPath.find(name);
    if (it != g_otherPath.end()) {
        g_otherPath.erase(it);
        // 更新 otherPath.json 文件
        std::string vsenvDir = homeDir() + "\\.vsenv";
        std::string jsonFile = vsenvDir + "\\otherPath.json";
        std::ofstream f(jsonFile, std::ios::trunc);
        if (f) {
            for (const auto& kv : g_otherPath) {
                f << kv.first << '\t' << kv.second << '\n';
            }
        }
    }

    // 5. 删除目录
    string instanceDir = rootDir(name);
    system(("rmdir /s /q \"" + instanceDir + "\"").c_str());
    printf(L.removed.c_str(), name.c_str());
}

void interactiveMode(const L10N& lang)
{
    system("cls");
    printBanner();
    cout << "交互模式：↑↓ 选择，回车启动，: 添加参数，Esc 退出\n\n";

    auto instances = enumerateInstances();
    if (instances.empty())
    {
        cout << "没有可用实例。\n";
        return;
    }

    int cur = 0;
    std::string extraArgs;

    // 取控制台输入句柄
    HANDLE hCon = GetStdHandle(STD_INPUT_HANDLE);
    DWORD  mode;
    GetConsoleMode(hCon, &mode);
    SetConsoleMode(hCon, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));

    while (true)
    {
        // 清屏并重画
        COORD home = { 0, 11 };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), home);
        for (int i = 0; i < (int)instances.size(); ++i)
        {
            if (i == cur) cout << " > "; else cout << "   ";
            cout << instances[i].first;
            if (!fileExists(instances[i].second + "\\vscode\\Code.exe"))
                cout << " (无效)";
            cout << "\n";
        }
        cout << "\n当前参数：[" << extraArgs << "]\n";

        INPUT_RECORD rec;
        DWORD        count;
        ReadConsoleInputA(hCon, &rec, 1, &count);
        if (rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown)
            continue;

        WORD vk = rec.Event.KeyEvent.wVirtualKeyCode;
        if (vk == VK_UP) { cur = (cur - 1 + instances.size()) % instances.size(); }
        else if (vk == VK_DOWN) { cur = (cur + 1) % instances.size(); }
        else if (vk == VK_ESCAPE) break;
        else if (vk == VK_RETURN)
        {
            cout << "\n正在启动 " << instances[cur].first << " ...\n";
            SetConsoleMode(hCon, mode);          // 恢复控制台模式
            std::string fullCmd = instances[cur].first;
            if (!extraArgs.empty()) fullCmd += " " + extraArgs;
            // 解析参数并调用 start
            // 简单做法：把 extraArgs 拆成 argc/argv 形式再传进 start
            start(instances[cur].first, lang,
                extraArgs.find("--host") != std::string::npos,
                extraArgs.find("--mac") != std::string::npos,
                extraArgs.find("--proxy") != std::string::npos ?
                extraArgs.substr(extraArgs.find("--proxy") + 7) : "",
                extraArgs.find("--sandbox") != std::string::npos,
                false, false,                 // useAppContainer / useWSB 已废弃
                extraArgs.find("--fake-hw") != std::string::npos);
            break;
        }
        else if (vk == 0xBA)   // ':' 键
        {
            SetConsoleMode(hCon, mode);          // 先恢复行缓冲
            cout << "\n请输入附加参数（如 --sandbox --fake-hw）：";
            std::getline(std::cin, extraArgs);
            SetConsoleMode(hCon, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
        }
    }
    SetConsoleMode(hCon, mode);                // 保险恢复
}

/* =========== 导入导出相关 =========== */

// 导出实例函数（打包整个实例）
void exportInstance(const string& name, const string& exportPath, const L10N& L) {
    string dir = rootDir(name);
    if (!fileExists(dir + "\\vscode\\Code.exe")) {
        cerr << "错误：实例 '" << name << "' 不存在或无效\n";
        return;
    }

    // 确保输出目录存在
    string outputDir = exportPath.substr(0, exportPath.find_last_of("\\/"));
    if (!outputDir.empty() && !fileExists(outputDir)) {
        if (_mkdir(outputDir.c_str()) != 0 && errno != EEXIST) {
            cerr << "错误：无法创建输出目录 " << outputDir << "\n";
            return;
        }
    }

    cout << "正在打包实例 '" << name << "'...\n";

    // 创建临时目录用于打包
    string tempDir = homeDir() + "\\.vsenv\\temp_export_" + name;
    if (_mkdir(tempDir.c_str()) != 0 && errno != EEXIST) {
        cerr << "错误：无法创建临时目录\n";
        return;
    }

    // 复制实例文件到临时目录
    string copyCmd = "xcopy \"" + dir + "\" \"" + tempDir + "\\" + name + "\" /E /I /H /Y";
    int result = system(copyCmd.c_str());
    if (result != 0) {
        cerr << "错误：复制文件失败\n";
        system(("rmdir /s /q \"" + tempDir + "\"").c_str());
        return;
    }

    // 创建配置文件
    json config;
    config["version"] = VSENV_VERSION;
    config["instanceName"] = name;

    // 检查是否是自定义路径
    loadOtherPath();
    auto it = g_otherPath.find(name);
    if (it != g_otherPath.end()) {
        config["customPath"] = it->second;
    }
    else {
        config["customPath"] = "";
    }

    // 读取硬件指纹配置（如果存在）
    string regPath = "Software\\VSenv\\" + name + "\\Hardware";
    char value[256];
    DWORD size = sizeof(value);

    if (readRegValue(regPath, "CPUID", value, size)) {
        config["hardware"]["CPUID"] = value;
    }
    if (readRegValue(regPath, "DiskSerial", value, size)) {
        config["hardware"]["DiskSerial"] = value;
    }
    if (readRegValue(regPath, "MACAddress", value, size)) {
        config["hardware"]["MACAddress"] = value;
    }

    // 保存配置文件
    std::ofstream configFile(tempDir + "\\config.json");
    configFile << config.dump(4);
    configFile.close();

    // 创建压缩包
    string zipCmd = "powershell -Command \"Compress-Archive -Path '" + tempDir + "\\*' -DestinationPath '" + exportPath + "' -Force\"";
    result = system(zipCmd.c_str());

    // 清理临时目录
    system(("rmdir /s /q \"" + tempDir + "\"").c_str());

    if (result == 0) {
        cout << "实例已成功导出到: " << exportPath << "\n";
        cout << "文件大小: " << getFileSize(exportPath) / (1024 * 1024) << " MB\n";
    }
    else {
        cerr << "错误：创建压缩包失败\n";
    }
}

// 导入实例函数（解压并恢复整个实例）
void importInstance(const string& importFile, const string& newName, const L10N& L) {
    if (!fileExists(importFile)) {
        cerr << "错误：配置文件不存在\n";
        return;
    }

    // 创建临时目录用于解压
    string tempDir = homeDir() + "\\.vsenv\\temp_import";
    if (_mkdir(tempDir.c_str()) != 0 && errno != EEXIST) {
        cerr << "错误：无法创建临时目录\n";
        return;
    }

    cout << "正在解压实例包...\n";

    // 解压文件
    string unzipCmd = "powershell -Command \"Expand-Archive -Path '" + importFile + "' -DestinationPath '" + tempDir + "' -Force\"";
    int result = system(unzipCmd.c_str());
    if (result != 0) {
        cerr << "错误：解压文件失败\n";
        system(("rmdir /s /q \"" + tempDir + "\"").c_str());
        return;
    }

    // 读取配置文件
    string configFile = tempDir + "\\config.json";
    if (!fileExists(configFile)) {
        cerr << "错误：配置文件不存在于包中\n";
        system(("rmdir /s /q \"" + tempDir + "\"").c_str());
        return;
    }

    try {
        std::ifstream file(configFile);
        json config = json::parse(file);
        file.close();

        // 检查版本兼容性
        string configVersion = config.value("version", "");
        if (configVersion != VSENV_VERSION) {
            cout << "警告：配置文件版本(" << configVersion
                << ")与当前版本(" << VSENV_VERSION
                << ")不同，可能不兼容\n";
        }

        // 确定实例名称
        string originalName = config.value("instanceName", "");
        string instanceName = newName.empty() ? originalName : newName;

        if (instanceName.empty()) {
            cerr << "错误：配置文件中没有实例名称\n";
            system(("rmdir /s /q \"" + tempDir + "\"").c_str());
            return;
        }

        // 检查实例是否已存在
        if (fileExists(rootDir(instanceName) + "\\vscode\\Code.exe")) {
            cerr << "错误：实例 '" << instanceName << "' 已存在\n";
            system(("rmdir /s /q \"" + tempDir + "\"").c_str());
            return;
        }

        // 确定目标路径
        string customPath = config.value("customPath", "");
        string targetDir;

        if (!customPath.empty()) {
            targetDir = customPath;
            // 如果原路径不可用，使用默认路径
            if (!fileExists(customPath)) {
                cout << "警告：原始路径不可用，使用默认路径\n";
                targetDir = homeDir() + "\\.vsenv\\" + instanceName;
            }
        }
        else {
            targetDir = homeDir() + "\\.vsenv\\" + instanceName;
        }

        // 确保目标目录存在
        if (_mkdir(targetDir.c_str()) != 0 && errno != EEXIST) {
            cerr << "错误：无法创建目标目录\n";
            system(("rmdir /s /q \"" + tempDir + "\"").c_str());
            return;
        }

        // 复制文件到目标位置
        string sourceDir = tempDir + "\\" + originalName;
        if (!fileExists(sourceDir)) {
            cerr << "错误：实例文件不存在于包中\n";
            system(("rmdir /s /q \"" + tempDir + "\"").c_str());
            return;
        }

        cout << "正在复制文件到目标位置...\n";
        string copyCmd = "xcopy \"" + sourceDir + "\" \"" + targetDir + "\" /E /I /H /Y";
        result = system(copyCmd.c_str());
        if (result != 0) {
            cerr << "错误：复制文件失败\n";
            system(("rmdir /s /q \"" + tempDir + "\"").c_str());
            return;
        }

        // 如果不是默认路径，更新路径映射
        if (targetDir != homeDir() + "\\.vsenv\\" + instanceName) {
            saveOtherPathEntry(instanceName, targetDir);
        }

        // 应用硬件配置（如果存在）
        if (config.contains("hardware")) {
            json hardware = config["hardware"];
            FakeHardwareID fakeID;

            if (hardware.contains("CPUID")) {
                fakeID.cpuID = hardware["CPUID"];
            }
            if (hardware.contains("DiskSerial")) {
                fakeID.diskSN = hardware["DiskSerial"];
            }
            if (hardware.contains("MACAddress")) {
                fakeID.macAddress = hardware["MACAddress"];
            }

            applyFakeHardwareProfile(instanceName, fakeID);
            printf(L.fakeHW.c_str(), fakeID.cpuID.c_str(),
                fakeID.diskSN.c_str(), fakeID.macAddress.c_str());
            cout << "\n";
        }

        // 注册自定义协议
        if (!registerCustomProtocol(instanceName)) {
            cerr << "警告：注册自定义协议失败\n";
        }

        cout << "实例已成功从 '" << importFile << "' 导入到 '" << instanceName << "'\n";
        cout << "位置: " << targetDir << "\n";

    }
    catch (const std::exception& e) {
        cerr << "导入失败: " << e.what() << "\n";
    }

    // 清理临时目录
    system(("rmdir /s /q \"" + tempDir + "\"").c_str());
}

/* =========== 扩展相关 =========== */

void installExtension(const string& name, const string& extId, const L10N& L) {
    string cli = vsCodeCliPath(name);
    if (!fileExists(cli)) {
        cerr << L.extNotFound << "\n";
        return;
    }

    string args = "\"" + cli + "\" --install-extension \"" + extId + "\" "
        "--user-data-dir=\"" + rootDir(name) + "\\data\" "
        "--extensions-dir=\"" + rootDir(name) + "\\extensions\"";

    STARTUPINFOA si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    if (CreateProcessA(nullptr, const_cast<char*>(args.c_str()),
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        printf(L.extOK.c_str(), extId.c_str());
    }
    else {
        cerr << "Install failed, code: " << GetLastError() << "\n";
    }
}
// 批量安装
void importExtensions(const string& name, const string& listFile, const L10N& L)
{
    if (!fileExists(listFile)) {
        cerr << "扩展列表文件不存在: " << listFile << "\n";
        return;
    }
    std::ifstream fin(listFile);
    if (!fin) {
        cerr << "无法读取扩展列表文件\n";
        return;
    }
    string line;
    while (std::getline(fin, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty() || line.front() == '#') continue;   // 支持行注释
        installExtension(name, line, L);                     // 复用已有函数
    }
}

// 列出已装扩展
void listExtensions(const string& name, const L10N& L)
{
    string cli = vsCodeCliPath(name);
    if (!fileExists(cli)) {
        cerr << L.extNotFound << "\n";
        return;
    }
    string args = "\"" + cli + "\" --list-extensions "
        "--user-data-dir=\"" + rootDir(name) + "\\data\" "
        "--extensions-dir=\"" + rootDir(name) + "\\extensions\"";

    // 直接让 code CLI 输出，不捕获
    STARTUPINFOA si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    if (CreateProcessA(nullptr, const_cast<char*>(args.c_str()),
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else {
        cerr << "列出扩展失败，错误码: " << GetLastError() << "\n";
    }
}

/* =========== 入口 =========== */
int main(int argc, char** argv) {
    printBanner();
    L10N lang = EN;
    bool randomHost = false, randomMac = false;
    string proxy;
    bool useSandbox = false, useAppContainer = false, useWSB = false, fakeHW = false, rest = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--lang") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "cn") == 0) lang = CN;
            ++i;
        }
        else if (strcmp(argv[i], "--host") == 0)    randomHost = true;
        else if (strcmp(argv[i], "--mac") == 0)    randomMac = true;
        else if (strcmp(argv[i], "--proxy") == 0 && i + 1 < argc) {
            proxy = argv[++i];
        }
        else if (strcmp(argv[i], "--sandbox") == 0) useSandbox = true;
        else if (strcmp(argv[i], "--appcontainer") == 0) cerr << "AppContainer支持已在正式版被删除，请使用sandbox\n";
        else if (strcmp(argv[i], "--wsb") == 0) cerr << "WSB支持已在正式版被删除，请使用sandbox\n";
        else if (strcmp(argv[i], "--fake-hw") == 0) fakeHW = true;
        else if (strcmp(argv[i], "--augment") == 0) cerr << "agument已在正式版被删除，请使用regist功能\n";
    }

    if (argc < 2)
    {
        showUsage();
        return 1;
    }
    // ============================== 不需要实例名称的 ==============================
    string cmd = argv[1];
    if (cmd == "--version") {
        return 0;
    }
    else if (cmd == "rest") {
        printBanner();
        // 处理带双引号的路径 (用户拖拽文件时可能包含)
        string path;
        if (argc >= 3) {
            path = argv[2];
            // 去除路径两端的双引号
            if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
                path = path.substr(1, path.size() - 2);
            }
        }

        if (path.empty()) {
            cerr << "错误：请提供 VS Code 的 Code.exe 路径\n";
            cerr << "用法: vsenv rest \"C:\\Path\\To\\Code.exe\"\n";
            return 1;
        }

        // 检查路径是否存在
        if (!fileExists(path)) {
            cerr << "错误：文件不存在 - " << path << "\n";
            return 1;
        }

        // 恢复协议
        if (restoreCodeProtocol(path)) {
            cout << "✅ vscode:// 协议已成功恢复至: " << path << "\n";
        }
        else {
            cerr << "恢复失败，请检查路径和权限\n";
        }
        return 0;
    }
    else if (cmd == "logoff") {

        if (restoreOriginalVSCodeHandler()) {
            cout << lang.logoffOK << "\n";
            return 0;
        }
        else {
            cerr << "恢复 vscode:// 失败\n";
            return 1;
        }
    }
    else if (cmd == "list") {
        auto v = enumerateInstances();
        printInstanceTable(v);
        return 0;
    }
    else if (cmd == "f") {
        interactiveMode(lang);
        return 0;
    }

    if (argc < 3) {
        cerr << "错误：需要实例名称\n";
        return 1;
    }
    // =============================== 需要实例名称的 ===============================
    string name = argv[2];
    if (cmd == "create")
    {

        string custom;
        if (argc >= 4 && argv[3][0] != '-')   // 不是开关就是路径
            custom = argv[3];
        create(name, custom, lang);
    }
    else if (cmd == "start") {

        start(name, lang, randomHost, randomMac, proxy, useSandbox, useAppContainer, useWSB, fakeHW);
    }
    else if (cmd == "stop") {

        stop(name, lang, argv[0]);
    }
    else if (cmd == "remove") {

        remove(name, lang, argv[0]);
    }
    else if (cmd == "regist") {

        backupOriginalVSCodeHandler();   // 先备份
        if (registVSCodeProtocol(name)) {
            printf(lang.registOK.c_str(), name.c_str());
        }
        else {
            cerr << "注册失败\n";
        }
    }
    else if (cmd == "regist-guard") {   // 新增

        guardRegist(name, lang);
    }
    else if (cmd == "extension") {
        if (argc < 3) {
            cerr << "用法:\n"
                "  vsenv extension <实例名> <扩展ID>          # 安装单个扩展\n"
                "  vsenv extension <实例名> import <列表文件> # 批量安装\n"
                "  vsenv extension <实例名> list             # 列出已装扩展\n";
            return 1;
        }
        string IName = argv[2];
        string sub = argv[3];
        if (sub == "import") {
            if (argc < 5) { cerr << "需要指定列表文件\n"; return 1; }
            importExtensions(IName, argv[4], lang);
        }
        else if (sub == "list") {
            if (argc < 4) { cerr << "需要指定实例名\n"; return 1; }
            listExtensions(IName, lang);
        }
        else {
            // 保持原有单扩展安装逻辑
            string extId = (argc >= 4) ? argv[3]
                : "MS-CEINTL.vscode-language-pack-zh-hans";
            installExtension(sub, extId, lang);
        }
    }
    else if (cmd == "export") {
        if (argc < 4) {
            cerr << "错误：需要实例名称和导出路径\n";
            cerr << "用法: vsenv export <实例名> <导出路径/文件名.vsenv>\n";
            return 1;
        }
        string exportPath = argv[3];
        exportInstance(name, exportPath, lang);
    }
    else if (cmd == "import") {
        if (argc < 3) {
            cerr << "错误：需要配置文件路径\n";
            cerr << "用法: vsenv import <配置文件名.vsenv> [新实例名]\n";
            return 1;
        }
        string importFile = argv[2];
        string newName = (argc >= 4) ? argv[3] : "";
        importInstance(importFile, newName, lang);
    }

    else {

        cerr << "无效命令 '" << cmd << "'";
        return 1;
    }
    return 0;
}