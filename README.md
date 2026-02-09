# AudioCleanUp HLS IP

## 1. 概要
AudioCleanUp は、マイク入力のオーディオ信号に対して  
DCオフセット除去およびノイズ低減を行うためのHLS IPである。

本IPはDC CutおよびNoise Gateの2つの処理ブロックから構成され、
AXI4-Streamで入力されたステレオPCMデータをリアルタイムに処理し、同形式で出力する。

本リポジトリは、AudioCleanUp IPの **HLS 実装および内部仕様の管理**を目的とする。

---

## 2. 特徴

- Vitis HLSによるC/C++記述
- AXI4-Streamベースのリアルタイム音声処理
- DC Cut + Noise Gate の直列構成
- 固定小数点演算によるFPGA向け実装
- パラメータはAXI4-Liteから動的に制御可能
- 高位合成でのGoogle Test / CI

---

## 3. ディレクトリ構成
```
.
├── design/
│   ├── AudioCleanUp              # TOP階層プロジェクト
│   ├── DcCut                     # DcCutプロジェクト
│   ├── NoiseGate                 # NoiseGateプロジェクト
│   └── from_hw/                  # xsa管理
├── doc/
│   ├── AudioCleanUp_設計書.md     # 設計書
│   └── img/                      # 設計書で使用した画像
├── src/
│   ├── AudioCleanUp              # AudioCleanUpソースコード
│   ├── Common                    # 共通ソースコード
│   ├── DcCut                     # DcCutソースコード
│   ├── NoiseGate                 # NoiseGateソースコード
│   └── include/                  # VitisHLSのヘッダーファイル（Google Test時に使用）
├── verification/
│   ├── AudioCleanUp              # AudioCleanUp検証環境
│   ├── Common                    # 検証での共通処理
│   ├── DcCut                     # DcCut検証環境
│   └── NoiseGate                 # NoiseGate検証環境
└── README.md
```

## 4. テスト環境

後で書く

---

## 5. 注意事項

何かあれば書く
