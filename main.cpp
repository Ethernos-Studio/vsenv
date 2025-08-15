// main.cpp —— VS Code 离线实例管理器
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#include <iostream>
#include <string>

#pragma comment(lib, "shell32.lib")

using std::string;
using std::cout;
using std::cerr;

/* =========== 语言包 =========== */
struct L10N {
    string title = "VSenv - 独立 VS Code 实例管理器";
    string created = "实例 '%s' 已创建：%s";
    string copyHint = "请把离线包解压到 %s\\vscode\\";
    string notFound = "未找到 Code.exe，请检查路径。";
    string started = "已启动 %s";
    string stopped = "已停止 %s";
    string removed = "已删除 %s";
};

static L10N EN;               // 默认英文
static L10N CN = { "VSenv - 独立 VS Code 实例管理器",
                   "实例 '%s' 已创建：%s",
                   "请把离线包解压到 %s\\vscode\\",
                   "未找到 Code.exe，请检查路径。",
                   "已启动 %s",
                   "已停止 %s",
                   "已删除 %s" };

/* =========== 工具函数 =========== */
string homeDir() {
    char buf[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, buf);
    return buf;
}
string rootDir(const string& name) { return homeDir() + "\\.vsenv\\" + name; }

bool fileExists(const string& path) {
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

/* =========== 彩蛋 =========== */
void printBanner() {
    cout << "==================================\n";
    cout << "  追随马斯克的步伐，坚持免费开源\n";
    cout << "==================================\n\n";
}

/* =========== 业务逻辑 =========== */
void create(const string& name, const L10N& L) {
    string dir = rootDir(name);
    _mkdir(dir.c_str());
    _mkdir((dir + "\\data").c_str());
    _mkdir((dir + "\\extensions").c_str());
    printf(L.created.c_str(), name.c_str(), dir.c_str());
    printf(("\n" + L.copyHint).c_str(), dir.c_str());
}

void start(const string& name, const L10N& L) {
    string dir = rootDir(name);
    string exe = dir + "\\vscode\\Code.exe";
    if (!fileExists(exe)) {
        cerr << L.notFound << "\n"; return;
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

void stop(const string& name, const L10N& L) {
    system(("taskkill /FI \"WINDOWTITLE eq " + name + "*\" /T /F >nul 2>&1").c_str());
    printf(L.stopped.c_str(), name.c_str());
}

void remove(const string& name, const L10N& L) {
    stop(name, L);
    system(("rmdir /s /q \"" + rootDir(name) + "\"").c_str());
    printf(L.removed.c_str(), name.c_str());
}

int main(int argc, char** argv) {
    printBanner();                               // 彩蛋
    L10N lang = EN;                              // 默认英文
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--lang") == 0 && i + 1 < argc) {
            if (strcmp(argv[i + 1], "cn") == 0) lang = CN;
            ++i;                                 // 跳过参数值
        }
    }

    if (argc < 2) {
        cerr << "Usage:\n"
            "  vsenv create <instance> [--lang cn]\n"
            "  vsenv start  <instance> [--lang cn]\n"
            "  vsenv stop   <instance> [--lang cn]\n"
            "  vsenv remove <instance> [--lang cn]\n";
        return 1;
    }
    string cmd = argv[1], name = argv[2];
    if (cmd == "create") {
        create(name, lang);
    }
    else if (cmd == "start") {
        start(name, lang);
    }
    else if (cmd == "stop") {
        stop(name, lang);
    }
    else if (cmd == "remove") {
        remove(name, lang);
    }
    else {
        cerr << "Invalid command\n";
        return 1;
    }
}