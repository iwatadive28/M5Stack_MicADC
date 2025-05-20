# M5Stack Core2 FFT Audio Monitor

このプロジェクトは、M5Stack Core2 for AWS を用いて  
マイク入力の音声信号をリアルタイムに取得・表示するサンプルです。

- ボタンAで表示モードを切り替え  
  - FFT（周波数ドメイン）  
  - 時間ドメイン波形
- 切り替え時にスピーカーから「ピッ」と音が鳴ります

---

## 動作環境

- 開発環境： [PlatformIO](https://platformio.org/) + [VS Code](https://code.visualstudio.com/)
- 使用ボード： M5Stack Core2 for AWS
- フレームワーク： Arduino
- ライブラリ：
  - [M5Core2](https://registry.platformio.org/libraries/m5stack/M5Core2)
  - [arduinoFFT](https://github.com/kosme/arduinoFFT)

---

## 実行方法

1. VS Code + PlatformIO をインストール
2. 本プロジェクトをクローンして開く
3. USB接続された M5Stack Core2 に書き込み（左下の "→" ボタン）
4. 起動後、LCDにリアルタイム表示されます

---

## ディレクトリ構成（簡略）
```
├── src/
│ └── main.cpp // メインスケッチ
├── lib/ // （必要であれば自作ライブラリを配置）
├── .pio/ // PlatformIOのビルドファイル（自動生成）
├── platformio.ini // 設定ファイル
└── README.md
```

---

## 備考

- ESP32のADC特性に合わせて、ADC値は電圧に補正済み
- FFTは `256点`, `Hamming窓`, `20kHz` サンプリングで計算

---

## 🛠 今後の拡張予定（例）

- ピーク周波数の検出
- dB表示への変換
- タッチUIによる設定変更