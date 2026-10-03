# 第 4 課:UART 序列埠 — 讓晶片對電腦說話(組合語言版)

## 這個範例做什麼
每隔約 1 秒,從序列埠送出一行固定字串 `Hello from N76E003!`。

## 接線
需要一個 USB-TTL 轉接器:
```
N76E003 P0.6 (TXD) --> USB-TTL 的 RX
N76E003 GND        --> USB-TTL 的 GND
```
- 本課只示範「送」,RXD 可先不接。
- 序列埠參數:**9600 8N1**。

## 編譯與燒錄
```bash
make        # 組譯 + 連結,產生 main.hex
make flash  # 燒錄到 N76E003
make clean  # 清除編譯產物
```
> 首次燒錄前,請先完成 [`../../docs/00-環境建置與燒錄.md`](../../docs/00-環境建置與燒錄.md) 的 udev 設定。

## 預期結果
序列埠終端機每秒出現一行:
```
Hello from N76E003!
Hello from N76E003!
...
```
終端機指令(擇一;裝置名稱常見 `/dev/ttyUSB0`):
```bash
picocom -b 9600 /dev/ttyUSB0
# 或
screen /dev/ttyUSB0 9600
```

## 程式重點
- UART 設定與 C 版相同(Timer1 當鮑率來源、9600 8N1)。
- 字串放在程式記憶體 (CODE),用 `DPTR` 指向、`movc A,@A+DPTR` 逐字讀出,讀到 0 結束。
- `putc` 把字元寫入 `SBUF`,`jnb TI, wait_ti` 等送完再清 `TI`。
- 原理詳見 [`../../docs/05-UART序列埠.md`](../../docs/05-UART序列埠.md)。

## 對照另一個版本
→ C 版:[`../../c/04-uart/`](../../c/04-uart/)。C 版還會印出遞增的 `count`;組語為求單純只送固定字串,並手動用 `movc`/`DPTR` 處理字串。

## 練習
1. 修改 `msg` 字串,送出你自己的訊息。
2. 參考 C 版,替組語版加上「印十進位數字」的副程式(需要除以 10 的迴圈)。
3. 接上 RXD,做收到字元就回送的回音程式。
