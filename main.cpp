/*
    VSenv --VS Code 离线实例管理器
    以AGPLv3.0开源 dhjs0000版权所有
    该程序允许用户创建、启动、停止和删除独立的 VS Code 实例，
    每个实例拥有独立的用户数据和扩展目录。

    版本：beta 0.2.0
*/

// 常量定义

#define VSENV_VERSION "0.2.0"
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
#include <shlwapi.h>   // 为了 PathAppendA
#include <random>      // 添加随机数生成
#include <iomanip>     // 添加流格式化

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
                   "硬件指纹已随机化: CPUID=%s, 磁盘序列号=%s, MAC地址=%s" };

/* =========== 工具函数 =========== */
bool fileExists(const std::string& path);


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
string rootDir(const string& name) { return homeDir() + "\\.vsenv\\" + name; }

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

// 添加缺失的函数定义
bool registVSCodeProtocol(const string& instanceName) {
    char userProfile[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, userProfile);

    char exePath[MAX_PATH];
    PathCombineA(exePath, userProfile, (".vsenv\\" + instanceName + "\\vscode\\Code.exe").c_str());
    if (!fileExists(exePath)) return false;

    char dataDir[MAX_PATH];
    PathCombineA(dataDir, userProfile, (".vsenv\\" + instanceName + "\\data").c_str());
    char extDir[MAX_PATH];
    PathCombineA(extDir, userProfile, (".vsenv\\" + instanceName + "\\extensions").c_str());

    // 修复命令行：添加 --open-url 和 --
    string cmdLine = "\"" + string(exePath) + "\" --open-url --user-data-dir=\"" + string(dataDir) +
        "\" --extensions-dir=\"" + string(extDir) + "\" -- \"%1\"";

    HKEY hKey;
    // 修正为 vscode 协议
    string key = "Software\\Classes\\vscode\\shell\\open\\command";
    if (RegCreateKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return false;
    RegSetValueExA(hKey, "", 0, REG_SZ, (LPBYTE)cmdLine.c_str(), (DWORD)cmdLine.size() + 1);
    RegCloseKey(hKey);

    // 建 protocol 键
    key = "Software\\Classes\\vscode";
    if (RegCreateKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
        return false;
    RegSetValueExA(hKey, "URL Protocol", 0, REG_SZ, (LPBYTE)"", 1);
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

/* =========== 彩蛋 =========== */
void printBanner() {
    cout << "==================================\n";
    cout << "  追随马斯克的步伐，坚持免费开源\n";
    cout << "==================================\n\n";
    cout << "Beta " << VSENV_VERSION << " by " << VSENV_AUTHOR << " (" << VSENV_LICENSE << ")\n\n";
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
        "    # 随机主机名\n"
        "    if ($RandomHost -and $isAdmin) {\n"
        "        $env:ORIGINAL_HOSTNAME = $env:COMPUTERNAME\n"
        "        $HostName = 'VS-' + (Get-Random -Minimum 1000 -Maximum 9999)\n"
        "        Rename-Computer -NewName $HostName -Force\n"
        "    }\n"
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
void create(const string& name, const L10N& L) {
    string dir = rootDir(name);

    if (_mkdir(dir.c_str()) == -1 && errno != EEXIST) {
        cerr << "Failed to create directory: " << dir << "\n";
        return;
    }

    string dataDir = dir + "\\data";
    if (_mkdir(dataDir.c_str()) == -1 && errno != EEXIST) {
        cerr << "Failed to create directory: " << dataDir << "\n";
        return;
    }

    string extDir = dir + "\\extensions";
    if (_mkdir(extDir.c_str()) == -1 && errno != EEXIST) {
        cerr << "Failed to create directory: " << extDir << "\n";
        return;
    }

    if (!registerCustomProtocol(name)) {
        cerr << "Failed to register custom protocol\n";
    }

    printf(L.created.c_str(), name.c_str(), dir.c_str());
    printf(("\n" + L.copyHint).c_str(), dir.c_str());
}

void start(const string& name, const L10N& L,
    bool randomHost, bool randomMac, const string& proxy,
    bool useSandbox, bool useAppContainer, bool useWSB, bool fakeHW) {

    if (fakeHW) {
        FakeHardwareID fakeID = generateFakeHardwareID();
        applyFakeHardwareProfile(name, fakeID);
        printf(L.fakeHW.c_str(), fakeID.cpuID.c_str(), fakeID.diskSN.c_str(), fakeID.macAddress.c_str());
        cout << "\n";
    }

    if (useSandbox) {
        if (startInSandbox(name, L)) return;
    }
    if (useAppContainer) {
        if (startInAppContainer(name, L)) return;
    }
    if (useWSB) {
        startInWindowsSandbox(name, L);
        return;
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
    // 1. 杀进程
    system(("taskkill /FI \"WINDOWTITLE eq " + name + "*\" /T /F >nul 2>&1").c_str());

    // 2. 调用外部脚本
    //    假设脚本与 exe 同目录；若放在别处请自行调整路径
    string cmd = "pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File \""
        + string(argv0).substr(0, string(argv0).find_last_of("\\/"))
        + "\\vsenv-stop.ps1\" -InstanceName \"" + name + "\"";
    system(cmd.c_str());

    printf(L.stopped.c_str(), name.c_str());
}

void remove(const string& name, const L10N& L, char* argv0) {
    stop(name, L, argv0);

    // 1. 如果当前劫持的是本实例，先还原 vscode://
    restoreOriginalVSCodeHandler();

    // 2. 删除实例私有协议
    SHDeleteKeyA(HKEY_CURRENT_USER, ("Software\\Classes\\vsenv-" + name).c_str());

    // 3. 删除硬件配置文件
    SHDeleteKeyA(HKEY_CURRENT_USER, ("Software\\VSenv\\" + name).c_str());

    // 4. 删除目录
    system(("rmdir /s /q \"" + rootDir(name) + "\"").c_str());
    printf(L.removed.c_str(), name.c_str());
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
        else if (strcmp(argv[i], "--appcontainer") == 0) useAppContainer = true;
        else if (strcmp(argv[i], "--wsb") == 0) useWSB = true;
        else if (strcmp(argv[i], "--fake-hw") == 0) fakeHW = true;
    }

    if (argc < 2) {
        cerr << "Usage:\n"
            "  vsenv create <instance> [--lang cn]\n"
            "  vsenv start  <instance> [--lang cn] [--host] [--mac] [--proxy <url>] [--sandbox|--appcontainer|--wsb] [--fake-hw]\n"
            "  vsenv stop   <instance> [--lang cn]\n"
            "  vsenv remove <instance> [--lang cn]\n"
            "  vsenv regist  <instance>        # redirect vscode:// to this instance\n"
            "  vsenv logoff                    # restore original vscode:// handler\n"
            "  vsenv rest <path>               # 手动重建 vscode:// 协议 (支持拖拽带双引号的路径)\n"
            "\n"
            "Global options:\n"
            "  --lang <en|cn>   Set display language. Default is \"en\".\n"
            "\n"
            "vsenv start options:\n"
            "  --host              Randomise the hostname inside the current Windows session\n"
            "                      (requires elevation). Useful to hide the real computer name.\n"
            "  --mac               Generate a random MAC address and create a temporary\n"
            "                      virtual NIC named VSenv-<instance> (requires elevation).\n"
            "  --proxy <url>       Force WinHTTP traffic through the given HTTP(S) proxy.\n"
            "                      Example: --proxy http://127.0.0.1:8080\n"
            "  --sandbox           Launch VS Code in a restricted Logon-Session sandbox\n"
            "                      (legacy method, low isolation).\n"
            "  --appcontainer(WIP) Launch VS Code inside an AppContainer\n"
            "                      (medium isolation, Win32k lockdown, no admin rights).\n"
            "  --wsb(WIP)          Launch VS Code inside Windows Sandbox (full virtual OS,\n"
            "                      best isolation, requires Windows Pro/Enterprise).\n"
            "  --fake-hw           Randomize hardware fingerprints (CPUID, Disk Serial, MAC)\n"
            "                      for enhanced privacy and isolation.\n"
            "\n"
            "Examples:\n"
            "  vsenv create work --lang cn\n"
            "  vsenv start work --appcontainer --fake-hw\n"
            "  vsenv start work --host --mac --proxy http://proxy.internal:3128 --wsb\n";
        return 1;
    }

    string cmd = argv[1];
    if (cmd == "rest") {
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

    // 其他命令需要实例名称
    if (argc < 3) {
        cerr << "错误：需要实例名称\n";
        return 1;
    }

    string name = argv[2];
    if (cmd == "create") {
        create(name, lang);
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
    else if (cmd == "logoff") {
        if (restoreOriginalVSCodeHandler()) {
            cout << lang.logoffOK << "\n";
        }
        else {
            cerr << "恢复 vscode:// 失败\n";
        }
    }
    else {
        cerr << "无效命令 '" << cmd << "'";
        return 1;
    }
    return 0;
}