# VSenv - VS Code オフラインインスタンスマネージャー 

![License](https://img.shields.io/badge/License-AGPLv3.0-blue.svg )  
![Version](https://img.shields.io/badge/Version-1.0.0-green.svg )  
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg )

[English](./README_EN.md) | [中文](./README.md)

## 📖 目次
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

## 🚀 プロジェクト概要

VSenvは、プロフェッショナル向けのVS Codeオフラインインスタンスマネージャーです。ユーザーは完全に独立したVS Codeインスタンスを作成・管理・実行できます。各インスタンスは独自のユーザーデータ、拡張機能ディレクトリ、設定環境を持ち、以下のような用途に最適です：

- 🔒 **プライバシー保護**：拡張機能や設定データの相互干渉を防ぐ  
- 🏢 **企業環境**：異なるプロジェクトごとに分離された開発環境を作成  
- 🔬 **テスト・検証**：メイン環境に影響を与えることなく拡張機能や設定をテスト  
- 🌐 **ネットワーク隔離**：プロキシやネットワーク分離設定に対応  
- 📦 **オフライン展開**：オフライン環境での拡張機能管理を完全サポート

---

## ✨ 主要機能

### 🛡️ 多層分離
- **ファイルシステム分離**：各インスタンスに独立したdata・extensionsディレクトリ  
- **レジストリ分離**：独自のプロトコル登録とハードウェアフィンガープリント設定  
- **ネットワーク分離**：プロキシ設定や仮想NICシミュレーションに対応  
- **プロセス分離**：サンドボックスモードでの実行も選択可能

### 🔧 高度な機能
- **ハードウェアフィンガープリント偽装**：CPUID、ディスクシリアル番号、MACアドレスをランダム化  
- **プロトコルリダイレクト**：vscode://プロトコルハンドラをカスタマイズ  
- **拡張機能一括管理**：拡張機能リストのインポート/エクスポートに対応  
- **インスタンス移行**：インスタンスの完全なエクスポート/インポート機能  
- **多言語対応**：日本語・英語・中国語インターフェースに対応

---

## 💻 システム要件

### 最小要件
- **OS**: Windows 10以降（64ビット）
- **アーキテクチャ**: x64
- **メモリ**: 4GB RAM
- **ストレージ**: 空き容量2GB以上

### 推奨環境
- **OS**: Windows 11 22H2以降
- **メモリ**: 8GB RAM以上
- **ストレージ**: SSD、空き容量10GB以上
- **権限**: 管理者権限（高度なネットワーク機能利用時）

---

## 📥 インストールガイド

### 方法1：プリコンパイル済みバイナリ（推奨）
1. [Releasesページ](https://github.com/dhjs0000/vsenv/releases)へアクセス  
2. 最新の`vsenv-x.x.x-windows-amd64.zip`をダウンロード  
3. 任意のフォルダ（例：`C:\Tools\vsenv`）へ展開  
4. システムのPATH環境変数に追加

### 方法2：ソースコードからビルド
```bash
# クローン
git clone https://github.com/dhjs0000/vsenv.git
cd vsenv

# 依存関係インストール
vcpkg install nlohmann-json

# Visual Studio 2019以降でビルド
msbuild vsenv.sln /p:Configuration=Release
```

### インストール確認
コマンドプロンプトで以下を実行：
```cmd
vsenv --version
```
出力例：`VSenv 1.0.0 by dhjs0000 (AGPLv3.0)`

---

## 🚀 クイックスタート

### 初回インスタンス作成
```cmd
# 「myproject」というインスタンスを作成
vsenv create myproject

# VS Codeオフライン配布版を以下に展開
# デフォルトパス: C:\Users\<ユーザー名>\.vsenv\myproject\vscode\
```

### インスタンス起動
```cmd
# 通常起動
vsenv start myproject

# プライバシー保護付き起動
vsenv start myproject --fake-hw --sandbox
```

### 拡張機能インストール
```cmd
# 日本語言語パック
vsenv extension myproject MS-CEINTL.vscode-language-pack-ja

# インストール済み一覧
vsenv extension list myproject
```

---

## 📚 詳細な使い方

### インスタンス管理コマンド

#### 作成
```cmd
# デフォルト場所に作成
vsenv create project-alpha

# カスタム場所に作成
vsenv create project-beta "D:\Dev\vscode-instances\beta"

# 日本語UIで作成
vsenv create project-gamma --lang ja
```

#### 起動オプション
```cmd
vsenv start project-alpha --host --mac --proxy http://127.0.0.1:8080
vsenv start project-alpha --sandbox --fake-hw
vsenv start project-alpha --lang ja
```
- `--host`: ホスト名ランダム化（要管理者権限）  
- `--mac`: 仮想NIC作成＋MACアドレスランダム化  
- `--proxy`: HTTPプロキシ指定  
- `--sandbox`: 制限付きLogonSessionで実行  
- `--fake-hw`: ハードウェアフィンガープリント偽装

#### 停止・削除
```cmd
vsenv stop  project-alpha
vsenv remove project-alpha   # 完全削除（復元不可）
```

### 拡張機能管理

#### 単体操作
```cmd
vsenv extension project-alpha ms-python.python
vsenv extension project-alpha MS-CEINTL.vscode-language-pack-ja
```

#### 一括操作
```cmd
vsenv extension import project-alpha extensions.txt
vsenv extension list project-alpha > installed_extensions.txt
```

**extensions.txt例**  
```
# コメント行
ms-python.python
ms-vscode.vscode-typescript-next
bradlc.vscode-tailwindcss
```

### プロトコル管理
```cmd
vsenv regist project-alpha        # vscode://を当該インスタンスへ
vsenv regist-guard project-alpha  # ガードモード（他プログラムによる上書き防止）
vsenv logoff                      # デフォルトへ戻す
vsenv rest "C:\Path\To\Code.exe"  # 手動再登録
```

### エクスポート/インポート
```cmd
vsenv export project-alpha "D:\Backups\project-alpha.vsenv"
vsenv import "D:\Backups\project-alpha.vsenv" project-new
```

### 一覧表示
```cmd
vsenv list
# Instance Name    Path
# -----------------------------------------
# project-alpha    C:\Users\user\.vsenv\project-alpha
# project-beta     D:\Dev\vscode-instances\beta
```

---

## 🏗️ 技術アーキテクチャ

### ディレクトリ構造
```
.vsenv/
├── instance1/
│   ├── vscode/     # VS Code本体
│   ├── data/       # ユーザー設定・状態
│   ├── extensions/ # 拡張機能
│   └── logs/       # ログ
├── instance2/
└── otherPath.json  # カスタムパス情報
```

### レジストリ構造
```
HKEY_CURRENT_USER
└── Software
    ├── Classes
    │   ├── vsenv-{instancename}
    │   └── vscode
    └── VSenv
        └── {instancename}
            └── Hardware   # ハードウェアフィンガープリント
```

### 分離メカニズム
1. ファイルシステムレベル分離  
2. レジストリレベル分離  
3. プロセスレベル分離（サンドボックス）  
4. ネットワークレベル分離（プロキシ・仮想NIC）

---

## 🔒 プライバシーとセキュリティ

### ハードウェアフィンガープリント保護
- CPUID
- ディスクシリアル番号
- MACアドレス  
をランダム化可能

### ネットワークプライバシー
- HTTP/HTTPSプロキシ対応
- 仮想NIC MACランダム化
- ホスト名ランダム化（要管理者権限）

### データ分離
- インスタンス間完全隔離
- 独立した拡張機能エコシステム
- 独立したユーザー設定と状態

---

## 🐛 トラブルシューティング

### よくある問題

| 現象 | 対処 |
|------|------|
| Code.exe not found | インスタンス内`vscode\Code.exe`を確認 |
| 権限不足（ネットワーク機能） | 管理者権限でコマンドプロンプト起動 |
| vscode://リダイレクト失敗 | `vsenv logoff` → `vsenv regist <Instance>` |
| 拡張機能インストールエラー | ネットワーク・拡張ID確認、`--proxy`利用 |

### ログ
`インスタンス\logs\vsenv.log`に出力

### デバッグモード
```cmd
set VSENV_DEBUG=1
vsenv start project-alpha
```

---

## 🤝 貢献ガイド

1. 本リポジトリをFork  
2. 機能ブランチ作成  
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. コミット  
   ```bash
   git commit -m 'Add some amazing feature'
   ```
4. プッシュ＆Pull Request  
5. 既存コードスタイル・コメント・ドキュメント更新・テストを実施

### 問題報告
GitHub Issuesに以下を含めて報告：
- 詳細な説明
- 再現手順
- 環境情報
- エラーログ

---

## 📄 ライセンス

**AGPLv3.0**

```
VSenv - VS Code オフラインインスタンスマネージャー
Copyright (C) 2025 dhjs0000

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
```

---

## 📋 更新履歴

### v1.0.0 (2025-09-27)
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

## 🔗 関連リンク

- 📖 **詳細ドキュメント**: [https://dhjs0000.github.io/VSenv/](https://dhjs0000.github.io/VSenv/)
- 🐛 **問題報告**: [GitHub Issues](https://github.com/dhjs0000/vsenv/issues)
- 💬 **ディスカッション**: [GitHub Discussions](https://github.com/dhjs0000/vsenv/discussions)
- ⭐ **Starを押す**: 役に立ったらStarをお願いします！

---

**注意**: 本ソフトウェアは完全に無料・オープンソースです。もし有料で購入した場合は詐欺です。必ず正規チャネルから入手してください。

---

*最終更新: 2025年9月*  
*メンテナ: [dhjs0000](https://github.com/dhjs0000)*