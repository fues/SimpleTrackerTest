# SimpleTrackerTest by fues

OpenVRドライバ作成例である。比較的シンプルな構成を目指した。

で作成したものである。

## 動作

2つのボタン等を持たないトラッカーを追加する。
ドライバのソースファイルを切り替えて以下の2種類の動作をさせることができる。

- 両手のコントローラーに追従しながら回転と上下する
- 位置をドライバ操作プログラムで操作する。

## 実行環境

|項目|名称|
|--|--|
|OS|Windows10 64bit|
|IDE|VisualStudio 2017 v141|

VisualStudio に Microsoft Child Process Debugging Power Toolを

## ファイル構成

```txt
├─openvr-1.16.8               //OpenVR SDKの中身を入れる。
│    ├─bin
│    ├─headers
│    ├─samples
│    以下略
│
├─SimpleTrackerDriverTest     //ドライバ用プロジェクト
│
└─SharedMemTest               //ドライバ操作プログラム用プロジェクト
```

## 導入

1. OpenVR SDK(1.16.8)をダウンロードし、中身をopenvr-1.16.8の中に入れる
1. `SimpleTrackerDriverTest\_driver_set.bat`を編集し、`set vrpathreg=`を正しく設定する
1. `SimpleTrackerDriverTest\_driver_en.bat`を実行し、ドライバを登録する。

## 動かす

### ドライバ単体の例

1. SimpleTrackerDriverTestのプロジェクトを開く
1. `main_shared_tracker.cpp`をビルドから除外する
1. `main_standalone.cpp`の「ビルドから除外」を解除する
1. デバッグを開始する

### プロセス間通信の例

1. SimpleTrackerDriverTestのプロジェクトを開く
1. `main_shared_tracker.cpp`の「ビルドから除外」を解除する
1. `main_standalone.cpp`をビルドから除外する
1. デバッグを開始する
1. SharedMemTestのプロジェクトを開く
1. デバッグを開始する
1. 出現したウィンドウをアクティブにし、WASD,QZ,ER,C,Vキーで移動、バッテリー残量の上下等を操作、スペースキーでどちらのトラッカーを操作するか切り替える。