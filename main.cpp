// main.cpp ���� VS Code ����ʵ��������
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

/* =========== ���԰� =========== */
struct L10N {
    string title = "VSenv - Stand-alone VS Code instance manager";
    string created = "Instance '%s' created at %s";
    string copyHint = "Please extract the offline package to %s\\vscode\\";
    string notFound = "Code.exe not found, please check the path.";
    string started = "Started %s";
    string stopped = "Stopped %s";
    string removed = "Removed %s";
};

static const L10N EN;              // Ĭ��Ӣ��
static L10N CN = { "VSenv - ���� VS Code ʵ��������",
                   "ʵ�� '%s' �Ѵ�����%s",
                   "������߰���ѹ�� %s\\vscode\\",
                   "δ�ҵ� Code.exe������·����",
                   "������ %s",
                   "��ֹͣ %s",
                   "��ɾ�� %s" };

/* =========== ���ߺ��� =========== */
string homeDir() {
    char buf[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, buf);
    return buf;
}
string rootDir(const string& name) { return homeDir() + "\\.vsenv\\" + name; }

bool fileExists(const string& path) {
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

/* =========== �ʵ� =========== */
void printBanner() {
    cout << "==================================\n";
    cout << "  ׷����˹�˵Ĳ����������ѿ�Դ\n";
    cout << "==================================\n\n";
}

/* =========== ҵ���߼� =========== */
void create(const string& name, const L10N& L) {
    string dir = rootDir(name);
    _mkdir(dir.c_str());
    _mkdir((dir + "\\data").c_str());
    _mkdir((dir + "\\extensions").c_str());
    printf(L.created.c_str(), name.c_str(), dir.c_str());
    printf(("\n" + L.copyHint).c_str(), dir.c_str());
}

void start(const string& name, const L10N& L,
    bool randomHost = false,
    bool randomMac = false,
    const string& proxy = "") {

    string dir = rootDir(name);
    string exe = dir + "\\vscode\\Code.exe";
    if (!fileExists(exe)) { cerr << L.notFound << "\n"; return; }

    // ����ű�����
    string cmd = "powershell -NoLogo -ExecutionPolicy Bypass -File \"vsenv-net.ps1\" " + name;
    if (randomHost) cmd += " -RandomHost";
    if (randomMac)  cmd += " -RandomMac";
    if (!proxy.empty()) cmd += " -Proxy " + proxy;
    system(cmd.c_str());

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
    printBanner();
    L10N lang = EN;
    bool randomHost = false, randomMac = false;
    string proxy;

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
    }

    if (argc < 2) {
        cerr << "Usage:\n"
            "  vsenv create <instance> [--lang cn]\n"
            "  vsenv start  <instance> [--lang cn] [--host] [--mac] [--proxy <url>]\n"
            "  vsenv stop   <instance> [--lang cn]\n"
            "  vsenv remove <instance> [--lang cn]\n";
        return 1;
    }

    string cmd = argv[1], name = argv[2];
    if (cmd == "create") {
        create(name, lang);
    }
    else if (cmd == "start") {
        start(name, lang, randomHost, randomMac, proxy);
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