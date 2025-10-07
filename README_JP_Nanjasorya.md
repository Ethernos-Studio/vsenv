# VSenv - VS Code オフラインインスタンスマネージャーですわ！

![License](https://img.shields.io/badge/License-AGPLv3.0-blue.svg )  
![Version](https://img.shields.io/badge/Version-1.0.0-green.svg )  
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg )

[English](./README_EN.md) | [中文](./README.md)

## 📖 目次ですよ〜
- [プロジェクト概要](#プロジェクト概要)
- [主要機能](#主要機能)
- [システム要件](#システム要件)
- [インストールガイド](#インストールガイド)
- [クイックスタート](#クイックスタート)
- [詳細な使い方](#詳細な使い方)
- [技術アーキテクチャ](#技術アーキテクチャ)
- [プライバシーとセキュリティ](#プライバシーとセキュリティ)
- [トラブルシューティング](#トラブルシューティング)
- [貢献ガイド](#貢献ガイド)
- [ライセンス](#ライセンス)
- [更新履歴](#更新履歴)

---

## 🚀 プロジェクト概要ですわ！

VSenvは、プロフェッショナル向けのVS Codeオフラインインスタンスマネージャーですよ〜！  
ユーザーは完全に独立したVS Codeインスタンスを作成・管理・実行できますの♪  
各インスタンスは独自のユーザーデータ、拡張機能ディレクトリ、設定環境を持っていますわ！

- 🔒 **プライバシー保護**：拡張機能や設定データの相互干渉を防げますの！  
- 🏢 **企業環境**：異なるプロジェクトごとに分離された開発環境を作れちゃうのよ〜  
- 🔬 **テスト・検証**：メイン環境に影響を与えることなく拡張機能や設定をテストできるのですわ！  
- 🌐 **ネットワーク隔離**：プロキシやネットワーク分離設定に対応してますの♪  
- 📦 **オフライン展開**：オフライン環境での拡張機能管理を完全サポートですよ〜！

---

## ✨ 主要機能ですわ！

### 🛡️ 多層分離なのです！
- **ファイルシステム分離**：各インスタンスに独立したdata・extensionsディレクトリですの♪  
- **レジストリ分離**：独自のプロトコル登録とハードウェアフィンガープリント設定ですわ！  
- **ネットワーク分離**：プロキシ設定や仮想NICシミュレーションに対応してますのよ〜  
- **プロセス分離**：サンドボックスモードでの実行も選択可能ですっ！

### 🔧 高度な機能なのです！
- **ハードウェアフィンガープリント偽装**：CPUID、ディスクシリアル番号、MACアドレスをランダム化しちゃうのよ〜！  
- **プロトコルリダイレクト**：vscode://プロトコルハンドラをカスタマイズできますわ！  
- **拡張機能一括管理**：拡張機能リストのインポート/エクスポートに対応してますの♪  
- **インスタンス移行**：インスタンスの完全なエクスポート/インポート機能ですよ〜！  
- **多言語対応**：日本語・英語・中国語インターフェースに対応してますのですわ！

---

## 💻 システム要件ですよ〜

### 最小要件ですわ
- **OS**: Windows 10以降（64ビット）ですの  
- **アーキテクチャ**: x64ですっ！  
- **メモリ**: 4GB RAMですよ〜  
- **ストレージ**: 空き容量2GB以上ですわ！

### 推奨環境ですの♪
- **OS**: Windows 11 22H2以降ですわ！  
- **メモリ**: 8GB RAM以上がおすすめですのよ〜  
- **ストレージ**: SSDで空き容量10GB以上ですっ！  
- **権限**: 管理者権限（高度なネットワーク機能利用時）ですわ！

---

## 📥 インストールガイドですよ〜！

### 方法1：プリコンパイル済みバイナリ（おすすめですわ！）
1. [Releasesページ](https://github.com/dhjs0000/vsenv/releases)へアクセスですの♪  
2. 最新の`vsenv-x.x.x-windows-amd64.zip`をダウンロードですわ！  
3. 任意のフォルダ（例：`C:\Tools\vsenv`）へ展開ですよ〜  
4. システムのPATH環境変数に追加してくださいの！

### 方法2：ソースコードからビルドですの！
```bash
git clone https://github.com/dhjs0000/vsenv.git
cd vsenv
vcpkg install nlohmann-json
msbuild vsenv.sln /p:Configuration=Release
```

### インストール確認ですわ！
```cmd
vsenv --version
# VSenv 1.0.0 by dhjs0000 (AGPLv3.0) と表示されたら成功ですの♪
```

---

## 🚀 クイックスタートですよ〜！

### 初回インスタンス作成ですわ！
```cmd
vsenv create myproject
# VS Codeオフライン配布版を展開するのですっ！
# デフォルト: C:\Users\<ユーザー名>\.vsenv\myproject\vscode\
```

### インスタンス起動ですの！
```cmd
vsenv start myproject
vsenv start myproject --fake-hw --sandbox  # プライバシー強化ですわ！
```

### 拡張機能インストールですよ〜！
```cmd
vsenv extension myproject MS-CEINTL.vscode-language-pack-ja
vsenv extension list myproject
```

---

## 📚 詳細な使い方ですわ！

### インスタンス管理コマンドですの♪

#### 作成ですよ〜
```cmd
vsenv create project-alpha
vsenv create project-beta "D:\Dev\vscode-instances\beta"
vsenv create project-gamma --lang ja
```

#### 起動オプションですわ！
```cmd
vsenv start project-alpha --host --mac --proxy http://127.0.0.1:8080
vsenv start project-alpha --sandbox --fake-hw
vsenv start project-alpha --lang ja
```
- `--host`: ホスト名ランダム化（要管理者権限）ですの！  
- `--mac`: 仮想NIC作成＋MACアドレスランダム化ですわ！  
- `--proxy`: HTTPプロキシ指定ですよ〜  
- `--sandbox`: 制限付きLogonSessionで実行ですっ！  
- `--fake-hw`: ハードウェアフィンガープリント偽装ですの♪

#### 停止・削除ですわ！
```cmd
vsenv stop  project-alpha
vsenv remove project-alpha   # 完全削除（復元不可）ですよ〜！
```

---

## 🏗️ 技術アーキテクチャですの！

### ディレクトリ構造ですわ！
```
.vsenv/
├── instance1/
│   ├── vscode/
│   ├── data/
│   ├── extensions/
│   └── logs/
└── otherPath.json
```

### レジストリ構造ですよ〜
```
HKEY_CURRENT_USER
└── Software
    ├── Classes
    │   ├── vsenv-{instancename}
    │   └── vscode
    └── VSenv
        └── {instancename}
            └── Hardware
```

### 分離メカニズムですの♪
1. ファイルシステムレベル分離  
2. レジストリレベル分離  
3. プロセスレベル分離（サンドボックス）  
4. ネットワークレベル分離（プロキシ・仮想NIC）

---

## 🔒 プライバシーとセキュリティですわ！

### ハードウェアフィンガープリント保護ですの！
- CPUID
- ディスクシリアル番号
- MACアドレス  
をランダム化できますわ！

### ネットワークプライバシーですよ〜
- HTTP/HTTPSプロキシ対応ですの！
- 仮想NIC MACランダム化ですわ！
- ホスト名ランダム化（要管理者権限）ですっ！

---

## 🐛 トラブルシューティングですわ！

| 現象 | 対処ですの♪ |
|------|--------------|
| Code.exe not found | インスタンス内`vscode\Code.exe`を確認してくださいね |
| 権限不足 | 管理者権限でコマンドプロンプト起動ですわ！ |
| vscode://リダイレクト失敗 | `vsenv logoff` → `vsenv regist <Instance>` ですの！ |
| 拡張機能インストールエラー | ネットワーク・拡張ID確認、`--proxy`使ってみてね |

### ログですよ〜
`インスタンス\logs\vsenv.log`に出力ですわ！

### デバッグモードですの！
```cmd
set VSENV_DEBUG=1
vsenv start project-alpha
```

---

## 🤝 貢献ガイドですわ！

1. リポジトリをForkですの♪  
2. 機能ブランチ作成ですよ〜  
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. コミットですわ！
   ```bash
   git commit -m 'Add some amazing feature'
   ```
4. プッシュ＆Pull Requestですの！  
5. コードスタイル・コメント・ドキュメント・テストをお願いしますね！

---

## 📄 ライセンスですわ！

**AGPLv3.0**ですの！

```
VSenv - VS Code オフラインインスタンスマネージャー
Copyright (C) 2025 dhjs0000

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
```

---

## 📋 更新履歴ですよ〜！

### v1.0.0 (2025-09-27)ですわ！
- インスタンス作成・起動・停止・削除  
- 多層分離（ファイル・レジストリ・ネットワーク）  
- ハードウェアフィンガープリント偽装  
- プロトコルリダイレクト＆ガード  
- 拡張機能一括管理  
- インスタンスエクスポート/インポート  
- 日本語・英語・中国語対応  
- サンドボックス実行環境  
- ネットワーク隔離＆プロキシ対応  

---

## 🔗 関連リンクですの！

- 📖 **詳細ドキュメント**: [https://dhjs0000.github.io/VSenv/](https://dhjs0000.github.io/VSenv/)  
- 🐛 **問題報告**: [GitHub Issues](https://github.com/dhjs0000/vsenv/issues)  
- 💬 **ディスカッション**: [GitHub Discussions](https://github.com/dhjs0000/vsenv/discussions)  
- ⭐ **Star押してね**: 役に立ったらStarお願いしますわ〜！

---

**注意**: 本ソフトウェアは完全に無料・オープンソースですの！  
もし有料で購入した場合は詐欺ですよ〜！正規チャネルから入手してくださいねっ！

---

*最終更新: 2025年9月*  
*メンテナ: [dhjs0000](https://github.com/dhjs0000)* ですわ〜！♪